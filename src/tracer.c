#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>


struct Info { //se o programName for NULL então é final
    int pid;
    char* programName;//buffer[0]
    //timestamp;    
};


int main(int argc, char **argv){
    
    if (argc < 2){
        return -1;
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
            if ( (a =fork()) == 0){
                struct Info inicial;
                inicial.pid = a;
                inicial.programName = strdup(argv[3]);

                printf("Running PID %d\n", inicial.pid);

//-----------------------------------------------------------------
//--------------------Notificar servidor---------------------------

                if((fd = open("fifo",O_WRONLY)) == -1)
                    return -1;
                // mandar a informação para o server
                printf("\nprogram name: %s|\n",inicial.programName);
                write (fd, &inicial,sizeof(struct Info));
                close (fd);

//----------------------------------------------------------------
//--------------------fazer execute -u----------------------------
                execvp(argv[3],buffer);
            }
            wait(0);
//-----------------------------------------------------------------
//--------------------Notificar servidor---------------------------
            
            if((fd = open("fifo",O_WRONLY)) == -1)
                return -1;
            
            // mandar a informação para o server
            struct Info final;
            final.pid = a;
            
            final.programName = NULL;
            write (fd, &final,sizeof(struct Info));
            close (fd);

//preencher a estrutura EINFO e mandar a informação para o server
//-----------------------------------------------------------------
//--------------------Notificar cliente----------------------------
                printf("ended\n");
                exit(5);
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

