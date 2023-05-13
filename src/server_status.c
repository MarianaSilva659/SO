#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "include/server_status.h"
#include "include/ht.h"

Log * newLog(){
    Log *log = calloc(1 , sizeof(Log));
    log->starting_value = getpid();
    log->size = 100000;
    log->pending_size = 10000;
    log->entries = calloc(100000, sizeof(Entry)); 
    log->pending = calloc(10000, sizeof(int));
    return log;
}

void upDateTable(Log *log, struct MSG entry, char *directory){
    const int x = entry.pid - log->starting_value;
    char loop_count = 0;
       while(x >= log->size){
        if(loop_count > 10) exit(9);
        Entry *aux = (Entry *)realloc(log->entries, log->size * 2 * sizeof(Entry));
        if(aux != NULL){
            log->size == log->size << 1;
            log->entries = aux;
            loop_count =0;
        }else loop_count++;
    }

 if(log->entries[x].pid == 0){
    log->entries[x].program_name = strdup(entry.programName);
    log->entries[x].start_time = entry.time;
    log->entries[x].pid = entry.pid;
     log->entries[x].pending_position = log->current_index;
         while(log->current_index >= log->pending_size){
        if(loop_count > 10) exit(10);
        int *aux = (int *)realloc(log->entries, log->pending_size * 2 * sizeof(int));
        if(aux != NULL){
            log->pending_size == log->pending_size << 1;
            log->pending = aux;
        } else loop_count++;
    }
    log->pending[log->current_index] = x;
    log->current_index++;
 }else{
    log->entries[x].end_time = entry.time;
    log->pending[log->entries[x].pending_position] = 0;
    log->entries[x].pending_position = -1;
    char *aux;
    int arquive;
            int tamanho = snprintf(NULL, 0, "%s/fifo_%d.txt",directory, entry.pid) + 1;
            aux = malloc(tamanho);
            if(aux == NULL) return;
            snprintf(aux, tamanho, "%s/fifo_%d.txt",directory, entry.pid);

            if((arquive = open(aux,O_CREAT|O_WRONLY,0640)) == -1){
            perror("open");
            return;
        }
        char *message;
        double timestamp =(double)(log->entries[x].end_time.tv_sec - log->entries[x].start_time.tv_sec)*1000 + (double)(log->entries[x].end_time.tv_usec - log->entries[x].start_time.tv_usec)/1000;
        tamanho = snprintf(NULL, 0, "Total execution time of program %s was %lf ms", log->entries[x].program_name, timestamp)+1;
        message = malloc(tamanho * sizeof(char));
        snprintf(message, tamanho, "Total execution time of program %s was %lf ms", log->entries[x].program_name, timestamp);
        ssize_t byteswriten;
        int i =0;
      do {
        byteswriten = write (arquive, message,(tamanho-1) * sizeof(char)); 
        if(i >= 10) break;
        i++;
      }while(byteswriten == -1);
        free(message);
 }
}

struct String *Status_to_string(Entry entry, double time)
{
   struct String *result = malloc(sizeof(struct String));
    result->lenght = snprintf(NULL, 0, "%d %s %lf \n", entry.pid, entry.program_name, time) + 1;
   result->content = malloc(result->lenght);
   if(result->content == NULL) return NULL;
   snprintf(result->content, result->lenght, "%d %s %lf \n", entry.pid, entry.program_name, time);
   return result;
}

void status(Log *log, struct MSG info){
    int fd;
    if (mkfifo(info.programName,0666)==0)
    perror("mkfifo");
    if((fd = open(info.programName, O_WRONLY)) == -1){
        write(1,"ERRO\n",6);
        exit(1);
    }
    const int limit = log->current_index;
    int starting_position = log->starting_position;
    for(;(log->pending[starting_position] == 0) && (starting_position < limit); starting_position++);
    log->starting_position = starting_position;
    struct timeval timestamp;
    struct String* string;
    gettimeofday(&timestamp, NULL);
    if(starting_position < limit)
    for(; starting_position < limit; starting_position++){
        if(log->pending[starting_position] > 0){
        ssize_t byteswriten;
        int i =0;
        string = Status_to_string(log->entries[log->pending[starting_position]], (((double)(timestamp.tv_sec - log->entries[log->pending[starting_position]].start_time.tv_sec)*1000) + ((double)timestamp.tv_usec - log->entries[log->pending[starting_position]].start_time.tv_usec)/1000));
              do {
        byteswriten =  write(fd, string->content, string->lenght);
        if(i >= 10) exit(99);
        i++;
      }while(byteswriten == -1);
        }
    }
      free(string->content);
    close(fd);
    exit(0);
}

