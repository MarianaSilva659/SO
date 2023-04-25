#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

struct Info { //se o programName for NULL então é final
    char pedido;
    int pid;
    char* programName;//buffer[0]
    //timestamp;    
};

int Server_parser (char *message, struct Info *store){
    char *message_copy = strdup(message);
    char **palavras = (char **) malloc(sizeof(char *) * 100);
    char *token;
    int index = 0;
    int size = 0;
    token = strtok(message_copy, " ");
    int i = 0;
    
    for (;(token != NULL) && (message_copy[index] != '\n') && (message_copy[index] != '\0') && (i < 100); i++) {
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
          }

        token = strtok(NULL, " ");
        index += size + 1;
    }
    free(message_copy);
    return index - 2;
}

int main(int argc, char **argv){
    
    if (mkfifo("fifo",0666)==0)
        perror("mkfifo"); 

    int log;
    int fd_read, fd_write,bytes_read;
    struct Info info;
    char *buffer = calloc(512, sizeof(char));
    info.programName = malloc(sizeof(char));
        if((fd_read = open("fifo",O_RDONLY)) == -1){
            perror("open");
            return 1;
        }
        if((fd_write = open("fifo",O_WRONLY)) == -1){
            perror("open");
            return 2;
        }
        if((log = open("log.txt",O_CREAT|O_WRONLY,0640)) == -1){
            perror("open");
            return 3;
        }
        lseek(log,0,SEEK_END);
         write(log, "Start\n", 6);

        while ((bytes_read = read (fd_read, buffer, 512)) > 0){
            printf("%s \n", buffer);
            lseek(fd_read, -(bytes_read - Server_parser(buffer, &info)), SEEK_CUR);
                  if(strcmp(info.programName, "close_monitor") == 0){
                    write(log, "Close\n", 6);
                    break;
                  }
            if(info.pedido == 'e'){
                write (log, buffer,strlen(buffer));
            }
            else if(info.pedido == 's'){
                write (log, buffer,strlen(buffer));
            }
        }
        close (fd_read);
        close(fd_write);
        unlink("fifo");
}
//se quiserem responder ao cliente cada cliente precisa tambem de um fifo e 
//mandar a informação necessaria para o servidor aceder a esse fifo.