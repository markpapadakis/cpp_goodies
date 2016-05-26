A collection of C++ classes; containers, algorithms implementations, etc. 

Starting off with an [std::deque<>](http://en.cppreference.com/w/cpp/container/deque) variant; a bounded FIFO queue optimized for adjusting its capacity after initialization for efficient reuse.
For example, you may be storing strings, and you may want to retain upto 10 now, and later on, for the same queue, you may want to retain 5. See `resetTo()` method.
