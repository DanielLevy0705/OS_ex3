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
#include <time.h>
#include <stdlib.h>
// a function to clear buffer.
void clearBuffWithSize(char buff[], int size) {
    int i;
    for (i = 0; i < size; i++) {
        buff[i] = '\0';
    }
}
//a function to copmile and execute c file if it exists.
int executeFile(char input[], char output[],
                char user[], int resFd, char pathName[], char myOtpt[]) {
    int status;
    char *args1[] = {"gcc", pathName, NULL};
    char *args2[] = {"./a.out", NULL};
    char *args3[] = {"./comp.out", output, myOtpt, NULL};
    int iptFd = open(input, O_RDONLY);
    int myOtptFd = open(myOtpt, O_CREAT | O_RDWR,
                        S_IRWXG | S_IRWXO | S_IRWXU);
    //check the time now
    time_t timeBfr, timeAftr;
    struct tm *tmInfBfr, *tmInfAftr;
    time(&timeBfr);
    tmInfBfr = localtime(&timeBfr);
    // get the time in seconds.
    int bfr = tmInfBfr->tm_sec;
    int pid = fork();
    if (pid == -1) {
        unlink(myOtpt);
        close(iptFd);
        return -1;
    } else if (pid > 0) {
        //compile the file in the child and wait for the process to return.
        waitpid(pid, &status, 0);
        status = WEXITSTATUS(status);
        //if the exit status is 1 there was a compilation error.
        if (status == 1) {
            strcat(user, COMP_ERR);
            write(resFd, user, strlen(user));
            unlink(myOtpt);
            close(iptFd);
            return 1;
        } else {
            pid = fork();
            if (pid == -1) {
                close(iptFd);
                unlink(myOtpt);
                return -1;
            } else if (pid > 0) {
                //while the child is still running
                while (waitpid(pid, &status, WNOHANG) != pid) {
                    //check if the time passed is over 5 seconds.
                    time(&timeAftr);
                    tmInfAftr = localtime(&timeAftr);
                    //if its over 5 there was a timeout.
                    if (abs(bfr - tmInfAftr->tm_sec) > 5) {
                        strcat(user, TIMEOUT);
                        write(resFd, user, strlen(user));
                        unlink(myOtpt);
                        close(iptFd);
                        return 1;
                    }
                }
                close(iptFd);
                pid = fork();
                if (pid == -1) {
                    unlink(myOtpt);
                    return -1;
                } else if (pid > 0) {
                    //get the exit status
                    waitpid(pid, &status, 0);
                    int retVal = WEXITSTATUS(status);
                    //for each case write the correct result.
                    switch (retVal) {
                        case 1:
                            strcat(user, GERAT_JOB);
                            write(resFd, user, strlen(user));
                            break;
                        case 2:
                            strcat(user, BAD_OTPT);
                            write(resFd, user, strlen(user));
                            break;
                        case 3:
                            strcat(user, SIM_OTPT);
                            write(resFd, user, strlen(user));
                            break;
                        default:
                            unlink(myOtpt);
                            return -1;
                    }
                } else if (pid == 0) {
                    execvp(args3[0], args3);
                    write(2, ERR_MSG, strlen(ERR_MSG));
                    _exit(1);
                }
            } else if (pid == 0) {
                //dup input and output because the c file use stdin and stdout.
                dup2(iptFd, 0);
                dup2(myOtptFd, 1);
                execvp(args2[0], args2);
                write(2, ERR_MSG, strlen(ERR_MSG));
                _exit(1);
            }
        }
    } else if (pid == 0) {
        execvp(args1[0], args1);
        write(2, ERR_MSG, strlen(ERR_MSG));
        _exit(1);
    }
    unlink(myOtpt);
    return 1;
}
// a recursive function to look for a c file in all directories
int dirActions(char dir[], char input[],
               char output[], DIR *pDir,
               struct dirent *pDirent, char user[],
                       int resFd, char myOtpt[],char origDir[]) {
    int res = 0;
    char name[MAX_ROW_LEN + 1], pathName[MAX_ROW_LEN + 1];
    clearBuffWithSize(name, MAX_ROW_LEN + 1);
    clearBuffWithSize(pathName, MAX_ROW_LEN + 1);
    struct stat statbuf;
    DIR * newPdir;
    //run for every file in the directory.
    while (pDirent != NULL) {
        //continue if the file is . or  ..
        if (strcmp(pDirent->d_name, ".") == 0 || strcmp(pDirent->d_name, "..") == 0) {
            pDirent = readdir(pDir);
            continue;
        }
        //get the name of the file and the path to the file.
        strcpy(name, pDirent->d_name);
        strcpy(pathName, dir);
        strcat(pathName, "/");
        strcat(pathName, name);
        //if its a c file, call executeFile.
        if (strlen(name) > 2) {
            if (name[strlen(name) - 1] == 'c' && name[strlen(name) - 2] == '.') {
                res = executeFile(input, output, user, resFd, pathName, myOtpt);
                return res;
            }
        }
        //if the stat of the path is negative the path is wrong.
        int st = stat(pathName, &statbuf);
        if (st < 0) {
            return -1;
        }
        //if the path is of a directory call the function recursively.
        if (S_ISDIR(statbuf.st_mode)) {
            newPdir = opendir(pathName);
            if (pDir == NULL) {
                return -1;
            }
            pDirent = readdir(newPdir);
            res = dirActions(pathName, input, output, newPdir, pDirent, user, resFd, myOtpt,origDir);
            closedir(newPdir);
            //if we returned to the user dir, return the result.
            if(dir == origDir){
                return res;
            }
        }
        //if we found a c file, return all the way up the recursion.
        if(res){
            return res;
        }
        pDirent = readdir(pDir);
    }
    return res;
}

