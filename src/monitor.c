#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

struct String {
    int lenght;
    char *content;
};


struct MSG { //se o programName for NULL então é final
    char pedido;
    int pid;
    char* programName;//buffer[0]
    struct timeval time;
    //timestamp;    
};

typedef struct log_entries{
 char *program_name;
 struct timeval start_time;
 struct timeval end_time;
 int pid;
 int pending_position;
}Entry;

typedef struct log
{
    Entry *entries;
    int *pending;
    int size;
    int starting_value;
   unsigned int entry_size;
   unsigned int pending_size;
    int starting_position;
    int current_index;
}Log;

Log * newLog(){
    Log *log = calloc(1 , sizeof(Log));
    log->starting_value = getpid();
    log->size = 100000;
    log->pending_size = 10000;
    log->entries = calloc(100000, sizeof(Entry)); 
    log->pending = calloc(10000, sizeof(int));
    return log;
}

void upDateTable(Log *log, struct MSG entry){
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
    printf("REMOVING\n");
    log->entries[x].end_time = entry.time;
    printf("%d\n",     log->pending[log->entries[x].pending_position]);
    log->pending[log->entries[x].pending_position] = 0;
    printf("%d\n",     log->pending[log->entries[x].pending_position]);
 }
}

struct String *Status_to_string(Entry entry, double time)
{
   struct String *result = malloc(sizeof(struct String));
    result->lenght = snprintf(NULL, 0, "PID: %d NAME: %s TIME: %lf \n", entry.pid, entry.program_name, time) + 1;
   result->content = malloc(result->lenght);
   if(result->content == NULL) return NULL;
   snprintf(result->content, result->lenght, "PID: %d NAME: %s TIME: %lf \n", entry.pid, entry.program_name, time);
   return result;
}

void status(Log *log, struct MSG info){
    int fd;
    printf("%s\n", info.programName);
    if (mkfifo(info.programName,0666)==0)
    perror("mkfifo");
    if((fd = open(info.programName, O_WRONLY)) == -1){
        printf("ERRO\n");
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
        string = Status_to_string(log->entries[log->pending[starting_position]], (((double)(timestamp.tv_sec - log->entries[log->pending[starting_position]].start_time.tv_sec)*1000) + ((double)timestamp.tv_usec - log->entries[log->pending[starting_position]].start_time.tv_usec)/1000));
        write(fd, string->content, string->lenght*sizeof(char*));
        }
    }
      free(string->content);
    close(fd);
}


int Server_parser (char *message, struct MSG *store){
    char *message_copy = strdup(message);
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
          }else if(i == 6){
            sscanf(token, "%ld.%06ld", &store->time.tv_sec, &store->time.tv_usec);
          }

        token = strtok(NULL, " ");
        index += size + 1;
    }
    free(message_copy);
    return index - 2;
}

int main(int argc, char **argv){
    Log *log = newLog();
    char bool = 1;
    if (mkfifo("fifo",0666)==0)
        perror("mkfifo"); 
    
    int arquive;
    int fd_read, fd_write,bytes_read;
    struct MSG info;
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
        if((arquive = open("arquive.txt",O_CREAT|O_WRONLY,0640)) == -1){
            perror("open");
            return 3;
        }
        lseek(arquive,0,SEEK_END);

        while (bool && (bytes_read = read (fd_read, buffer, 512)) > 0){
            printf("%s \n", buffer);
            lseek(fd_read, -(bytes_read - Server_parser(buffer, &info)), SEEK_CUR);
            switch (info.pedido)
            {
            case 's':
               if((pid = fork()) == 0){
                status(log, info);
                exit(1);
               }
                break;
            case 'c':
                bool = 0;
                break;
            case 'e':
                upDateTable(log, info);
                //escrever no ficheiro correspondente ao pid (pode ser feito num fork para melhor concorrêmcia)
                break;
            default:
            break;
            }
                write (arquive, buffer,strlen(buffer));
        }
        free(info.programName);
        free(buffer);
        close (fd_read);
        close(fd_write);
        unlink("fifo");
}
//se quiserem responder ao cliente cada cliente precisa tambem de um fifo e 
//mandar a informação necessaria para o servidor aceder a esse fifo.