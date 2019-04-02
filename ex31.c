/***
 * Name: Daniel Levy
 * ID: 318813714
 * Group: 06
 */
#include <stdio.h>
#include <unistd.h>
#include <syscall.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    if(argc<3){
        perror("not enough arguments");
        return 1;
    }
    int fl1 = open(argv[1],O_RDONLY);
    int fl2 = open(argv[2],O_RDONLY);
    return 0;
}