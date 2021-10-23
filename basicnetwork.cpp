
#include "basicnetwork.h"
#include <unistd.h>
#include "job.h"


BasicNetwork::BasicNetwork(JobQueue *job_queue):m_is_exit(true),m_job_queue(job_queue),m_netid(0)
{
    InitSocket();
}

BasicNetwork::~BasicNetwork()
{
    Clear();
}

void BasicNetwork::Start()
{
    if (m_is_exit)
    {
        m_is_exit = false;
        m_work_thread = new thread(BasicNetwork::ThreadFunc, this);
    }
}

void BasicNetwork::Stop()
{
    if (!m_is_exit)
    {
        m_is_exit = true;
        m_work_thread->join();
    }
}

NetID BasicNetwork::Add(BasicNetworkHandler *handler)
{
    m_register_table_mutex.lock();

    RegisterTableItem item;
    item.handler = handler;
    item.write_event = 0;

    m_register_table.insert(std::make_pair(m_netid, item));
    handler->SetBasicNetwork(this);
    handler->SetNetid(m_netid);

    AddSocket(handler);

    m_register_table_mutex.unlock();
    int ret = m_netid;
    m_netid++;
    return ret;
}

void BasicNetwork::Remove(NetID netid)
{
    m_dirty_queue_mutex.lock();
    m_dirty_queue.push_back(netid);
    m_dirty_queue_mutex.unlock();
}

void BasicNetwork::Clear()
{
    Stop();

    m_register_table_mutex.lock();
    ReleaseSocket();
    for (RegisterTableIter iter = m_register_table.begin() ; iter != m_register_table.end(); ++iter)
    {
        iter->second.handler->OnClose();
        delete iter->second.handler;
    }
    m_register_table_mutex.unlock();

    m_dirty_queue_mutex.lock();
    m_dirty_queue.clear();
    m_dirty_queue_mutex.unlock();

    while (!m_job_to_push.empty())
    {
        Job *job = m_job_to_push.front();
        m_job_to_push.pop();
        delete job;
    }

    m_is_exit = false;
}

void BasicNetwork::DeleteDirtySocket()
{
    //清理dirty socket
    DirtyQueue temp_queue;
    m_dirty_queue_mutex.lock();
    m_dirty_queue.swap(temp_queue);
    m_dirty_queue_mutex.unlock();

    if (temp_queue.size() != 0)
    {
        m_register_table_mutex.lock();
        for (DirtyQueue::iterator iter = temp_queue.begin(); iter != temp_queue.end(); ++ iter)
        {
            RegisterTableIter item_erase = m_register_table.find(*iter);
            if (item_erase == m_register_table.end())
            {
                continue;
            }

            //先删除BasicNetwork中的信息，再调用OnClose
            BasicNetworkHandler *handler = item_erase->second.handler;
            SOCKET sock = handler->GetSocket();
            m_register_table.erase(*iter);
            handler->OnClose();             // 这里有点问题，这里限制了OnClose里面不能调用BasicNetwork中加锁的接口!*****************************
                                            // 因为在Linux下pthread_mutex_t是非递归锁

            RemoveSocket(sock);
            delete handler;

        }
        m_register_table_mutex.unlock();
    }
}

void BasicNetwork::PushJobToInvoke()
{
    // TODO: 线程安全
    while (!m_job_to_push.empty())
    {
        Job *job = m_job_to_push.front();
        m_job_queue->push(job);
        m_job_to_push.pop();
    }
}

void BasicNetwork::PushJob(Job * job)
{
    // TODO: 线程安全
    m_job_to_push.push(job);
}

bool BasicNetwork::UnregisterWrite(NetID netid, int num)
{
    m_register_table_mutex.lock();

    BasicNetwork::RegisterTableIter iter = m_register_table.find(netid);
    if (iter == m_register_table.end())
    {
        m_register_table_mutex.unlock();
        return false;
    }

    iter->second.write_event -= num;
    if (iter->second.write_event == 0)
    {
        UnregisterSocketWrite(iter->second.handler);
    }

    m_register_table_mutex.unlock();

    return true;
}

//工作线程的主循环函数
void BasicNetwork::ThreadFunc(void *param)
{
    BasicNetwork *pthis = (BasicNetwork*) param;
    pthis->WorkFunc();
    return;
}

void BasicNetwork::WorkFunc()
{
    HandlerList vector_can_read;
    HandlerList vector_can_write;

    for (;;)
    {
        if (m_is_exit)
        {
            return;
        }

        DeleteDirtySocket();
        PushJobToInvoke();

        m_register_table_mutex.lock();
        if (m_register_table.size() == 0)
        {
            m_register_table_mutex.unlock();
            continue;
        }
        m_register_table_mutex.unlock();

        PollSocket(&vector_can_read, &vector_can_write);

        //处理事件
        for (HandlerList::iterator iter = vector_can_read.begin(); iter != vector_can_read.end(); ++iter)
        {
            (*iter)->OnCanRead();
        }

        for (HandlerList::iterator iter = vector_can_write.begin(); iter != vector_can_write.end(); ++iter)
        {
            (*iter)->OnCanWrite();
        }
        vector_can_read.clear();
        vector_can_write.clear();
    }
    DeleteDirtySocket();
    return;
}

void BasicNetwork::InitSocket()
{
    m_epfd = epoll_create(MAX_EPOLL_SIZE);
}

void BasicNetwork::ReleaseSocket()
{
    ::close(m_epfd);
}

void BasicNetwork::AddSocket(BasicNetworkHandler* handler)
{
    SOCKET sock_fd = handler->GetSocket();
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = (void *)handler;
    if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, sock_fd, &ev) == -1)
    {
        // 添加失败
    }
}

void BasicNetwork::RemoveSocket(SOCKET sock_remove)
{
    struct epoll_event ev;
    if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, sock_remove, &ev) == -1)
    {
        // 删除失败
    }
}

void BasicNetwork::RegisterSocketWrite(BasicNetworkHandler* handler)
{
    SOCKET sock = handler->GetSocket();
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
    ev.data.ptr = (void *)handler;
    if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, sock, &ev) == -1)
    {
        // 注册写失败
    }
}

void BasicNetwork::UnregisterSocketWrite(BasicNetworkHandler* handler)
{
    SOCKET sock = handler->GetSocket();
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = (void *)handler;
    if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, sock, &ev) == -1)
    {
        // 反注册写失败
    }
}

void BasicNetwork::PollSocket(HandlerList *readhandler, HandlerList *writehandler)
{
    int eret = epoll_wait(m_epfd, m_tmp_event, MAX_EPOLL_SIZE, 10);
    if (eret > 0)
    {
        m_register_table_mutex.lock();
        for (int i = 0; i < eret; ++i)
        {
            if (m_tmp_event[i].events & EPOLLIN)
            {
                readhandler->push_back((BasicNetworkHandler*)m_tmp_event[i].data.ptr);
            }
            if (m_tmp_event[i].events & EPOLLOUT)
            {
                writehandler->push_back((BasicNetworkHandler*)m_tmp_event[i].data.ptr);
            }
        }
        m_register_table_mutex.unlock();
    }
}

