# Colt

Colt is a C archive of generic collections such as Lists, Vector, Queue, Stack ...

# Building

Colt uses **CMake version 3.0 or higher** as default build system.  
In order to build the project manually open a shell in the root of the repository and run the following commands:

```bash
mkdir build
cd build
cmake ..
make
```

This will build the all the static archives under lib/ folder and all the tests under bin/ folder.  
You can then include in your project just the collections you need (linking the corresponding archives)  
or the whole library (linking *libcolt*).

Otherwise have a look to the [Testing](#testing) section for an automated build that will also run the tests for you.

# Testing

Tests are made using **googletest** which is already included in the repository.  
In order to extract the code coverage **gcov** and **lcov** are required.

To run the coverage (assuming the shell is opened in the repository root):

```bash
./coverage.sh
```

This will build the whole project, run all the tests and collect the code coverage.
