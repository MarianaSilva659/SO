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
#include "biblioteca/execute.h"
#include "biblioteca/status.h"


int main(int argc, char **argv){
    struct String *message;
    if (argc < 2)
        return 1;

    if (strcmp(argv[1], "execute") == 0){
        if (strcmp(argv[2], "-u") == 0){
                execute_U(argv[3]);
                return 0;
        }
        else if(strcmp(argv[2], "-p") == 0){
            execute_P(argv[3]);
        }
        return 0;
    }else { //status
        return status(argc, argv);
    }
}

