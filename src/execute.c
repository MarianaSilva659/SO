#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "include/util.h"

int execute_U(char *argv){
    struct String *message;

    int buffer_size = 100;
    char **buffer = Tracer_parser(argv, &buffer_size);

    int fd;
    pid_t a;
    double time_execute;
    int pipe_time[2];
    struct timeval current_time_filho;
    struct timeval current_time_pai;

    if(pipe(pipe_time) == -1){
        perror("erroPipe");
        exit(EXIT_FAILURE);
    }
    
    if((a = fork()) == 0){
        struct Info inicial;
        inicial.pedido = 'e';
        inicial.pid = getpid();
        inicial.programName = strdup(argv);

        char *pid_out = malloc(sizeof(char) * 100);
        sprintf(pid_out, "Running PID %d\n", getpid());
        write(1, pid_out, strlen(pid_out));
        free(pid_out);

    //notificar servidor
        if((fd = open("fifo",O_WRONLY)) == -1)
            return 2;

        char *program_name = malloc(sizeof(char) * 100);
        sprintf(program_name, "Program name: %s\n", inicial.programName);
        write(1, program_name, strlen(program_name));
        free(program_name);

        gettimeofday(&current_time_filho, NULL);

        close(pipe_time[0]);
        write(pipe_time[1], &current_time_filho, sizeof(current_time_filho));
        close(pipe_time[1]);

        message = to_String(inicial, current_time_filho, "");
        write (fd, message->content,sizeof(char) * message->lenght);
        close (fd);
    //notificar servidor
    
        execvp(argv, buffer);
        exit(0);
    }
    wait(0);

    gettimeofday(&current_time_pai, NULL);
    close(pipe_time[1]);
    read(pipe_time[0], &current_time_filho, sizeof(current_time_filho));
    close(pipe_time[0]);

    time_execute = (((current_time_pai.tv_sec - current_time_filho.tv_sec)*1000) + (double)(current_time_pai.tv_usec - current_time_filho.tv_usec)/1000);
        
    char *time = malloc(sizeof(char) * 100);
    sprintf(time, "Ended in %.3fms\n", time_execute);
    write(1, time, strlen(time));
    free(time);

    //notificar servidor
    if((fd = open("fifo",O_WRONLY)) == -1)
        return 3;

    struct Info final;
    final.pid = a;
    final.pedido = 'e';

    final.programName = strdup("Ended");
    message = to_String(final, current_time_pai, "");

    write (fd, message->content,sizeof(char) * message->lenght);
    close (fd);
    exit(0);
    //notificar servidor
    
    for(int k = 0; k <= buffer_size; k++) free(buffer[k]);
    free(buffer);

    return 0;
}


int execute_P(char *argv3){
    struct timeval start_time, end_time;
    double time_execute;
    int arg;
    int *tollerance = malloc(1000 * sizeof(int));
    for(int i = 0; i < 1000; i++) tollerance[i] = 1000;
    
    char *line = strdup(argv3);
    char ***comandos = Pipeline_Parser(line, tollerance, &arg);
    free(line);
    pid_t pid;
    int pipe_argumentos[arg - 1][2];
    int pid_primeiro_processo;

    for (int h = 0; h < arg-1; h++){
        if(pipe(pipe_argumentos [h]) == -1) {
            perror("erroPipe");
            exit(EXIT_FAILURE);
        }
    }
    gettimeofday(&start_time, NULL);

    //notificar servidor
    int fd;
    if((fd = open("fifo",O_WRONLY)) == -1)
        return 2;
    struct String *message;
    struct Info info;
    info.pedido = 'e';
    info.pid = getpid();
    char *argv = (char *) malloc(sizeof(char)*255);
 
    sprintf(argv,"%s",comandos[0][0]); 

    for(int iterator = 1; iterator < arg; iterator++){

        argv = strcat(argv,"|");
        argv = strcat(argv,comandos[iterator][0]);
    }
    info.programName = strdup(argv);
    message = to_String(info, start_time,"");
    write (fd, message->content,sizeof(char) * message->lenght);
    close (fd);
    //notificar servidor
    
    for (int i = 0; i < arg; i++){
        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit (EXIT_FAILURE);
        }
        if (pid == 0) {
            if (i != (arg - 1)) {
                dup2(pipe_argumentos[i][1], STDOUT_FILENO);
            }
            if (i > 0) {
                dup2(pipe_argumentos[i - 1][0], STDIN_FILENO);
            }
            execvp(comandos[i][0], comandos[i]);
            exit(EXIT_FAILURE);
        }
        //processo pai
        else {
            if (i == 0) {
                char *str = malloc(sizeof(char) * 100);
                sprintf(str, "Running PID %d\n", getpid());
                write(1, str, strlen(str));
                free(str);
            }
            wait(NULL);
            if (i > 0) {
                close(pipe_argumentos[i - 1][0]);
            }
            if (i != (arg - 1)) {
                close(pipe_argumentos[i][1]);
            }
        }
    }
    gettimeofday(&end_time, NULL);
    
    //notificar servidor
    fd = -1;
    if((fd = open("fifo",O_WRONLY)) == -1)
        return 2;
    info.pedido = 'e';
    info.programName = strdup("Ended");
    message = to_String(info, end_time,"");
    write (fd, message->content,sizeof(char) * message->lenght);
    close (fd); 
    //notificar servidor
    
    time_execute = (end_time.tv_sec - start_time.tv_sec) * 1000 + (double)(end_time.tv_usec - start_time.tv_usec) / 1000;

    char *time = malloc(sizeof(char) * 100);
    sprintf(time, "Ended in %.3fms\n", time_execute);
    write(1, time, strlen(time));
    free(time);
    return 0;
}