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

struct Info {
    char pedido;
    int pid;
    char* programName;
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

char***Pipeline_Parser(char *message, int *tollerance, int *arg){
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

    arguments[i] = calloc(1, sizeof(char));
    return arguments;
}

struct String *to_String(struct Info info, struct timeval time)
{
   struct String *result = malloc(sizeof(struct String));
    result->lenght = snprintf(NULL, 0, "%c PID: %d NAME: %s TIME: %ld.%06ld \n", info.pedido ,info.pid, info.programName, time.tv_sec, time.tv_usec) + 1;
   result->content = malloc(result->lenght);
   if(result->content == NULL) return NULL;
   snprintf(result->content, result->lenght, "%c PID: %d NAME: %s TIME: %ld.%06ld \n", info.pedido ,info.pid, info.programName, time.tv_sec, time.tv_usec);
   return result;
}

struct String *Status_to_String(struct Info info){ 
   struct String *result = malloc(sizeof(struct String));
   result->lenght = snprintf(NULL, 0, "%c PID: %d \n", info.pedido ,info.pid) + 1;
   result->content = malloc(result->lenght);
   if(result->content == NULL) return NULL;
   snprintf(result->content, result->lenght, "%c PID: %d \n", info.pedido ,info.pid);
   return result;
}

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

            printf("Running PID %d\n", inicial.pid);

            if((fd = open("fifo",O_WRONLY)) == -1)
                return 2;

            printf("\nProgram name: %s|\n",inicial.programName);

        gettimeofday(&current_time_filho, NULL);

        close(pipe_time[0]);
        write(pipe_time[1], &current_time_filho, sizeof(current_time_filho));
        close(pipe_time[1]);

            message = to_String(inicial, current_time_filho);
            write (fd, message->content,sizeof(char) * message->lenght);
            close (fd);

            execvp(argv, buffer);
            exit(0);
        }
        wait(0);

        gettimeofday(&current_time_pai, NULL);
        close(pipe_time[1]);
        read(pipe_time[0], &current_time_filho, sizeof(current_time_filho));
        close(pipe_time[0]);

        time_execute = (((current_time_pai.tv_sec - current_time_filho.tv_sec)*1000) + (double)(current_time_pai.tv_usec - current_time_filho.tv_usec)/1000);
        printf("Ended in %.3fms\n", time_execute);

        if((fd = open("fifo",O_WRONLY)) == -1)
            return 3;

        struct Info final;
        final.pid = a;
        final.pedido = 'e';

        final.programName = strdup("Ended");
        message = to_String(final, current_time_pai);

        write (fd, message->content,sizeof(char) * message->lenght);
        close (fd);
        exit(0);

        for(int k = 0; k <= buffer_size; k++) free(buffer[k]);
        free(buffer);

        return 0;
}


int funcionalidades_avancadas(char *argv3){
    struct timeval start_time, end_time;
    double time_execute;
    int arg = 3;
     /*int tollerance[5] = {100, 100, 100, 100, 100};
    char *arroz = strdup("cat fich1 | cat fich1 | wc -l");
    char  ***comandos = Pipeline_Parser(arroz, tollerance, &arg);
printf("arg %d\n", arg);
for(int i = 0; i < 4; i++){
    for(int j = 0; j < 3;j++){
        printf("comandos %s\n", comandos[i][j]);
    }
}*/
   char* comandos [][3] = {{"cat", "fich1", NULL}, {"grep", "palavra", NULL}, {"wc", "-l", NULL}};
   /* for(int i = 0; i < 3;i++){
        for(int j =0 ; j < 3; j++){
    printf("comandooo %s\n", comandos[i][j]);
    }
    }*/
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
                    printf("Running PID %d\n", getpid());
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
    time_execute = (end_time.tv_sec - start_time.tv_sec) * 1000 + (double)(end_time.tv_usec - start_time.tv_usec) / 1000;
    printf("Ended in %.3fms\n", time_execute);
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
        else if(strcmp(argv[2], "-p") == 0){
            funcionalidades_avancadas(argv[3]);
            return 0;
        }
        printf("Usage->outras opçoes do execute:not done yet\n");
        return 0;
    }else {
//-----------------------------------------------------------------
printf("oi\n");
            //criar o fifo
                  int fd_write, self_read, self_write;
            struct Info info;
            info.pid = getpid();
            int tamanho = snprintf(NULL, 0, "fifo_%d", info.pid) + 1;
            info.programName = malloc(tamanho);
            if(info.programName == NULL) return -1;
            snprintf(info.programName, tamanho, "fifo_%d", info.pid);
            if(strcmp(argv[1], "status") == 0)
            info.pedido = 's';
            else if(strcmp(argv[1], "close_monitor") == 0)
            info.pedido = 'c';
            if(info.pedido != 'c')
            if (mkfifo(info.programName,0666)==0)
                perror("mkfifo");
            printf("%s\n", info.programName);

            //mandar para o servidor o fifo e o tempo a que foi pedido
            struct timeval current_time;
            

            if((fd_write = open("fifo",O_WRONLY)) == -1){
             perror("open");
                    return 2;
            }
            // mandar a informação para o server
   

            struct String *message; 
            gettimeofday(&current_time, NULL);
            message = to_String(info, current_time);
            write (fd_write, message->content,sizeof(char) * message->lenght);
            close (fd_write);
            if(info.pedido != 'c'){

            if((self_read = open(info.programName,O_RDONLY)) == -1){
                perror("open");
                    return 2;
            }
                printf("Entrou\n");
                char *buffer = malloc(512*sizeof(char));
                int bytes_read;
                while((bytes_read = read(self_read, buffer, 512)) > 0){
                    printf("%s \n", buffer);
                }
                free(buffer);
            close (self_read);
            unlink(info.programName);
            }
            //esperar pelo servidor e fazer print de tudo o que receber do fifo no strout
            //no servidor
            //percorrer os logs e procurar aqueles que até ao momento pedido, não tinham acabado e fazer print deles no fifo recebido
    } 
}

