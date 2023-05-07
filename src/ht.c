#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "include/ht.h"

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
