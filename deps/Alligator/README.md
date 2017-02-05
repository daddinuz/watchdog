# Alligator

Alligator is a dumb memory allocator that will check for out of memory and will abort the execution if it happens.

> We've all been taught that when malloc returns 0, it means the machine ran out of memory.
> This case should be detected and "handled" by our application in some graceful manner.
> But what does "handled" mean here?
> How does an application recover from an out of memory (OOM) condition?
> And what about the increased code complexity of checking all those malloc return values and passing them around?

## The policies

It's safe to say there are three major policies for handling OOM:

- **Recovery**

    The recovery policy is the least commonly used because it's the most difficult to implement, and is highly domain-specific.
    This policy dictates that an application has to gracefully recover from an OOM condition.
    By "gracefully recover", we usually mean one or more of:

    * Release some resources and try again
    * Save the user's work and exit
    * Clean up temporary resources and exit

    To be certain that your application recovers correctly, you must be sure that the steps it takes don't require any more dynamic memory allocation.
    This sometimes isn't feasible and always difficult to implement correctly.
    Since C has no exceptions, memory allocation errors should be carefully propagated to the point where they can be recovered from, and this sometimes means multiple levels of function calls.

- **Abort**

    The abort policy is simple and familiar: when no memory is available, print a polite error message and exit (abort) the application.
    This is the most commonly used policy - most command-line tools and desktop applications use it.
    As a matter of fact, this policy is so common that most Unix programs use a gnulib library function xmalloc instead of malloc.
    When this function is called, its return value isn't checked, reducing the code's complexity.

- **Segfault**

    The segfault policy is the most simplistic of all: don't check the return value of malloc at all.
    In case of OOM, a NULL pointer will get dereferenced, so the program will die in a segmentation fault.
    If there are proponents to this policy, they'd probably say - "Why abort with an error message, when a segmentation fault would do? With a segfault, we can at least inspect the code dump and find out where the fault was".

## Conclusion

> IMHO the abort policy is the best approach.
> Wrap your allocation functions with some wrapper that aborts on OOM - this will save you a lot of error checking code in your main logic.
> The wrapper does more: it provides a viable path to scale up in the future, if required.
> Perhaps when your application grows more complex you'll want some kind of gentle recovery - if all the allocations in your application go through a wrapper, the change will be very easy to implement.


Source: Eli Bendersky - [Handling out of memory conditions in C](http://eli.thegreenplace.net/2009/10/30/handling-out-of-memory-conditions-in-c)
