/***  README FOR CSE 506 PROJECT - PREEMPTIVE OS ***/

                *********************   BLEH OS   *************************

1. BLEH is a basic preemptive multitasking Operating System.

2. Supports Terminal Handling, VFS access and 13 different syscalls, brackets indicating syscall numbers, namely read(0), write(1),   fork(2), execve(3), wait(4), exit(5), open(6), close(7), getdents(8), getprocs64(9), mmap(10), sleep(11), wait(12), munmap(13).

3. Syscalls use SWAPGS and SYSCALL instruction, instead of INT x80

4. Terminal supports Backspaces (please use Ctrl + H to perform a backspace, not the normal backspace key press)

5. Page Fault handlers are performing on-demand paging - implemented 3 different page fault handlers dealing with each of File-backed page faults, Anonymous VMA based page faults and Copy-On-Write page faults.

6. Supports - ls, cat, echo, sleep, kill, ps.

7. Since there is no support for cd, ls will display files from the current working directory which is the root directory always.

9. sleep will perform timeouts for values expressed in seconds

10. ps, will display only currently active processes, and the zombie ones.

11. Future plans include -- supporting on-disk RW file system and handling signals, along with syscall support for important binaries like cd.