void stats_uniq(Log *log, struct MSG info){
    int fd;
      if (mkfifo(info.programName,0666)==0)
    perror("mkfifo");
    if((fd = open(info.programName, O_WRONLY)) == -1){
        write(1,"ERRO\n",6);
        exit(1);
    }
    HT* table =newTable();

    const int limit = info.current_index;
    const int inicial_pid = log->starting_value;
    int x;
    for(int i = 0; i < limit; i++){
        x = info.arguments[i]- inicial_pid;
        if(x>=0 && (info.arguments[i] == log->entries[x].pid) && (log->entries[x].pending_position == -1)){
        updateHT(log->entries[x].program_name, table);
        }
    }
    const int max = table[0].current_valid_entry;
    for(int i = 0; i < max; i++){
        write(fd, table[0].valid_entries[i]->name, (strlen(table[0].valid_entries[i]->name))+1);
        write(fd, "\n", 1);
    } 
    close(fd);
    freeTable(table);
}

void stats_command(Log *log, struct MSG info){
    int fd;
    char *fifo;
    int tamanho = snprintf(NULL, 0, "fifo_%d", info.pid) + 1;
    fifo = malloc(tamanho*sizeof(char));
    if(fifo == NULL) return;
    snprintf(fifo, tamanho, "fifo_%d", info.pid);
    if (mkfifo(fifo,0666)==0)
    perror("mkfifo");
    if((fd = open(fifo, O_WRONLY)) == -1){
        write(1,"ERRO\n",6);
        exit(1);
    }
    HT* table =newTable();
    const int limit = info.current_index;
        int temp;
    for(int i = 0; i <  limit; i++){
        temp = info.arguments[i];
        for(int j = i+1; j < limit; j++){
            if(temp == info.arguments[j]) info.arguments[j] = -1;
        }
    }
    const int inicial_pid = log->starting_value;
    int x;
    int count = 0;
    for(int i = 0; i < limit; i++){
        x = info.arguments[i]- inicial_pid;
        if(x>=0 && (info.arguments[i] == log->entries[x].pid) && (log->entries[x].pending_position == -1))
        if(strcmp(log->entries[x].program_name, info.programName) == 0)count++;
    }
    x = snprintf(NULL, 0, "%s was executed %d\n", info.programName, table[0].current_valid_entry) + 1;
    char* aux = malloc(x * sizeof(char));
    snprintf(aux, x, "%s was executed %d\n", info.programName, count) + 1;
    write(fd, aux, x);
    close(fd);
    free(aux);
    freeTable(table);
    free(fifo);
}

void stats_time(Log *log, struct MSG info){
    int fd;
      if (mkfifo(info.programName,0666)==0)
    perror("mkfifo");
    if((fd = open(info.programName, O_WRONLY)) == -1){
        write(1,"ERRO\n",6);
        exit(1);
    }
    const int limit = info.current_index;
    const int inicial_pid = log->starting_value;
    int x;
    double total = 0;
    int temp;
        for(int i = 0; i <  limit; i++){
        temp = info.arguments[i];
        for(int j = i+1; j < limit; j++){
            if(temp == info.arguments[j]) info.arguments[j] = -1;
        }
    }
    for(int i = 0; i < limit; i++){
        x = info.arguments[i]- inicial_pid;
        if(x>=0 && (info.arguments[i] == log->entries[x].pid) && (log->entries[x].pending_position == -1))
        total += (log->entries[x].end_time.tv_sec - log->entries[x].start_time.tv_sec)*1000 + (((double)log->entries[x].end_time.tv_usec - log->entries[x].start_time.tv_usec)/1000);
    }
    x = snprintf(NULL, 0, "Total execution time is %lf ms\n", total) + 1;
    char* aux = malloc(x * sizeof(char));
    snprintf(aux, x, "Total execution time is %lf ms\n", total) + 1;
    write(fd, aux, x);
    free(aux);
    close(fd);
}

