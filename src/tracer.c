#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>

struct Info { //se o programName for NULL então é final
    int pid;
    char* programName;//buffer[0]
    //timestamp;    
};

struct String {
    int lenght;
    char *content;
};

struct String *to_String(struct Info info, char *time_string)
{
   struct String *result = malloc(sizeof(struct String));
    result->lenght = snprintf(NULL, 0, "PID: %d NAME: %s TIME: %s \n", info.pid, info.programName, time_string) + 1;
   result->content = malloc(result->lenght);
   if(result->content == NULL) return NULL;
   snprintf(result->content, result->lenght, "PID: %d NAME: %s TIME: %s \n", info.pid, info.programName, time_string);
   return result;
}



int main(int argc, char **argv){
   struct String *message;
    if (argc < 2){
        return 1;
    }

    if (strcmp(argv[1], "execute") == 0){
        if (strcmp(argv[2], "-u") == 0){

            //parce do arg[3] ignora espaços
            int n = 10; //max arguments
            char **buffer = (char**)calloc(n, sizeof(char*));

            for(int i = 0,j = 0; argv[3][j] != '\0' && i < n; j++){
                if(argv[3][j] == ' '){
                    argv[3][j] = '\0';
                    if(argv[3][j+1] && argv[3][j+1] != ' ')
                        buffer[i++] = &argv[3][j+1];
                }
            }

//--------------------Notificar cliente----------------------------
            int fd;
            pid_t a;
            int time_execute;
            int pipe_time[2];
            struct timeval current_time_filho;
            struct timeval current_time_pai;
            struct tm *nowtm;
            char *time_string = (char*)malloc(sizeof(char)* 32);
            char *time_ms = (char*)malloc(sizeof(char) * 8);

            if(pipe(pipe_time) == -1){
                perror("erroPipe");
                exit(EXIT_FAILURE);
            }

            if((a = fork()) == 0){
 
                struct Info inicial;
                inicial.pid = getpid();
                inicial.programName = strdup(argv[3]);

                printf("Running PID %d\n", inicial.pid);

//-----------------------------------------------------------------
//--------------------Notificar servidor---------------------------

                if((fd = open("fifo",O_WRONLY)) == -1)
                    return 2;
                // mandar a informação para o server
                printf("\nProgram name: %s|\n",inicial.programName);
                
                        gettimeofday(&current_time_filho, NULL);
                        time_t nowtime = current_time_filho.tv_sec;
                        nowtm = localtime(&nowtime);

                        strftime(time_string, 32, "%Y-%m-%d %H:%M:%S", nowtm); 
                        sprintf(time_ms, ".%ld", current_time_filho.tv_usec);
                        strcat(time_string, time_ms);

                        close(pipe_time[0]);
                        write(pipe_time[1], &current_time_filho.tv_usec, sizeof(suseconds_t));
                        close(pipe_time[1]);

                message = to_String(inicial, time_string);
                write (fd, message->content,sizeof(char) * message->lenght);
                close (fd);

//----------------------------------------------------------------
//--------------------fazer execute -u----------------------------
                execvp(argv[3],buffer);
                exit(0);
            }
            wait(0);

                        gettimeofday(&current_time_pai, NULL);
                        time_t nowtime = current_time_pai.tv_sec;
                        nowtm = localtime(&nowtime);

                        strftime(time_string, 32,"%Y-%m-%d %H:%M:%S", nowtm);
                        sprintf(time_ms, ".%ld", current_time_pai.tv_usec);
                        strcat(time_string, time_ms);


                        close(pipe_time[1]);
                        read(pipe_time[0], &current_time_filho.tv_usec, sizeof(suseconds_t));
                        close(pipe_time[0]);

                        time_execute = current_time_pai.tv_usec - current_time_filho.tv_usec;
            printf("Ended in %dms\n", time_execute);
//-----------------------------------------------------------------
//--------------------Notificar servidor---------------------------
            
            if((fd = open("fifo",O_WRONLY)) == -1)
                return 3;
            
            // mandar a informação para o server
            struct Info final;
            final.pid = a;
            
            final.programName = strdup("Ended");
            message = to_String(final, time_string);

            write (fd, message->content,sizeof(char) * message->lenght);
            close (fd);

//preencher a estrutura EINFO e mandar a informação para o server
//-----------------------------------------------------------------
//--------------------Notificar cliente----------------------------
                exit(0);
//-----------------------------------------------------------------
            free(time_string);
            free(time_ms);
        }
        printf("Usage->outras opçoes do execute:not done yet\n");
        return 0;
    }
    else{
        printf("Usage:not done yet\n");
        return 0;
    }
}
