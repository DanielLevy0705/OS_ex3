#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>

/***
 * Name: Daniel Levy
 * ID: 318813714
 * Group: 06
 */
int main(int argc, char* argv[]){
    if(argc!=2){
        perror("not enough arguments.");
    }
    int fdConf = open(argv[1],O_RDONLY);
}