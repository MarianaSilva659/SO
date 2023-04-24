#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

struct Info { //se o programName for NULL então é final
    int pid;
    char* programName;//buffer[0]
    //timestamp;    
};

int indexLastChar (char *message, struct Info *store){
    char *message_copy = strdup(message);
    char **palavras = (char **) malloc(sizeof(char *) * 100);
    char *token;
    int index = 0;
    int size = 0;
    token = strtok(message_copy, " ");
    int i = 0;
    
    for (;(token != NULL) && (message_copy[index] != '\n') && (message_copy[index] != '\0') && (i < 100); i++) {
        size = strlen(token);
        printf("%s \n", token);
          if(i == 1){
             store->pid = atoi(token);
          }
          if(i == 3) {
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

        while ((bytes_read = read (fd_read, buffer, 512)) > 0){
            printf("%s \n", buffer);
            lseek(fd_read, -(bytes_read - indexLastChar(buffer, &info)), SEEK_CUR);
            write (log, buffer,strlen(buffer)+1);
            if(strcmp(info.programName, "close_monitor") == 0) break;
            // write (log, &bytes_read,sizeof(int));
            //write (log, buf.programName,sizeof(char)*255);
        }
        close (fd_read);
        close(fd_write);
        unlink("fifo");
}
//se quiserem responder ao cliente cada cliente precisa tambem de um fifo e 
//mandar a informação necessaria para o servidor aceder a esse fifo.