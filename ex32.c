/***
 * Name: Daniel Levy
 * ID: 318813714
 * Group: 06
 */

#define ERR_MSG "Error in system call"
#define MAX_ROW_LEN 150

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>

void clearBuffWithSize(char buff[],int size){
    int i;
    for(i=0;i<size;i++){
        buff[i] = '\0';
    }
}

void cd(char path[]){
    char cwd[MAX_ROW_LEN+1];
    clearBuffWithSize(cwd,MAX_ROW_LEN+1);
    int ret=0;
    //get the working directory and concatenate it with the path, if needed.
    if (path[0] != '/') {
        getcwd(cwd, sizeof(cwd));
        strcat(cwd, "/");
        strcat(cwd, path);
        ret = chdir(cwd);
    } else {
        ret = chdir(path);
    }
    //if the return value isn't zero, there was an error.
    if (ret) {
        fprintf(stderr, "Error in cd command\n");
    }
}

int main(int argc, char* argv[]){
    if(argc < 2){
        perror("not enough arguments.");
    }
    int fdConf = open(argv[1],O_RDONLY);
    if(fdConf == -1){
        write(2,ERR_MSG,strlen(ERR_MSG));
    }
    int oneByte=1,rowFlag = 1,rd,i1=0,i2=0,i3=0;
    char readByte[oneByte +1];
    clearBuffWithSize(readByte,oneByte+1);
    char confRow1[MAX_ROW_LEN + 1],confRow2[MAX_ROW_LEN + 1],confRow3[MAX_ROW_LEN + 1];
    clearBuffWithSize(confRow1,MAX_ROW_LEN+1);
    clearBuffWithSize(confRow2,MAX_ROW_LEN+1);
    clearBuffWithSize(confRow3,MAX_ROW_LEN+1);
    rd = read(fdConf,readByte,oneByte);
    // a loop to read every row of conf file to a different buffer.
    while(rowFlag!=0){
        if(rd == -1)
            rowFlag=-1;
        //switch the flag to know which buffer to fill or if an error has occurred.
        switch(rowFlag){
            case 1:
                if (readByte[0] == '\n'){
                    rowFlag = 2;
                    rd = read(fdConf,readByte,oneByte);
                    continue;
                }
                //if(readByte[0] != '\"'){
                confRow1[i1] = readByte[0];
                i1++;

                //}
                break;
            case 2:
                if (readByte[0] == '\n'){
                    rowFlag = 3;
                    rd = read(fdConf,readByte,oneByte);
                    continue;
                }
                //if(readByte[0] != '\"'){
                confRow2[i2] = readByte[0];
                i2++;

                //}
                break;
            case 3:
                //if its end of line or end of file break the loop.
                if (readByte[0] == '\n' || rd == 0)
                {
                    rowFlag = 0;
                    break;
                }
                //if(readByte[0] != '\"'){
                confRow3[i3] = readByte[0];
                i3++;
                //}
                break;
                //default case is an error. finish program properly.
            default:
                write(2,ERR_MSG,strlen(ERR_MSG));
                close(fdConf);
                return -1;
        }
        rd = read(fdConf,readByte,oneByte);
    } // end of while loop
    DIR* pDir;
    struct dirent* pDirent;
    pDir=opendir(confRow1);
    if(pDir==NULL){
        write(2,ERR_MSG,strlen(ERR_MSG));
        close(fdConf);
        return -1;
    }
    pDirent=readdir(pDir);
    while(pDirent!=NULL){
        printf("%s\n",pDirent->d_name);
        pDirent=readdir(pDir);
    }
    closedir(pDir);
    close(fdConf);
    return 0;
}