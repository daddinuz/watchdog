Watchdog
=========

Watchdog is a C99-compliant runtime dynamically allocated memory tracer library that may come in handy in 
development stage, during a memory leak hunting session, while analyzing the memory bottle-neck of you program.

### How it works?

watchdog only fulfills the following task:

 * log every usage of heap memory while the program is running.

This allow, with a small overhead over performances, to maintain the whole "history" of the dynamic memory usage, that can be analyzed in a separate stage.
At this point one can freely analyze the "history" by itself, alternatively [watchdog analyzer](https://github.com/daddinuz/watchdog_analyzer "watchdog_analyzer") can be use in order to ease this task.

### How to integrate?

Watchdog is designed to be really simple to integrate into existing code, the only thing that must be done is to include "watchdog.h" instead of "stdlib.h".

### How to turn it off?
 
If NDEBUG is defined watchdog is automatically disabled, so that programs will run with zero overhead, 
using the standard C library allocators in "stdlib.h". 

### Recommendations

It's strongly recommend to use Watchdog only in pre-production stages.

Useful links: 
 * [Analyzer and TUI viewer](https://github.com/daddinuz/watchdog "watchdog") for watchdog reports.
