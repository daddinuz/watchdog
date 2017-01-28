Watchdog
=========

Watchdog is a C99-compliant memory tracer library useful to find memory leaks or analyze memory usage.  
Basically Watchdog creates a layer over the C stdlib.h routines used to manage memory such as 'malloc'/'free' in order 
to maintain the whole "history" of your dynamic allocated variables with a small overhead over performances and memory.  
This feature may come in handy in development stage, during a memory leak hunting session, while analyzing the memory 
bottle-neck of you program, or for smaller applications, it can help to focus on coding leaving the boiler plate of 
memory de-allocation to the basic garbage collection utility that Watchdog provides. 

This is the memory report of the example program under the `examples/` folder:
  
```bash
$ ./bin/example
[WATCHDOG] INFO: Watchdog Initialized
[WATCHDOG] INFO: malloc  at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0020
[WATCHDOG]       8 bytes allocated to address 0x9c4038
[WATCHDOG] INFO: calloc  at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0023
[WATCHDOG]       10 bytes allocated to address 0x9c4108
[WATCHDOG] INFO: realloc at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0024
[WATCHDOG]       12 bytes reallocated from address 0x9c4108 to address 0x9c4108
[WATCHDOG] INFO: free    at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0025
[WATCHDOG]       12 bytes freed from address 0x9c4108
[WATCHDOG] WARN: exit    at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0028
[WATCHDOG]       exit code: 0
[WATCHDOG] WARN: Garbage Collector
[WATCHDOG]          address 0x9c4038:
[WATCHDOG]                  malloc  at  /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0020 |  8 bytes still allocated
[WATCHDOG]       8 bytes collected
[WATCHDOG] INFO: Report
[WATCHDOG]          address 0x9c4038:
[WATCHDOG]                  malloc  at  /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0020 |  8 bytes were in use
[WATCHDOG]                  free    at                                     <garbage collector>:0000 |  0 bytes currently allocated
[WATCHDOG]          address 0x9c4108:
[WATCHDOG]                  calloc  at  /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0023 | 10 bytes were in use
[WATCHDOG]                  realloc at  /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0024 | 12 bytes were in use
[WATCHDOG]                  free    at  /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0025 |  0 bytes currently allocated
[WATCHDOG]       2 allocations, 2 frees
[WATCHDOG]       30 bytes allocated, 30 bytes freed (whereof 8 bytes collected on exit)
[WATCHDOG] INFO: Watchdog Terminated
```

### Integrate with your code

Watchdog is designed to be really simple to integrate into your existing code.  
Basically you just have to include "Watchdog.h" after "stdlib.h".  
I strongly recommend to use Watchdog only in pre-production stages, so a basic real life integration would look like this:

```C
/**
 * Always include <Watchdog.h> after <stdlib.h>
 */
#include <stdlib.h>
#ifndef NDEBUG
#include <Watchdog/Watchdog.h>
#endif

int main(int argc, char *argv[]) {

    /*
        Your Code
     */

    return 0;
}
```
