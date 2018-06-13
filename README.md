Watchdog
=========

Watchdog is a C99-compliant runtime dynamically-allocated memory-tracer library that may come in handy at the 
development stage, during a memory leak hunting session, or while analyzing the memory bottle-neck of you program.

### How does it work?

Watchdog fulfills only the following task:

 * log every usage of heap memory while the program is running.

This allows, with a small overhead over performances, to maintain the whole "history" of the dynamic memory usage, that can be analyzed in a separate stage.  
At this point one can freely analyze the "history" by itself, alternatively [watchdog analyzer](https://github.com/daddinuz/watchdog_analyzer "watchdog_analyzer") can be used to ease this task.

### How to integrate?

Watchdog is designed to be integrated simply into the existing code.  
One should just include "watchdog.h" instead of "stdlib.h" into the files that need to be traced.

Watchdog does not trace external libraries, it only traces those ones in which it is included.

### How to turn it off?
 
If NDEBUG is defined, watchdog is automatically disabled so that programs will run with zero overhead, 
using the standard allocators in "stdlib.h". 

### Recommendations

It is strongly recommended to use Watchdog only in pre-production stages.

Useful links: 
 * [Analyzer and TUI viewer](https://github.com/daddinuz/watchdog "watchdog") for watchdog reports.
