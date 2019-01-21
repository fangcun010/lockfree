# lockfree
lock-free library

lock_free_queue.hpp

Compiled under the mingw64
********You need to implement your own memory manager********


********Or you will get memory leak**************************

无锁队列ming64实现，使用时需要定义自己的内存管理器，构造一个free list。

这并不是bug，而是这个算法设计问题，java的无锁队列使用的就是这一算法。

相关文献：

https://zhuanlan.zhihu.com/p/54690219

http://blog.shealevy.com/2015/04/23/use-after-free-bug-in-maged-m-michael-and-michael-l-scotts-non-blocking-concurrent-queue-algorithm/

https://stackoverflow.com/questions/1061889/lock-free-memory-reclamation-with-64bit-pointers

https://stackoverflow.com/questions/4825400/cmpxchg16b-correct

https://stackoverflow.com/questions/7890601/circular-buffer-vs-lock-free-stack-to-implement-a-free-list
