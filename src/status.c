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

int status(int argc, char **argv){

    int fd_write, self_read, self_write;
    struct Info info;
    char *argumments = calloc(1000, sizeof(char));
    int arg_size = 1000;
    int current_arg_size = 0;
    info.pid = getpid();

    if(argc >= 3){
        int temp;
        char loop_count = 0;
        for(int i = 2; i < argc; i++){
            temp = strlen(argv[i]);

            while(current_arg_size + temp >= arg_size){
                if(loop_count > 10) exit(9);
                char *aux = (char *)realloc(argumments, arg_size * 2 * sizeof(char));
                if(aux != NULL){
                    arg_size == arg_size << 1;
                    argumments = aux;
                    loop_count =0;
                }else loop_count++;
            }
            sprintf(argumments,"%s %s",argumments, argv[i]);
        }
    }

    int tamanho = snprintf(NULL, 0, "fifo_%d", info.pid) + 1;
    info.programName = malloc(tamanho);
    if(info.programName == NULL) return -1;
    snprintf(info.programName, tamanho, "fifo_%d", info.pid);
    if(strcmp(argv[1], "status") == 0)
        info.pedido = 's';
    else if(strcmp(argv[1], "close_monitor") == 0)
        info.pedido = 'c';
    else if(strcmp(argv[1], "stats-time") == 0)
        info.pedido = 't';
    else if(strcmp(argv[1], "stats-command") == 0)
        info.pedido = 'n';
    else if(strcmp(argv[1], "stats-uniq") == 0)
        info.pedido = 'u';
    else{
        write(1,"Invalid_Command\n",17*sizeof(char));
        return 100;
    }
    int count = 1;
    if(info.pedido != 'n'){
        int tamanho = snprintf(NULL, 0, "fifo_%d", info.pid) + 1;
        info.programName = malloc(tamanho);
        if(info.programName == NULL) return -1;
        snprintf(info.programName, tamanho, "fifo_%d", info.pid);
    }
    if((info.pedido != 'c'))
        if (mkfifo(info.programName,0666)==0)
            perror("mkfifo");
        
    /*mandar para o servidor o fifo e o tempo a que foi pedido*/
    struct timeval current_time;
    
    if((fd_write = open("fifo",O_WRONLY)) == -1){
     perror("open");
            return 2;
    }
    struct String *message; 
    gettimeofday(&current_time, NULL);

    if(info.pedido != 'n')
        message = to_String(info, current_time, &argumments[1]);
    else{             
        for(; argumments[count] != ' '; count++);

        argumments[count] = '\0';
        char *temp = info.programName;
        info.programName = strdup(&argumments[1]);
        message= to_String(info, current_time, &argumments[count+1]);

        free(info.programName);
        info.programName = temp;
    }
    write (fd_write, message->content,sizeof(char) * message->lenght);
    close (fd_write);
    /*----------------------------------------------------------------*/
    
    if(info.pedido != 'c'){
        if((self_read = open(info.programName,O_RDONLY)) == -1){
            perror("open");
            return 2;
        }
        char *buffer = malloc(512*sizeof(char));
        ssize_t bytes_read;
        ssize_t byteswriten;
        int loopcount = 0;
        off_t offset;

        while((bytes_read = read(self_read, buffer, 512)) != 0){
            if(bytes_read > 0){
                loopcount = 0;
            do {
               byteswriten = write(1,buffer,(bytes_read));
               if(loopcount > 10){
                perror("write");
                exit(7);
               } 
               loopcount++;
            }while(byteswriten == -1);
            }else{
                loopcount++;
                if(loopcount>10){
                    perror("read");
                    exit(8);
                }
            };
        }
        if(bytes_read == -1){
            printf("Erro\n");
        }

        free(buffer);
        close (self_read);
        unlink(info.programName);
    }
    free(argumments);
    return 0;
} 

