# Project-3-Shell-with-process-scheduler

For CST-315
Improved shell with better file system navigation and background and foreground process scheduling

To compile: gcc shellwithsched.c

To run: ./a.out

Available commands in the shell:

exit

pwd

cd

help function is currently broken, this will cause errors but the shell will still work

>

batch mode is functioning

this shell uses foreground-background scheduling making it simple priority based: every process will enter the forground list for execution and wait there a small period of time, after this time if the process has not completed then it will enter the background waiting list and will only execute when the foreground list is empty.
