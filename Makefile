BUILDDIR := objects/
SRCFILES = $(wildcard src/*.c)
OBJFILES = $(SRCFILES:src/%.c=$(BUILDDIR)%.o)

program: folders $(OBJFILES)  # só compila o programa (o que for necessário)
	gcc src/util.c src/status.c src/execute.c src/tracer.c -g -o tracer
	gcc src/ht.c src/server_status.c src/monitor.c -g -o monitor

folders: pid_files

monitor: program
	./monitor pid_files

objects: # constroi a diretoria com os objects
	mkdir -p $@

pid_files: # constroi a diretoria com os fichas
	mkdir -p $@

$(BUILDDIR)%.o: src/%.c | objects # cria os objetos
	$(CC) -o "$@" -c "$<"

clean: #apaga td o que foi criado
	rm -rf $(BUILDDIR) monitor tracer fifo fifo_* pid_files/*
