
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


void freeTable(HT* table);

int calculateIdentifier(char *string);

HT * newTable();

void updateHT(char *string, HT *table);
