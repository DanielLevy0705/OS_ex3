/***
 * Name: Daniel Levy
 * ID: 318813714
 * Group: 06
 */
#define ERR_MSG "Error in system call"
#define KILO_SIZE 1024
#define SPACE 32

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int readCheck(int rd1,int rd2,int fl1,int fl2,
        int *rdMore1 ,int *rdMore2){
    if (rd1 < 0 || rd2 < 0) {
        write(2, ERR_MSG, strlen(ERR_MSG));
        close(fl1);
        close(fl2);
        return -1;
    }
    if(rd1<KILO_SIZE)
        *rdMore1=0;
    if(rd2<KILO_SIZE)
        *rdMore2=0;
    return 0;
}
char isSpace(char c){
    if(c == SPACE)
        return SPACE;
    if(c == '\n'){
        return '\n';
    }
    return 0;
}

int compareChars(char c1, char c2){
    if(c1 == c2)
        return 1;
    return 0;
}

int isLetter(char c){
    char a='a';
    char capA = 'A';
    char z = 'z';
    char capZ = 'Z';
    if((capA<=c && c<=capZ) ||(a<=c && c<= z))
        return 1;
    return 0;
}
/***
 *
 * @param buff1
 * @param buff2
 * @return
 */
int compareStrings(const char* buf1, const char* buf2,
        int rdMore1=1,int rdMore2=1,int len1,int len2){
    int i=0,j=0;
    while(i<len1,j<len2){
        i++;
        j++;
    }
}
int main(int argc, char *argv[]) {
    int res = 0, run = 1, rd1, rd2, rdMore1 = 1, rdMore2 = 1;
    char buf1[KILO_SIZE + 1];
    char buf2[KILO_SIZE + 1];
    if (argc != 3) {
        perror("not enough arguments.");
        return -1;
    }
    // TODO: take care of paths with spaces. path is max 150 chars.

    int fl1 = open(argv[1], O_RDONLY);
    if (fl1 < 0) {
        write(2, ERR_MSG, strlen(ERR_MSG));
        return -1;
    }
    int fl2 = open(argv[2], O_RDONLY);
    if (fl2 < 0) {
        write(2, ERR_MSG, strlen(ERR_MSG));
        close(fl1);
        return -1;
    }
    int cmp = 0;
    while(!cmp){
        if(!rdMore2 && !rdMore1)
            return 1;
        if(rdMore1)
            rd1 = read(fl1, buf1, KILO_SIZE);
        if(rdMore2)
            rd2 = read(fl2, buf2, KILO_SIZE);
        int rdErr = readCheck(rd1,rd2,fl1,fl2,&rdMore1,&rdMore2);
        if(rdErr)
            return -1;
        cmp = strcmp(buf1,buf2);
    }
    //a loop to read untill a diffrence or end of files.
    while (run) {

        if (rdMore1)
            rd1 = read(fl1, buf1, KILO_SIZE);
        if (rdMore2)
            rd2 = read(fl2, buf2, KILO_SIZE);
        int rdErr = readCheck(rd1,rd2,fl1,fl2,&rdMore1,&rdMore2);
        if(rdErr)
            return -1;
        res = compareStrings(buf1,buf2,rdMore1,rdMore2,rd1,rd2);
    }
    //close the files.
    close(fl1);
    close(fl2);
    return res;
}