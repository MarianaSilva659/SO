#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

struct Info { //se o programName for NULL então é final
    int pid;
    char* programName;//buffer[0]
    //timestamp;    
};

struct String {
    int lenght;
    char *content;
};

struct String *to_String(struct Info info)
{
   struct String *result = malloc(sizeof(struct String));
    result->lenght = snprintf(NULL, 0, "PID: %d NAME: %s \n", info.pid, info.programName) + 1;
   result->content = malloc(result->lenght);
   if(result->content == NULL) return NULL;
   snprintf(result->content, result->lenght, "PID: %d NAME: %s \n", info.pid, info.programName);
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
            double time_execute;
            clock_t start_time_filho, start_time_pai, end_time;
            int pipe_time[2];

            if(pipe(pipe_time) == -1){
                perror("erroPipe");
                exit(EXIT_FAILURE);
            }

            if((a = fork()) == 0){
                close(pipe_time[0]);
                struct Info inicial;
                inicial.pid = getpid();
                inicial.programName = strdup(argv[3]);

                printf("Running PID %d\n", inicial.pid);

//-----------------------------------------------------------------
//--------------------Notificar servidor---------------------------

                if((fd = open("fifo",O_WRONLY)) == -1)
                    return 2;
                // mandar a informação para o server
                printf("\nprogram name: %s|\n",inicial.programName);
                
                start_time_filho = clock();
                write(pipe_time[1], &start_time_filho, sizeof(double));
                close(pipe_time[1]);

                message = to_String(inicial);
                write (fd, message->content,sizeof(char) * message->lenght);
                close (fd);

//----------------------------------------------------------------
//--------------------fazer execute -u----------------------------
                execvp(argv[3],buffer);
            }
            wait(0);
            end_time = clock();

            close(pipe_time[1]);
            read(pipe_time[0], &start_time_pai, sizeof(double));
            close(pipe_time[0]);

            time_execute = ((double)(end_time-start_time_pai) * 1000) / CLOCKS_PER_SEC;
            printf("Enden in %fms\n", time_execute);
//-----------------------------------------------------------------
//--------------------Notificar servidor---------------------------
            
            if((fd = open("fifo",O_WRONLY)) == -1)
                return 3;
            
            // mandar a informação para o server
            struct Info final;
            final.pid = a;
            
            final.programName = strdup("Ended");
            message = to_String(final);

            write (fd, message->content,sizeof(char) * message->lenght);
            close (fd);

//preencher a estrutura EINFO e mandar a informação para o server
//-----------------------------------------------------------------
//--------------------Notificar cliente----------------------------
                exit(0);
//-----------------------------------------------------------------

        }
        printf("Usage->outras opçoes do execute:not done yet\n");
        return 0;
    }
    else{
        printf("Usage:not done yet\n");
        return 0;
    }
}

