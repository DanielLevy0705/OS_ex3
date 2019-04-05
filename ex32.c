/***
 * Name: Daniel Levy
 * ID: 318813714
 * Group: 06
 */

#define ERR_MSG "Error in system call"
#define MAX_ROW_LEN 150
#define NO_C ",0,NO_C_FILE\n"
#define COMP_ERR ",20,COMPILATION_ERROR\n"
#define TIMEOUT ",40,TIMEOUT\n"
#define BAD_OTPT ",60,BAD_OUTPUT\n"
#define SIM_OTPT ",80,SIMILAR_OUTPUT\n"
#define GERAT_JOB ",100,GREAT_JOB\n"
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <wait.h>

void clearBuffWithSize(char buff[],int size){
    int i;
    for(i=0;i<size;i++){
        buff[i] = '\0';
    }
}
int executeFile(char input[],char output[],
        struct dirent* pDirent,int myOtptFd,char user[],int resFd,char pathName[]){
    int i,status;
    char *args1[] = {"gcc",pathName,NULL};
    char *args2[] = {"./comp.out",input,output,NULL};

    int pid = fork();
    if(pid == -1){
        return -1;
    }else if (pid>0){
        // TODO:
        // waitpid(pid,&status,WNOHANG)
        // if(status == -1){
        // strcat(user,COMP_ERR)
        // write to result file that comp error has occured.
        // return 1}

        for(i=0;i<5;i++){
            if(waitpid(pid,NULL,WNOHANG)!=0){
                break;
            }
            sleep(1);
        }
        if(i==5){
            strcat(user,TIMEOUT);
            write(resFd,user,strlen(user));
            return 1;
        }else{
            pid = fork();
            if(pid == -1)
                return -1;
            else if(pid>0){
                waitpid(pid,&status,NULL);
                switch(status){
                    case 1:
                        //TODO: write to results: great job.
                        break;
                    case 2:
                        //TODO: write to results: bad output.
                        break;
                    case 3:
                        //TODO: write to results: similar output.
                        break;
                    default:
                        return -1;
                }
            }else if(pid == 0){
                //TODO: need to dup input and output
                execvp(args2[0],args2);
                return -1;
            }
        }
    }else if (pid==0){
        //TODO: need to dup input and output
        execvp(args1[0],args1);
        return -1;
    }
}


int dirActions(char dir[],char input[],
        char output[],DIR* pDir,struct dirent* pDirent,char user[],int resFd){
    int res = 0;
    char myOtpt[MAX_ROW_LEN+1],name[MAX_ROW_LEN +1],pathName[MAX_ROW_LEN + 1];
    clearBuffWithSize(myOtpt,MAX_ROW_LEN+1);
    clearBuffWithSize(name,MAX_ROW_LEN+1);
    clearBuffWithSize(pathName,MAX_ROW_LEN+1);
    strcpy(myOtpt,dir);
    strcat(myOtpt,"/myOtpt.txt");
    int myOtptFd = open(myOtpt,O_CREAT|O_WRONLY|O_RDONLY|O_APPEND);

    while(pDirent!=NULL){
        struct stat st;
        strcpy(name,pDirent->d_name);
        strcpy(pathName,dir);
        strcat(pathName,"/");
        strcat(pathName,name);
        if(strlen(name)>2){
            if(name[strlen(name)-1] == 'c' && name[strlen(name)-2] == '.'){
                res = executeFile(input,output,pDirent,myOtptFd,user,resFd,pathName);
                if(res == -1){
                    unlink(myOtpt);
                    return -1;
                }
            }
        }
        if(stat(pathName,&st)==0 && S_ISDIR(st.st_mode)){
            pDir=opendir(pathName);
            if(pDir==NULL){
                unlink(myOtpt);
                return -1;
            }
            res = dirActions(pathName,input,output,pDir,pDirent,user,resFd);
        }
        pDirent = readdir(pDir);
    }
    unlink(myOtpt);
    return res;
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
    char cwd[MAX_ROW_LEN+1];
    clearBuffWithSize(cwd,MAX_ROW_LEN+1);
    getcwd(cwd,sizeof(cwd));
    strcat(cwd,"/results.csv");
    int resFd = open(cwd,O_CREAT|O_WRONLY|O_RDONLY|O_APPEND);
    //run for every user in directory
    while(pDirent!=NULL){
        int res = dirActions(confRow1,confRow2,
                confRow3,pDir,pDirent,pDirent->d_name,resFd);
        if(res == 0){
            //write to result file that the result is 0.
        }else if(res == -1){
            write(2,ERR_MSG,strlen(ERR_MSG));
            closedir(pDir);
            close(resFd);
            close(fdConf);
            return -1;
        }
        pDirent=readdir(pDir);
    }
    closedir(pDir);
    close(resFd);
    close(fdConf);
    return 0;
}