/**
 *  Written By: Michael Hegglin
 *
 *  Description: 
 *
 **/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

void output_reversed_ls(int argc, char ** argv);

int main(int argc, char ** argv)
{
    output_reversed_ls(argc, argv);
    return 0;
}

/**
 *  Description:
 *    Creates a pipe, and forks two subprocesses.  Within
 *    each subprocess, cmds will be executed.  The first cmd will 
 *    be an "ls", which is then piped to a reversed sort "sort -r". Finally
 *    the output of this is written to a file "> outfile".
 **/
void output_reversed_ls(int argc, char ** argv)
{
    pid_t pid;
    int fd[2], out_fd;
    char * first_cmd[2];
    char * second_cmd[3];

    first_cmd[0] = "ls";
    first_cmd[1] = NULL;

    second_cmd[0] = "sort";
    second_cmd[1] = "-r";
    second_cmd[2] = NULL;

    /*  Opens File For Writing  */
    out_fd = open("outfile", O_WRONLY | O_CREAT | O_TRUNC, 0666);

    if (pipe(fd) == -1) /*  Creates A pipe          */
    {
        perror("Failed To Create Pipe ");
        exit(1);
    }

    if ((pid = fork()) == 0)            /* Forks A Child Process   */
    {
        dup2(fd[1], 1);                 /*  dups stdout to pipe     */
        close(fd[0]);
        execvp(first_cmd[0], first_cmd); /* executes the ls cmd     */
    }
    else if (pid < 0)
    {
        perror("ls cmd failed at fork ");
    }

    if ((pid = fork()) == 0)
    {
        dup2(fd[0], 0);     /*  dups stdin to Pipe      */
        dup2(out_fd, 1);    /* dups stdout to fd       */
        close(fd[1]);
        execvp(second_cmd[0], second_cmd); /*   executes the ls cmd     */
    }
    else if (pid < 0)
    {
        perror("sort cmd failed at fork ");
    }
    
    close(out_fd);
    close(fd[0]);
    close(fd[1]);
    wait(NULL);
    wait(NULL);
}