int main(int argc, char *argv[]) {
    //open configuration file.
    int fdConf = open(argv[1], O_RDONLY);
    if (fdConf == -1) {
        write(2, ERR_MSG, strlen(ERR_MSG));
        return -1;
    }
    int oneByte = 1, rowFlag = 1, rd, i1 = 0, i2 = 0, i3 = 0;
    char readByte[oneByte + 1];
    clearBuffWithSize(readByte, oneByte + 1);
    char confRow1[MAX_ROW_LEN + 1], confRow2[MAX_ROW_LEN + 1], confRow3[MAX_ROW_LEN + 1];
    clearBuffWithSize(confRow1, MAX_ROW_LEN + 1);
    clearBuffWithSize(confRow2, MAX_ROW_LEN + 1);
    clearBuffWithSize(confRow3, MAX_ROW_LEN + 1);
    rd = read(fdConf, readByte, oneByte);
    // a loop to read every row of conf file to a different buffer.
    while (rowFlag != 0) {
        if (rd == -1)
            rowFlag = -1;
        //switch the flag to know which buffer to fill or if an error has occurred.
        switch (rowFlag) {
            case 1:
                if (readByte[0] == '\n') {
                    rowFlag = 2;
                    rd = read(fdConf, readByte, oneByte);
                    continue;
                }
                //if(readByte[0] != '\"'){
                confRow1[i1] = readByte[0];
                i1++;

                //}
                break;
            case 2:
                if (readByte[0] == '\n') {
                    rowFlag = 3;
                    rd = read(fdConf, readByte, oneByte);
                    continue;
                }
                //if(readByte[0] != '\"'){
                confRow2[i2] = readByte[0];
                i2++;

                //}
                break;
            case 3:
                //if its end of line or end of file break the loop.
                if (readByte[0] == '\n' || rd == 0) {
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
                write(2, ERR_MSG, strlen(ERR_MSG));
                close(fdConf);
                return -1;
        }
        rd = read(fdConf, readByte, oneByte);
    } // end of while loop
    DIR *pDir;
    struct dirent *pDirent;
    pDir = opendir(confRow1);
    if (pDir == NULL) {
        write(2, ERR_MSG, strlen(ERR_MSG));
        close(fdConf);
        return -1;
    }
    pDirent = readdir(pDir);
    //open result file in the current directory.
    char cwd[MAX_ROW_LEN + 1], myOtpt[MAX_ROW_LEN + 1],
            result[MAX_ROW_LEN + 1], user[MAX_ROW_LEN + 1];
    clearBuffWithSize(cwd, MAX_ROW_LEN + 1);
    clearBuffWithSize(result, MAX_ROW_LEN + 1);
    clearBuffWithSize(myOtpt, MAX_ROW_LEN + 1);
    clearBuffWithSize(user, MAX_ROW_LEN + 1);
    getcwd(cwd, sizeof(cwd));
    strcpy(result, cwd);
    strcpy(myOtpt, cwd);
    strcat(cwd, "/results.csv");
    int resFd = open(cwd, O_CREAT | O_RDWR | O_APPEND,
                     S_IRWXG | S_IRWXO | S_IRWXU);
    strcat(myOtpt, "/myOtpt.txt");
    //run for every user in directory
    while (pDirent != NULL) {
        strcpy(user, pDirent->d_name);
        //continue if the name is . or  ..
        if (strcmp(user, ".") == 0 || strcmp(user, "..") == 0) {
            pDirent = readdir(pDir);
            continue;
        }
        //call dirActions to find c files.
        int res = dirActions(confRow1, confRow2,
                             confRow3, pDir, pDirent,
                             user, resFd, myOtpt,confRow1);
        //if the result is 0, no c file was found.
        if (res == 0) {
            strcat(user, NO_C);
            write(resFd, user, strlen(user));
            //there was an error, write error msg.
        } else if (res == -1) {
            write(2, ERR_MSG, strlen(ERR_MSG));
            closedir(pDir);
            close(resFd);
            close(fdConf);
            return -1;
        }
        pDirent = readdir(pDir);
    }
    //close directories and files.
    closedir(pDir);
    close(resFd);
    close(fdConf);
    return 0;
}