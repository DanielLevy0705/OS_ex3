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
#include <ctype.h>

//a function to clear a buffer
void clearBuff(char buff[]){
    int i;
    for(i=0;i<KILO_SIZE+1;i++){
        buff[i] = '\0';
    }
}
//a function to check the reading result.
int readCheck(int rd1, int rd2, int fl1, int fl2,
              int *rdMore1, int *rdMore2) {
    //if there was an error, print err msg
    if (rd1 < 0 || rd2 < 0) {
        write(2, ERR_MSG, strlen(ERR_MSG));
        return -1;
    }
    //check for both files if they have more to read.
    if (rd1 < KILO_SIZE)
        *rdMore1 = 0;
    if (rd2 < KILO_SIZE)
        *rdMore2 = 0;
    return 0;
}
//check if the char is a space, if it is return the space value, else return 0.
char isSpace(char c) {
    if (c == SPACE)
        return SPACE;
    if (c == '\n') {
        return '\n';
    }
    return 0;
}
//create a new string from the original buffer.
void newBuffs(int len,const char buf[],char newBuf[]){
    int i,k=0;
    //new string will be the original one with uppercase and no spaces.
    for(i=0;i<len;i++){
        if(!isSpace(buf[i])) {
            newBuf[k] = toupper(buf[i]);
            k++;
        }
    }
}

//a function to check if strings are similar or different.
int compareStrings(const char *buf1, const char *buf2,
                   int *rdMore1, int *rdMore2, int len1, int len2,int fd1,int fd2) {
    int run=1,rd1=0,rd2=0;
    int diff = 0;
    char temp[KILO_SIZE+1],newBuf1[KILO_SIZE+1],newBuf2[KILO_SIZE+1],newTemp[KILO_SIZE+1];
    clearBuff(newBuf1);
    clearBuff(newBuf2);
    clearBuff(temp);
    clearBuff(newTemp);
    newBuffs(len1,buf1,newBuf1);
    newBuffs(len2,buf2,newBuf2);
    while (run){
        //check the difference between the strings length.
        diff = strlen(newBuf1) - strlen(newBuf2);
        //create a new buffer with newBuffs function.
        if(diff>0){
            if(*rdMore1){
                clearBuff(temp);
                rd2 = read(fd2,temp,diff);
                newBuffs(rd2,temp,newTemp);
                strcat(newBuf2,newTemp);
                clearBuff(newTemp);
            }else{
                //if there is nothing to read the strings are different.
                return 2;
            }
            //create a new buffer with newBuffs function.
        }else if(diff<0){
            if(*rdMore2){
                clearBuff(temp);
                rd1 = read(fd1,temp,diff);
                newBuffs(rd1,temp,newTemp);
                strcat(newBuf1,newTemp);
                clearBuff(newTemp);
                //if there is nothing to read the strings are different.
            }else{
                //if the lengths are equal the strings are different.
                return 2;
            }
        }else{
            //if the new buffs are similar break the loop.
            int cmp = strcmp(newBuf1,newBuf2);
            if(cmp == 0)
                run=0;
        }
        int rdErr = readCheck(rd1, rd2, fd1, fd2, rdMore1, rdMore2);
        if (rdErr)
            return -1;
    }
    return 3;
}

int main(int argc, char *argv[]) {
    int res = 3, rd1 = 0, rd2 = 0, rdMore1 = 1, rdMore2 = 1;
    char buf1[KILO_SIZE+1];
    char buf2[KILO_SIZE+1];
    clearBuff(buf1);
    clearBuff(buf2);
    //open files.
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
    //a loop to read untill a diffrence or end of files.
    while (cmp == 0) {
        //if both files are at the end, the files are equal.
        if (!rdMore2 && !rdMore1){
            close(fl1);
            close(fl2);
            return 1;
        }
        //read if has more to read.
        if (rdMore1){
            clearBuff(buf1);
            rd1 = read(fl1, buf1, KILO_SIZE);
        }
        //read if has more to read.
        if (rdMore2){
            clearBuff(buf2);
            rd2 = read(fl2, buf2, KILO_SIZE);
        }
        //check the reading status.
        int rdErr = readCheck(rd1, rd2, fl1, fl2, &rdMore1, &rdMore2);
        if (rdErr){
            close(fl1);
            close(fl2);
            return -1;
        }
        cmp = strcmp(buf1, buf2);
    }
    //if we got here the files are different, check if they are similar.
    while (res == 3) {
        res = compareStrings(buf1, buf2, &rdMore1, &rdMore2, rd1, rd2,fl1,fl2);
        // if the files are different, break the loop.
        if(res == 2 || !(rdMore1 || rdMore2)){
            close(fl1);
            close(fl2);
            return res;
        }
        if(res==-1)
            break;
        //read if has more to read.
        if (rdMore1){
            clearBuff(buf1);
            rd1 = read(fl1, buf1, KILO_SIZE);
        }
        //read if has more to read.
        if (rdMore2){
            clearBuff(buf2);
            rd2 = read(fl2, buf2, KILO_SIZE);
        }
        //check the reading status.
        int rdErr = readCheck(rd1, rd2, fl1, fl2, &rdMore1, &rdMore2);
        if (rdErr){
            close(fl1);
            close(fl2);
            return -1;
        }
    }
    //close the files.
    close(fl1);
    close(fl2);
    return res;
}