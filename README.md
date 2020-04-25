# LinuxProgramming
Demonstration of the Linux programming interface

## I/O - io
**file-holes.c**
- Program like *cp* that copies a regular file that contains holes (sequence of null bytes) and also creates corresponding holes in the target file (without lseek() SEEK_DATA / SEEK_HOLE operations and truncate() / ftruncate() functions)

**atomic-append.c**
- Demonstration of pread() and writev() performing gather output in program that atomically appends multiple files to a single file

## Access - access
**authentication.c**
- Demonstration of user authentication against the shadow password file using getpwnam(), getspnam(), getpass() and crypt(), compile program with the -lcrypt option (so it is linked against the crypt library)

## Time - time
**time_and_timezone.c**
- Program displaying calendar time in seconds and microseconds since Epoch, local time or current time in time zone specified in argument (example: ":Pacific/Auckland"), demonstration of gettimeofday(), time(), localtime(), asctime() and setenv() functions

## System - system
**information-and-limits.c**
- Program displaying system information and limits

## File - file
**file-attributes.c**
- Program demonstrating system calls chmod() for changing file permissions and ioctl() with FS_IOC_SETFLAGS flag (equivalent to chattr command) for changing file i-node flags

## Directory - directory
**directory-management.c**
- Program demonstrating mkdir() and rmdir() commands and directory iteration

## Monitoring - monitoring
**monitoring.c**
- Program that recursively monitors and logs all file/directory events using tree walking function nftw() and inotify
API

## Signal - signal
**signal.c**
- Program that fetches and sends pending standard/realtime signals using sigprocmask(), sigwaitinfo(), sigqueue() and kill()

## Timer - timer
**timerfd.c**
- Program demonstrating timer using file descriptor with Linux-specific timerfd
 facility

## Process - process
**synchronized-process.c**
- Program demonstrating process synchronization using signals to avoid race conditions with fork(), sigprocmask(), sigaction() and sigsuspend()

