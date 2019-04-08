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

void clearBuffWithSize(char buff[], int size) {
    int i;
    for (i = 0; i < size; i++) {
        buff[i] = '\0';
    }
}

int executeFile(char input[], char output[],
                char user[], int resFd, char pathName[], char myOtpt[]) {
    int status;
    char *args1[] = {"gcc", pathName, NULL};
    char *args2[] = {"./a.out", NULL};
    char *args3[] = {"./comp.out", output, myOtpt, NULL};
    int iptFd = open(input, O_RDONLY);
    int myOtptFd = open(myOtpt, O_CREAT | O_WRONLY | O_RDONLY,
                        S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH | S_IWOTH | S_IWGRP | S_IXGRP | S_IXOTH);
    time_t timeBfr, timeAftr;
    struct tm *tmInfBfr, *tmInfAftr;
    time(&timeBfr);
    tmInfBfr = localtime(&timeBfr);
    int bfr = tmInfBfr->tm_sec;
    int pid = fork();
    if (pid == -1) {
        unlink(myOtpt);
        close(iptFd);
        return -1;
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
        status = WEXITSTATUS(status);
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
                close(iptFd);
                while (waitpid(pid, &status, WNOHANG) != pid) {
                    time(&timeAftr);
                    tmInfAftr = localtime(&timeAftr);
                    if (abs(bfr - tmInfAftr->tm_sec) > 5) {
                        strcat(user, TIMEOUT);
                        write(resFd, user, strlen(user));
                        unlink(myOtpt);
                        return 1;
                    }
                }
                pid = fork();
                if (pid == -1) {
                    unlink(myOtpt);
                    return -1;
                } else if (pid > 0) {
                    waitpid(pid, &status, 0);
                    int retVal = WEXITSTATUS(status);
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

// TODO: something is going on in here, must be with compilation directories.
int dirActions(char dir[], char input[],
               char output[], DIR *pDir,
               struct dirent *pDirent, char user[], int resFd, char myOtpt[],char origDir[]) {
    int res = 0;
    char name[MAX_ROW_LEN + 1], pathName[MAX_ROW_LEN + 1];
    clearBuffWithSize(name, MAX_ROW_LEN + 1);
    clearBuffWithSize(pathName, MAX_ROW_LEN + 1);
    struct stat statbuf;
    DIR * newPdir;
    while (pDirent != NULL) {
        if (strcmp(pDirent->d_name, ".") == 0 || strcmp(pDirent->d_name, "..") == 0) {
            pDirent = readdir(pDir);
            continue;
        }
        strcpy(name, pDirent->d_name);
        strcpy(pathName, dir);
        strcat(pathName, "/");
        strcat(pathName, name);
        if (strlen(name) > 2) {
            if (name[strlen(name) - 1] == 'c' && name[strlen(name) - 2] == '.') {
                res = executeFile(input, output, user, resFd, pathName, myOtpt);
                return res;
            }
        }
        int st = stat(pathName, &statbuf);
        if (st < 0) {
            perror("stat()");
            printf("%s\n",pathName);
            return -1;
        }
        if (S_ISDIR(statbuf.st_mode)) {
            newPdir = opendir(pathName);
            if (pDir == NULL) {
                return -1;
            }
            pDirent = readdir(newPdir);
            res = dirActions(pathName, input, output, newPdir, pDirent, user, resFd, myOtpt,origDir);
            closedir(newPdir);
            if(dir == origDir){
                return res;
            }
        }
        if(res){
            return res;
        }
        pDirent = readdir(pDir);
    }
    return res;
}

int main(int argc, char *argv[]) {
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
    int resFd = open(cwd, O_CREAT | O_WRONLY | O_RDONLY | O_APPEND,
                     S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH | S_IWOTH | S_IWGRP | S_IXGRP | S_IXOTH);
    strcat(myOtpt, "/myOtpt.txt");
    //run for every user in directory
    while (pDirent != NULL) {
        if (strcmp(pDirent->d_name, ".") == 0 || strcmp(pDirent->d_name, "..") == 0) {
            pDirent = readdir(pDir);
            continue;
        }
        int res = dirActions(confRow1, confRow2,
                             confRow3, pDir, pDirent,
                             pDirent->d_name, resFd, myOtpt,confRow1);
        if (res == 0) {
            strcpy(user, pDirent->d_name);
            strcat(user, NO_C);
            write(resFd, user, strlen(user));
        } else if (res == -1) {
            write(2, ERR_MSG, strlen(ERR_MSG));
            closedir(pDir);
            close(resFd);
            close(fdConf);
            return -1;
        }
        pDirent = readdir(pDir);
    }
    closedir(pDir);
    close(resFd);
    close(fdConf);
    return 0;
}