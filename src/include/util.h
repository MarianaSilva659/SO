struct Info {
    char pedido;
    int pid;
    char* programName;
};

struct String {
    int lenght;
    char *content;
};

char** Tracer_parser (char *message, int *tollerance);
char***Pipeline_Parser(char *message, int *tollerance, int *arg);
struct String *to_String(struct Info info, struct timeval time, char* arguments);
struct String *Status_to_String(struct Info info);