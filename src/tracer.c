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

struct Info { //se o programName for NULL então é final
    char pedido;
    int pid;
    char* programName;//buffer[0]
    //timestamp;    
};

struct String {
    int lenght;
    char *content;
};

char** Tracer_parser (char *message, int *tollerance){
    char **arguments = (char **) malloc(sizeof(char *) * tollerance[0]);
    char *token;
    token = strtok(message, " ");
    int i = 0;
    for (;(token != NULL); i++) {
        if(i >= tollerance[0] - 10){
            char **aux = realloc(arguments, sizeof(arguments) * tollerance[0] << 1);
            if(aux != NULL){
               tollerance[0] = tollerance[0] << 1;
               arguments = aux;
            }
        }
        arguments[i] = strdup(token);
        token = strtok(NULL, " ");
    }
    arguments[i] = NULL;
    return arguments;
}


char***Pipeline_Parser(char *message, int *tollerance){
    char ***arguments = (char ***) malloc(sizeof(char **) * tollerance[0]);
    char *token;
    int k=0;
    int current_word = k;
    for(;message[k] != '|' && (message[k] != '\0');k++); message[k] = '\0';
    k++;
    if(message[current_word] != '\0')
    token = strdup(&message[current_word]);
    else token = NULL;
    int i = 0;
    for (;(token != NULL); i++) {
        if(i >= tollerance[0] - 10){
            char ***aux = realloc(arguments, sizeof(arguments) * tollerance[0] << 1);
            if(aux != NULL){
               tollerance[0] = tollerance[0] << 1;
               arguments = aux;
            }
        }
        arguments[i] = Tracer_parser(token, &tollerance[i+1]);
        current_word = k;
        for(;(message[k] != '|') && (message[k] != '\0');k++); message[k] = '\0';
        k++;
        if(message[current_word] != '\0')
        token = strdup(&message[current_word]);
        else token = NULL;
    }
    arguments[i] = NULL;
    return arguments;
}

struct String *to_String(struct Info info, char *time_string)
{
   struct String *result = malloc(sizeof(struct String));
    result->lenght = snprintf(NULL, 0, "%c PID: %d NAME: %s TIME: %s \n", info.pedido ,info.pid, info.programName, time_string) + 1;
   result->content = malloc(result->lenght);
   if(result->content == NULL) return NULL;
   snprintf(result->content, result->lenght, "%c PID: %d NAME: %s TIME: %s \n", info.pedido ,info.pid, info.programName, time_string);
   return result;
}

int execute_U(char *argv){
    struct String *message;

        int buffer_size = 100;
        char **buffer = Tracer_parser(argv, &buffer_size);

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
            inicial.pedido = 'e';
            inicial.pid = getpid();
            inicial.programName = strdup(argv);

            printf("Running PID %d\n", inicial.pid);

            if((fd = open("fifo",O_WRONLY)) == -1)
                return 2;

            printf("\nProgram name: %s|\n",inicial.programName);

        gettimeofday(&current_time_filho, NULL);
        time_t nowtime = current_time_filho.tv_sec;
        nowtm = localtime(&nowtime);

        strftime(time_string, 32, "%Y-%m-%d %H:%M:%S", nowtm); 
        sprintf(time_ms, ".%ld", (current_time_filho.tv_usec/1000));
        strcat(time_string, time_ms);

        close(pipe_time[0]);
        write(pipe_time[1], &current_time_filho, sizeof(current_time_filho));
        close(pipe_time[1]);

            message = to_String(inicial, time_string);
            write (fd, message->content,sizeof(char) * message->lenght);
            close (fd);

            execvp(argv, buffer);
            exit(0);
        }
        wait(0);

        gettimeofday(&current_time_pai, NULL);
        time_t nowtime = current_time_pai.tv_sec;
        nowtm = localtime(&nowtime);

        strftime(time_string, 32,"%Y-%m-%d %H:%M:%S", nowtm);
        sprintf(time_ms, ".%ld", (current_time_pai.tv_usec/1000));
        strcat(time_string, time_ms);


        close(pipe_time[1]);
        read(pipe_time[0], &current_time_filho, sizeof(current_time_filho));
        close(pipe_time[0]);

        time_execute = (((current_time_pai.tv_sec - current_time_filho.tv_sec)*1000) + (current_time_pai.tv_usec - current_time_filho.tv_usec)/1000);
        printf("Ended in %dms\n", (time_execute ));

        if((fd = open("fifo",O_WRONLY)) == -1)
            return 3;

        struct Info final;
        final.pid = a;
        final.pedido = 'e';

        final.programName = strdup("Ended");
        message = to_String(final, time_string);

        write (fd, message->content,sizeof(char) * message->lenght);
        close (fd);
        exit(0);

        free(time_string);
        free(time_ms);
        for(int k = 0; k <= buffer_size; k++) free(buffer[k]);
        free(buffer);

        return 0;
}


int main(int argc, char **argv){
   struct String *message;
    if (argc < 2){
        return 1;
    }
    /*Exemplo de como usar o novo parser
    int tollerance[5] = {100, 100, 100, 100, 100};
    char *arroz = strdup("cat log.txt | greep \"NAME\" | wc -l");
    char  ***justforfun = Pipeline_Parser(arroz, tollerance);
    for(int j = 0; j < 3; j++){
        printf("HELLO\n");
    for(int i = 0; i < 3; i++){
        printf("%s\n\n", justforfun[j][i]);
        if(justforfun[j][i] != NULL){
        free(justforfun[j][i]);
        }
    }
    free(justforfun[j]);
    }
    free(justforfun);
    */

    if (strcmp(argv[1], "execute") == 0){
        if (strcmp(argv[2], "-u") == 0){
                execute_U(argv[3]);
        }
        
        printf("Usage->outras opçoes do execute:not done yet\n");
        return 0;
    }else 
//-----------------------------------------------------------------
        if(strcmp(argv[1], "status") == 0){
            //criar o fifo
            struct Info info;
            info.pid = getpid();
            int tamanho = snprintf(NULL, 0, "fifo_%d", info.pid) + 1;
            info.programName = malloc(tamanho);
            if(info.programName == NULL) return -1;
            snprintf(info.programName, tamanho, "fifo_%d", info.pid);
            if (mkfifo(info.programName,0666)==0)
                perror("mkfifo"); 
            info.pedido = 's';

            //mandar para o servidor o fifo e o tempo a que foi pedido
            struct timeval current_time;
            int fd;
            int time_execute;
            struct tm *nowtm;
            char *time_string = (char*)malloc(sizeof(char)* 32);
            char *time_ms = (char*)malloc(sizeof(char) * 8);
            
            if((fd = open("fifo",O_WRONLY)) == -1)
                    return 2;
                // mandar a informação para o server
                
            gettimeofday(&current_time, NULL);
            time_t nowtime = current_time.tv_sec;
            nowtm = localtime(&nowtime);

            strftime(time_string, 32, "%Y-%m-%d %H:%M:%S", nowtm); 
            sprintf(time_ms, ".%ld", (current_time.tv_usec/1000));
            strcat(time_string, time_ms);

            struct String *message;
            message = to_String(info, time_string);
            write (fd, message->content,sizeof(char) * message->lenght);
            close (fd);
            //esperar pelo servidor e fazer print de tudo o que receber do fifo no strout

            free(time_string);
            free(time_ms);
            unlink(info.programName);
            //no servidor
            //percorrer os logs e procurar aqueles que até ao momento pedido, não tinham acabado e fazer print deles no fifo recebido
    }else{
        printf("Usage:not done yet\n");
        return 0;
    }
}