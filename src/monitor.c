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

int main(int argc, char **argv){
    
    if (mkfifo("fifo",0666)==0)
        perror("mkfifo"); 

    int log;
    int fd2;
    int fd,bytes_read;
    struct Info buf;
        if((fd = open("fifo",O_RDONLY)) == -1){
            perror("open");
            return -1;
        }else{
            printf("opened");
        }
        
        if((fd2 = open("fifo",O_WRONLY)) == -1){
            perror("open");
            return -1;
        }else{
            printf("opened");
        }

        if((log = open("log.txt",O_CREAT|O_WRONLY,0640)) == -1){
            perror("open");
            return -1;
        }else{
            printf("opened");
        }

        while ((bytes_read = read (fd, &buf,sizeof(struct Info))) > 0)
            // write (log, &buf.pid,sizeof(int));
            // write (log, &bytes_read,sizeof(int));
            //write (log, buf.programName,sizeof(char)*255);
        close (fd);
}
//se quiserem responder ao cliente cada cliente precisa tambem de um fifo e 
//mandar a informação necessaria para o servidor aceder a esse fifo.