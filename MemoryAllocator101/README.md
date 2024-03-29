March 29, 2024: Memory Allocators 101 - Write a simple memory allocator
https://arjunsreedharan.org/post/148675821737/memory-allocators-101-write-a-simple-memory

Compile and Run
gcc -o main.so -fPIC -shared main.c

The -fPIC and -shared options makes sure the compiled output has position-independent code and tells the linker to produce a shared object suitable for dynamic linking.

On Linux, if you set the enivornment variable LD_PRELOAD to the path of a shared object, that file will be loaded before any other library. We could use this trick to load our compiled library file first, so that the later commands run in the shell will use our malloc(), free(), calloc() and realloc().

export LD_PRELOAD=$PWD/main.so
Now, if you run something like ls, it will use our memory allocator.

ls
main.c		main.so		README.md
or

vim main.c
You can also run your own programs with this memory allocator.

Once you're done experimenting, you can do unset LD_PRELOAD to stop using our allocator.
