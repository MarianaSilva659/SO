#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "server_status.h"
#include "ht.h"

int Server_parser (char *message, struct MSG *store){
    char *message_copy = strdup(message);
    char *token;
    int index = 0;
    int size = 0;
    token = strtok(message_copy, " ");
    int i = 0;
    
    for (;(token != NULL); i++) {
        size = strlen(token);
          if(i == 0){
            store->pedido = token[0];
          }
          else if(i == 2){
             store->pid = atoi(token);
          }
         else if(i == 4) {
               free(store->programName);
            store->programName = strdup(token);
          }else if(i == 6){
            sscanf(token, "%ld.%06ld", &store->time.tv_sec, &store->time.tv_usec);
          }else if(i == 7){
            store->max_size=100;
            store->arguments= malloc(100 * sizeof(int));
            store->current_index=0;
            store->arguments[store->current_index] = atoi(&token[4]);
            store->current_index++;
          }else if(i > 7){
            char loop_count = 0;
            while(store->current_index > store->max_size){
                if(loop_count > 10) exit(10);
                  int *aux = (int *)realloc(store->arguments, store->max_size * 2 * sizeof(int));
               if(aux != NULL){
            store->max_size == store->max_size << 1;
            store->arguments = aux;
        } else loop_count++;
            }
            store->arguments[store->current_index] = atoi(&token[4]);
            store->current_index++;
          }

        token = strtok(NULL, " ");
        index += size + 1;
    }
    free(message_copy);
    return index - 2;
}

int main(int argc, char **argv){
    Log *log = newLog();
     int tamanho;
    char bool = 1;
    if (mkfifo("fifo",0666)==0)
        perror("mkfifo"); 
    
    int arquive;
    int fd_read, fd_write,bytes_read;
    struct MSG info;
    info.arguments = malloc(100 * sizeof(char));
    info.programName = malloc(sizeof(char));
    char *buffer = calloc(512, sizeof(char));
    int pid;
        if((fd_read = open("fifo",O_RDONLY)) == -1){
            perror("open");
            return 1;
        }
        if((fd_write = open("fifo",O_WRONLY)) == -1){
            perror("open");
            return 2;
        }
        lseek(arquive,0,SEEK_END);
        int i;
        while (bool && (bytes_read = read (fd_read, buffer, 512)) > 0){
            for(i=0;(buffer[i] != '\n') && (buffer[i] != '\0'); i++);
            lseek(fd_read, -(bytes_read - i +2), SEEK_CUR);
            switch (buffer[0])
            {
            case 's':
               if((pid = fork()) == 0){
                Server_parser(buffer, &info);
                status(log, info);
                exit(1);
               }
                break;
            case 'c':
                bool = 0;
                break;
            case 'e':
            Server_parser(buffer, &info);
                upDateTable(log, info, argv[1]);
                //escrever no ficheiro correspondente ao pid (pode ser feito num fork para melhor concorrêmcia)
                break;
            case 't':
             if((pid = fork()) == 0){
                Server_parser(buffer, &info);
                stats_time(log, info);
                exit(1);
               }
                break;
            case 'u':
             if((pid = fork()) == 0){
                Server_parser(buffer, &info);
                stats_uniq(log, info);
                exit(1);
               }
                break;
            case 'n':
            if((pid = fork()) == 0){
                Server_parser(buffer, &info);
                stats_command(log, info);
               exit(1);
             }
                break;
            default:
            break;
            }
              printf("%s \n", buffer);
        }
        free(info.programName);
        free(info.arguments);
        free(buffer);
        close (fd_read);
        close(fd_write);
        unlink("fifo");
}
//se quiserem responder ao cliente cada cliente precisa tambem de um fifo e 
//mandar a informação necessaria para o servidor aceder a esse fifo.