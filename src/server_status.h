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


Log * newLog();

void upDateTable(Log *log, struct MSG entry, char *directory);

struct String *Status_to_string(Entry entry, double time);

void status(Log *log, struct MSG info);

void stats_uniq(Log *log, struct MSG info);

void stats_command(Log *log, struct MSG info);

void stats_time(Log *log, struct MSG info);
