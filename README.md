Watchdog
=========

Watchdog is a C99-compliant memory tracer library useful to find memory leaks or analyze memory usage of your programs.  
Basically Watchdog creates a layer over the C stdlib.h routines used to manage memory such as 'malloc'/'free' in order 
to maintain the whole "history" of your dynamic allocated variables with a small overhead over performances and memory.
This feature may come in handy in development stage, during a memory leak hunting session, while analyzing the memory 
bottle-neck of you program, or for smaller applications, it can just help to focus on coding leaving the boiler plate of 
memory de-allocation to the basic garbage collection utility that Watchdog provides. 

This is the memory report of the example program under the examples/ folder:
  
```bash
$ ./bin/example
[WATCHDOG] INFO: Watchdog initialized
[WATCHDOG] INFO: summary at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0026
[WATCHDOG]       0 allocations, 0 frees
[WATCHDOG]       0 bytes allocated, 0 bytes freed
[WATCHDOG] INFO: malloc  at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0029
[WATCHDOG]       8 bytes allocated to address 0x1e81468
[WATCHDOG] INFO: summary at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0031
[WATCHDOG]          address 0x1e81468:
[WATCHDOG]                  malloc  at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0029 |  8 bytes currently allocated
[WATCHDOG]       1 allocations, 0 frees
[WATCHDOG]       8 bytes allocated, 0 bytes freed
[WATCHDOG] INFO: calloc  at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0033
[WATCHDOG]       10 bytes allocated to address 0x1e81558
[WATCHDOG] INFO: realloc at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0034
[WATCHDOG]       12 bytes reallocated from address 0x1e81558 to address 0x1e81558
[WATCHDOG] INFO: free    at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0035
[WATCHDOG]       12 bytes freed from address 0x1e81558
[WATCHDOG] INFO: summary at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0041
[WATCHDOG]          address 0x1e81468:
[WATCHDOG]                  malloc  at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0029 |  8 bytes currently allocated
[WATCHDOG]          address 0x1e81558:
[WATCHDOG]                  calloc  at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0033 | 10 bytes were in use
[WATCHDOG]                  realloc at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0034 | 12 bytes were in use
[WATCHDOG]                  free    at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0035 |  0 bytes currently allocated
[WATCHDOG]       2 allocations, 1 frees
[WATCHDOG]       30 bytes allocated, 22 bytes freed
[WATCHDOG] WARN: collect at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0042
[WATCHDOG]          address 0x1e81468:
[WATCHDOG]                  malloc  at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0029 |  8 bytes currently allocated
[WATCHDOG]       8 bytes collected
[WATCHDOG] INFO: Watchdog terminated
[WATCHDOG]       2 allocations, 2 frees
[WATCHDOG]       30 bytes allocated, 30 bytes freed
```

### Integrate with your code

Watchdog is designed to be really simple to import into your existing code.
Basically you just have to include "Watchdog.h" after "stdlib.h" and initialize the library in your main.
I strongly recommend to use Watchdog only in pre-production stages, so basically a real life integration would look like this:

```C
/**
 * Always include <Watchdog.h> after <stdlib.h>
 */
#include <stdlib.h>
#ifndef NDEBUG
#include <Watchdog/Watchdog.h>
#endif

int main(int argc, char *argv[]) {
#ifndef NDEBUG
    watchdog_initialize(stdout);    /* remember to initialize Watchdog as first thing */ 
    atexit(watchdog_terminate);     /* register a cleanup function that terminates Watchdog on exit */
#endif

    /*
        Your Code
     */

    return 0;
}
```
