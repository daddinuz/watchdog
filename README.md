Watchdog
=========

Watchdog is a C99-compliant runtime memory tracer library useful to find memory leaks or analyze memory usage.  
Basically Watchdog creates a layer over the C stdlib.h routines used to manage memory such as 'malloc'/'free' in order 
to maintain the whole "history" of your dynamic allocated variables with a small overhead over performances and memory.  
This feature may come in handy in development stage, during a memory leak hunting session, while analyzing the memory 
bottle-neck of you program, or for smaller applications, it can help to focus on coding leaving the boiler plate of 
memory de-allocation to the basic garbage collection utility that Watchdog provides. 

Below there is a memory report of the example program under the `examples/` folder, 
by now the only supported output format is YAML:
  
```bash
$ ./cmake-build-debug/main
"0x521ca90":
  status: leaked
  chunks:
    - href: /watchdog/examples/main.c:42
      file: /watchdog/examples/main.c
      line: 42
      size: 30
      call: realloc
    - href: /watchdog/examples/main.c:42
      file: /watchdog/examples/main.c
      line: 42
      size: 28
      call: realloc
    - href: /watchdog/examples/main.c:42
      file: /watchdog/examples/main.c
      line: 42
      size: 26
      call: realloc
    - href: /watchdog/examples/main.c:42
      file: /watchdog/examples/main.c
      line: 42
      size: 24
      call: realloc
    - href: /watchdog/examples/main.c:42
      file: /watchdog/examples/main.c
      line: 42
      size: 22
      call: realloc
    - href: /watchdog/examples/main.c:42
      file: /watchdog/examples/main.c
      line: 42
      size: 20
      call: realloc
    - href: /watchdog/examples/main.c:42
      file: /watchdog/examples/main.c
      line: 42
      size: 18
      call: realloc
    - href: /watchdog/examples/main.c:42
      file: /watchdog/examples/main.c
      line: 42
      size: 16
      call: realloc
    - href: /watchdog/examples/main.c:40
      file: /watchdog/examples/main.c
      line: 40
      size: 5
      call: realloc

"0x521c260":
  status: freed
  chunks:
    - href: /watchdog/examples/main.c:38
      file: /watchdog/examples/main.c
      line: 38
      size: 0
      call: free
    - href: /watchdog/examples/main.c:37
      file: /watchdog/examples/main.c
      line: 37
      size: 48
      call: realloc
    - href: /watchdog/examples/main.c:36
      file: /watchdog/examples/main.c
      line: 36
      size: 10
      call: calloc

"0x521c060":
  status: leaked
  chunks:
    - href: /watchdog/examples/main.c:33
      file: /watchdog/examples/main.c
      line: 33
      size: 8
      call: malloc
```

### Integrate into your code

Watchdog is designed to be really simple to integrate into your existing code.  
Basically you just have to include "watchdog.h" instead of "stdlib.h", if NDEBUG is defined watchdog is automatically 
disabled, so that programs will run with zero overhead using the standard C library in "stdlib.h". 
I strongly recommend to use Watchdog only in pre-production stages, so a basic real life integration would look like this:

```C
/**
 * Watchog will include <stdlib.h> itself
 */
#include <watchdog.h>

int main(int argc, char *argv[]) {

    /*
        Your unchanged code with mallocs, callocs, reallocs and free
     */

    return 0;
}
```
