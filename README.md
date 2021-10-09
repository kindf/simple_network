## 流程图

[![4ypfhR.png](https://z3.ax1x.com/2021/09/25/4ypfhR.png)](https://imgtu.com/i/4ypfhR)

## 流程

* 主线程对象network循环从m_job_queue队列中pop出job对象，回调用户传入的callback
* io线程循环执行epoll_wait，将对应的事件通知到对应的handler
* 对应handler将产生的job推到自己的m_job_to_push队列，并在下次循环的时候将该队列的job推到主线程的m_job_queue队列中

## 主要类

* network类：
  * 循环调用update()函数，调用用户传入的处理函数处理工作线程传入的各类job
* basenetwork类：
  * io线程类，循环调用epoll_wait()，发生相应的事件则调用handler对应的方法处理事件
* handler类：
  * 生成各种job类对象并push到io线程的job队列中，这些job对象最终会push到主线程的工作队列中
* job类：
  * 该类的实例化对象由handler类生成，并最终会push到主线程的job队列当中
  * 主线程每次循环都会调用job类的Invoke方法，处理对应的事件（调用用户传入的处理函数处理）

## 优缺点

* 代码结构简单，使用容易
* 频繁的内存申请可能引发性能问题，可以考虑使用内存池
* 只有一个io线程遇到流量大的场景可能会导致响应缓慢

## TODO

* 频繁的内存申请可以考虑使用内存池
* 减小加锁的粒度
* 可以适当增加io线程的数量
