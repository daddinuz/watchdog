Watchdog
=========

Watchdog is a C99-compliant memory tracer library useful to find memory leak 
or analyze memory usage of your programs at runtime.
Watchdog also provides some basic garbage collection utilities. 

```sh
$ ./bin/example 
[WATCHDOG] DUMP: 3 allocations, 1 reallocations, 1 frees
[WATCHDOG]       address 0x189d298:
[WATCHDOG]            malloc     at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:46):    6 bytes allocated
[WATCHDOG]            realloc    at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:51):   10 bytes allocated
[WATCHDOG]       address 0x189d388:
[WATCHDOG]            calloc     at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:47):    8 bytes allocated
[WATCHDOG]       address 0x189d4e8:
[WATCHDOG]            calloc     at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:55):   20 bytes allocated
[WATCHDOG]            free       at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:56):    0 bytes allocated
[WATCHDOG]       ---- 18 bytes currently allocated
[WATCHDOG] WARN: running garbage collection
[WATCHDOG]       address 0x189d298:
[WATCHDOG]            collecting  10 bytes
[WATCHDOG]       address 0x189d388:
[WATCHDOG]            collecting   8 bytes
[WATCHDOG]       address 0x189d4e8:
[WATCHDOG]            collecting   0 bytes
[WATCHDOG]       ---- 18 bytes collected
```

In the dump file under bin/ folder

```
[WATCHDOG] DUMP: 0 allocations, 0 reallocations, 0 frees
[WATCHDOG]       ---- 0 bytes currently allocated
[WATCHDOG] CALL: at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:46) -- malloc  : 6 bytes requested
[WATCHDOG] DONE: at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:46) -- malloc  : 6 bytes allocated to address 0x189d298
[WATCHDOG] CALL: at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:47) -- calloc  : 8 bytes requested
[WATCHDOG] DONE: at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:47) -- calloc  : 8 bytes allocated to address 0x189d388
[WATCHDOG] DUMP: 2 allocations, 0 reallocations, 0 frees
[WATCHDOG]       address 0x189d298:
[WATCHDOG]            malloc     at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:46):    6 bytes allocated
[WATCHDOG]       address 0x189d388:
[WATCHDOG]            calloc     at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:47):    8 bytes allocated
[WATCHDOG]       ---- 14 bytes currently allocated
[WATCHDOG] CALL: at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:51) -- realloc : 10 bytes requested
[WATCHDOG] DONE: at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:51) -- realloc : 10 bytes reallocated from address 0x189d298 to address 0x189d298
[WATCHDOG] DUMP: 2 allocations, 1 reallocations, 0 frees
[WATCHDOG]       address 0x189d298:
[WATCHDOG]            malloc     at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:46):    6 bytes allocated
[WATCHDOG]            realloc    at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:51):   10 bytes allocated
[WATCHDOG]       address 0x189d388:
[WATCHDOG]            calloc     at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:47):    8 bytes allocated
[WATCHDOG]       ---- 18 bytes currently allocated
[WATCHDOG] CALL: at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:55) -- calloc  : 20 bytes requested
[WATCHDOG] DONE: at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:55) -- calloc  : 20 bytes allocated to address 0x189d4e8
[WATCHDOG] CALL: at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:56) -- free    : address 0x189d4e8
[WATCHDOG] DUMP: 3 allocations, 1 reallocations, 1 frees
[WATCHDOG]       address 0x189d298:
[WATCHDOG]            malloc     at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:46):    6 bytes allocated
[WATCHDOG]            realloc    at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:51):   10 bytes allocated
[WATCHDOG]       address 0x189d388:
[WATCHDOG]            calloc     at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:47):    8 bytes allocated
[WATCHDOG]       address 0x189d4e8:
[WATCHDOG]            calloc     at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:55):   20 bytes allocated
[WATCHDOG]            free       at (/home/daddinuz/Workspace/C/Watchdog/examples/example.c:56):    0 bytes allocated
[WATCHDOG]       ---- 18 bytes currently allocated
```
