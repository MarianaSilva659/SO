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
    int current_index;
    int max_size;
    int* arguments;
    struct timeval time;
    //timestamp;    
};

typedef struct Stats_hash_table_entry {
    char* name;
    char status;
    int current_copycat;
    int copycat_size;
    struct Stats_hash_table_entry *copycats;
}HTE;

typedef struct Stats_hash_table{
    HTE *table;
    int valid_entries_size;
    int current_valid_entry;
    HTE **valid_entries;
}HT;

void freeTable(HT* table){
    for(int i = 0; i < 200003; i++){
        if(table[0].table[i].status == 'V'){
            if(table[0].table[i].copycat_size > 0){
            for(int k; k < table[0].table[i].current_copycat; k++){
                free(table[0].table[i].copycats[k].name);
            }
            free(table[0].table[i].copycats);
            }
            free(table[0].table[i].name);
        }
    }
    free(table[0].table);
    free(table[0].valid_entries);
    free(table);
}

int calculateIdentifier(char *string){
    unsigned long int identifier = 0;
    for(int i = 0; string[i] != '\0'; i++){
        identifier = (identifier << 5) + identifier + string[i];
    }
    identifier = identifier % 200003;
    return identifier;
}

HT * newTable(){
    HT* table = calloc(1, sizeof(HT));
    table[0].table = calloc(200003, sizeof(HTE));
    table[0].valid_entries = calloc(100000, sizeof(HTE*));
    table[0].valid_entries_size = 100000;
    return table;
}

void updateHT(char *string, HT *table){
    int key = calculateIdentifier(string);
    if(table[0].table[key].status == 'V'){
        if(strcmp(table[0].table[key].name, string) != 0){
        if(table[0].table[key].copycat_size == 0){
            table[0].table[key].copycat_size = 4;
            table[0].table[key].copycats = calloc(4, sizeof(HTE));
        }
        char count = 0;
        while(table[0].table[key].current_copycat >= table[0].table[key].copycat_size){
            if(count > 10){
                char *message = strdup("REALLOC FALHOU, ABORTANDO COMANDO\n");
                write(1, message, (strlen(message)+1) * sizeof(char));
                free(message);
                exit(100);
            }
            HTE* aux = realloc(table[0].table[key].copycats, table[0].table[key].copycat_size * 2 * sizeof(HTE));
            if(aux != NULL){
                table[0].table[key].copycats = aux;
                for(int i = 0; i < table[0].table[key].current_copycat; i++){
                    table[0].valid_entries[table[0].table[key].copycats[i].current_copycat] = &table[0].table[key].copycats[i];
                }
                table[0].table[key].copycat_size *= 2;
                count = 0;
            }else count++;
        }
         table[0].table[key].copycats[table[0].table[key].current_copycat].name = strdup(string);
            while(table[0].current_valid_entry >= table[0].valid_entries_size){
            if(count > 10){
                char *message = strdup("REALLOC FALHOU, ABORTANDO COMANDO\n");
                write(1, message, (strlen(message)+1) * sizeof(char));
                free(message);
                exit(100);
            }
            HTE** aux = realloc(table[0].valid_entries, table[0].valid_entries_size * 2 * sizeof(HTE*));
            if(aux != NULL){
                table[0].valid_entries = aux;
                table[0].valid_entries_size *= 2;
            }else count++;
        }
         table[0].valid_entries[table[0].current_valid_entry] = & table[0].table[key].copycats[table[0].table[key].current_copycat];
         table[0].table[key].copycats[table[0].table[key].current_copycat].current_copycat = table[0].current_valid_entry;
         table[0].current_valid_entry++;
         table[0].table[key].current_copycat++;
        }else return;
    }else{
        table[0].table[key].name = strdup(string);
        table[0].table[key].status = 'V';
        char count = 0;
                   while(table[0].current_valid_entry >= table[0].valid_entries_size){
            if(count > 10){
                char *message = strdup("REALLOC FALHOU, ABORTANDO COMANDO\n");
                write(1, message, (strlen(message)+1) * sizeof(char));
                free(message);
                exit(100);
            }
            HTE** aux = realloc(table[0].valid_entries, table[0].valid_entries_size * 2 * sizeof(HTE*));
            if(aux != NULL){
                table[0].valid_entries = aux;
                table[0].valid_entries_size *= 2;
            }else count++;
        }
        table[0].valid_entries[table[0].current_valid_entry] = &table[0].table[key];
        table[0].current_valid_entry++;
    }
}

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
            printf("aux = %s\n",aux);
            if((arquive = open(aux,O_CREAT|O_WRONLY,0640)) == -1){
            perror("open");
            return;
        }
        char *message;
        double timestamp =(double)(log->entries[x].end_time.tv_sec - log->entries[x].start_time.tv_sec)*1000 + (double)(log->entries[x].end_time.tv_usec - log->entries[x].start_time.tv_usec)/1000;
        tamanho = snprintf(NULL, 0, "Total execution time of program %s was %lf ms", log->entries[x].program_name, timestamp)+1;
        message = malloc(tamanho * sizeof(char));
        snprintf(message, tamanho, "Total execution time of program %s was %lf ms", log->entries[x].program_name, timestamp);
        write (arquive, message,(tamanho-1) * sizeof(char));
        free(message);
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

void stats_uniq(Log *log, struct MSG info){
    int fd;
      if (mkfifo(info.programName,0666)==0)
    perror("mkfifo");
    if((fd = open(info.programName, O_WRONLY)) == -1){
        printf("ERRO\n");
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
        write(fd, table[0].valid_entries[i]->name, (strlen(table[0].valid_entries[i]->name)+1) * sizeof(char));
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
    printf("FIOF = %s\n", fifo);
    if (mkfifo(fifo,0666)==0)
    perror("mkfifo");
    if((fd = open(fifo, O_WRONLY)) == -1){
        printf("ERRO\n");
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
    write(fd, aux, x * sizeof(char));
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
        printf("ERRO\n");
         exit(1);
    }
    const int limit = info.current_index;
    const int inicial_pid = log->starting_value;
    int x;
    double total = 0;
    for(int i = 0; i < limit; i++){
        x = info.arguments[i]- inicial_pid;
        if(x>=0 && (info.arguments[i] == log->entries[x].pid) && (log->entries[x].pending_position == -1))
        total += (log->entries[x].end_time.tv_sec - log->entries[x].start_time.tv_sec)*1000 + (((double)log->entries[x].end_time.tv_usec - log->entries[x].start_time.tv_usec)/1000);
    }
    x = snprintf(NULL, 0, "Total execution time is %lf ms\n", total) + 1;
    char* aux = malloc(x * sizeof(char));
    snprintf(aux, x, "Total execution time is %lf ms\n", total) + 1;
    printf("aux:%s\n", aux);
    write(fd, aux, x*sizeof(char));
    free(aux);
    close(fd);
}

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