#include <stdio.h>
#include <unistd.h>

/*
 * Environment setup:
 * 
 *      sudo mkdir /tmp/root/a
 *      sudo gcc setuid.c
 *
 * Execute:
 *
 *      $ ./a.out
 *      ruid 501, euid 501
 *      Couldn't not open file: Permission denied
 *
 *      $ sudo chmod u+s a.out
 *      $ ./a.out
 *      ruid 501, euid 0
 *      Opened root file
 *
 */

int main() {
    int status = 1;
    FILE* fp;
    int ruid, euid;

    ruid = getuid();
    euid = geteuid();

    printf("ruid %d, euid %d\n", ruid, euid);

    fp = fopen("/tmp/root/a", "w");
    if (fp == NULL) {
        perror("Couldn't not open file");
        return -1;
    }

    printf("Opened root file\n");
    fclose(fp);

    return 0;
}
