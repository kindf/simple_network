# simple_network
## 流程图

[![4ypfhR.png](https://z3.ax1x.com/2021/09/25/4ypfhR.png)](https://imgtu.com/i/4ypfhR)

## 流程

* 主线程对象network循环从m_job_queue队列中pop出job对象，回调用户传入的callback
* io线程循环执行epoll_wait，将对应的事件通知到对应的handler
* 对应handler将产生的job推到自己的m_job_to_push队列，并在下次循环的时候将该队列的job推到主线程的m_job_queue队列中

## 主要类

* network类：
* basenetwork类：
* handler类：
* job类：

## TODO

* 频繁的内存申请可以考虑使用内存池
* 减小加锁的粒度
