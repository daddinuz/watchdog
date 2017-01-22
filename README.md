Watchdog
=========

Watchdog is a C99-compliant memory tracer library useful to find memory leak 
or analyze memory usage of your programs at runtime.
Watchdog also provides some basic garbage collection utilities. 

```sh
$ ./bin/example 
[MEMORY] DUMP: 3 allocations, 1 reallocations, 1 frees
[MEMORY]          at 0xc04278 in (example.c:51):  10 bytes allocated
[MEMORY]          at 0xc042b8 in (example.c:47):   8 bytes allocated
[MEMORY]       HEAP: 18 bytes allocated
[MEMORY] WARN: running garbage collection
[MEMORY]          at 0xc04278 in (example.c:51):  10 bytes allocated
[MEMORY]          at 0xc042b8 in (example.c:47):   8 bytes allocated
[MEMORY]       HEAP: 18 bytes collected
```

Example dump file

```
[MEMORY] DUMP: 0 allocations, 0 reallocations, 0 frees
[MEMORY]       HEAP: 0 bytes allocated
[MEMORY] DUMP: 2 allocations, 0 reallocations, 0 frees
[MEMORY]          at 0xc042b8 in (example.c:47):   8 bytes allocated
[MEMORY]          at 0xc04278 in (example.c:46):   6 bytes allocated
[MEMORY]       HEAP: 14 bytes allocated
[MEMORY] DUMP: 2 allocations, 1 reallocations, 0 frees
[MEMORY]          at 0xc04278 in (example.c:51):  10 bytes allocated
[MEMORY]          at 0xc042b8 in (example.c:47):   8 bytes allocated
[MEMORY]       HEAP: 18 bytes allocated
[MEMORY] DUMP: 3 allocations, 1 reallocations, 1 frees
[MEMORY]          at 0xc04278 in (example.c:51):  10 bytes allocated
[MEMORY]          at 0xc042b8 in (example.c:47):   8 bytes allocated
[MEMORY]       HEAP: 18 bytes allocated
```
