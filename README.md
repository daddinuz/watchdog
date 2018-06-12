Watchdog
=========

Watchdog is a C99-compliant runtime memory tracer library useful to find memory leaks or analyze memory usage of a 
specific module or even an entire application.  
Basically Watchdog creates a layer over the C stdlib.h routines used to manage memory such as *malloc*/*free* in order 
to maintain the whole "history" of your dynamic allocated variables with a small overhead over performances.
This feature may come in handy in development stage, during a memory leak hunting session, while analyzing the memory 
bottle-neck of you program.

### Integrate into your code

Watchdog is designed to be really simple to integrate into your existing code, basically you just have to include 
"watchdog.h" instead of "stdlib.h", if NDEBUG is defined watchdog is automatically disabled, so that programs will run 
with zero overhead using the standard C library allocators in "stdlib.h". 

It's strongly recommend to use Watchdog only in pre-production stages.
