# LinuxProgramming
Demonstration of the Linux programming interface

## I/O - io
**file-holes.c**
- Program like *cp* that copies a regular file that contains holes (sequence of null bytes) and also creates corresponding holes in the target file (without lseek() SEEK_DATA / SEEK_HOLE operations and truncate() / ftruncate() functions)

**atomic-append.c**
- Demonstration of getopt(), pread() and writev() performing gather output in program that atomically appends multiple files to a single file

## Access - access
**authentication.c**
- Demonstration of user authentication against the shadow password file using getpwnam(), getspnam(), getpass() and crypt(), compile program with the -lcrypt option (so it is linked against the crypt library)

