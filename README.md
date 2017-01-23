Watchdog
=========


Watchdog is a C99-compliant memory tracer library useful to find memory leak 
or analyze memory usage of your programs at runtime.
Watchdog also provides some basic garbage collection utilities. 

```bash
$ ./bin/example 
[WATCHDOG] INFO: Watchdog initialized
[WATCHDOG] INFO: summary at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0026
[WATCHDOG]       0 allocations, 0 frees
[WATCHDOG]       0 bytes allocated, 0 bytes freed
[WATCHDOG] INFO: malloc  at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0029
[WATCHDOG]       8 bytes allocated to address 0x1574468
[WATCHDOG] INFO: summary at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0031
[WATCHDOG]          address 0x1574468:
[WATCHDOG]                  malloc  at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0029 |  8 bytes currently allocated
[WATCHDOG]       1 allocations, 0 frees
[WATCHDOG]       8 bytes allocated, 0 bytes freed
[WATCHDOG] INFO: calloc  at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0033
[WATCHDOG]       10 bytes allocated to address 0x1574558
[WATCHDOG] INFO: realloc at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0034
[WATCHDOG]       12 bytes reallocated from address 0x1574558 to address 0x1574558
[WATCHDOG] INFO: free    at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0035
[WATCHDOG]       12 bytes freed from address 0x1574558
[WATCHDOG] INFO: summary at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0041
[WATCHDOG]          address 0x1574468:
[WATCHDOG]                  malloc  at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0029 |  8 bytes currently allocated
[WATCHDOG]          address 0x1574558:
[WATCHDOG]                  calloc  at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0033 | 10 bytes were in use
[WATCHDOG]                  realloc at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0034 | 12 bytes were in use
[WATCHDOG]                  free    at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0035 |  0 bytes currently allocated
[WATCHDOG]       2 allocations, 1 frees
[WATCHDOG]       30 bytes allocated, 22 bytes freed
[WATCHDOG] WARN: collect at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0042
[WATCHDOG]          address 0x1574468:
[WATCHDOG]                  malloc  at /home/daddinuz/Workspace/C/Watchdog/examples/example.c:0029 |  8 bytes currently allocated
[WATCHDOG]       8 bytes collected
[WATCHDOG] INFO: Watchdog terminated
[WATCHDOG]       2 allocations, 2 frees
[WATCHDOG]       30 bytes allocated, 30 bytes freed
```
