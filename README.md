# LinuxProgramming
Demonstration of the Linux programming interface

## I/O - io
**file-holes.c**
- Program like *cp* that copies a regular file that contains holes (sequence of null bytes) and also creates corresponding holes in the target file (without lseek() SEEK_DATA / SEEK_HOLE operations and truncate() / ftruncate() functions)

**atomic-append.c**
- Demonstration of getopt() and writev() in program that atomically appends multiple files to a single file

