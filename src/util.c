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
        if(i >= tollerance[0] -1){
            char ***aux = (char***)realloc(arguments, sizeof(char**) * tollerance[0] * 2);
            if(aux != NULL){
               int *aux2 = (int*)realloc(tollerance, sizeof(int) * tollerance[0] * 2);
               if(aux2 != NULL){
                tollerance = aux2;
                int temp = tollerance[0];
                tollerance[0] = tollerance[0] * 2;
               for(; temp < tollerance[0]; temp++) tollerance[temp] = 1000;
               }
               arguments = aux;
            }
        }
        arguments[i] = Tracer_parser(token, &tollerance[i+1]);
        current_word = k;
        for(;(message[k] != '|') && (message[k] != '\0');k++);
        if(message[k] != '\0'){
            message[k] = '\0';
            k++;
        }
        if(message[current_word] != '\0')
            token = strdup(&message[current_word]);
        else token = NULL;
    }
    arg[0] = i;
    arguments[i] = NULL;
    return arguments;
}



struct String *to_String(struct Info info, struct timeval time, char* arguments)
{
    struct String *result = malloc(sizeof(struct String));
    result->lenght = snprintf(NULL, 0, "%c %d %s %ld.%06ld %s\n", info.pedido ,info.pid, info.programName, time.tv_sec, time.tv_usec, arguments) + 1;
    result->content = malloc(result->lenght);

    if(result->content == NULL) return NULL;
    snprintf(result->content, result->lenght, "%c %d %s %ld.%06ld %s\n", info.pedido ,info.pid, info.programName, time.tv_sec, time.tv_usec, arguments);
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

