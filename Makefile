BUILDDIR := objects/
SRCFILES = $(wildcard src/*.c)
OBJFILES = $(SRCFILES:src/%.c=$(BUILDDIR)%.o)

program: folders $(OBJFILES)  # só compila o programa (o que for necessário)
	gcc src/tracer.c -g -o bin/tracer
	gcc src/monitor.c -g -o bin/monitor

folders: bin pid_files

monitor: program
	./bin/monitor pid_files

bin:
	mkdir -p $@

objects: # constroi a diretoria com os objects
	mkdir -p $@

pid_files: # constroi a diretoria com os fichas
	mkdir -p $@

$(BUILDDIR)%.o: src/%.c | objects # cria os objetos
	$(CC) -o "$@" -c "$<"

clean: #apaga td o que foi criado
	rm -rf $(BUILDDIR) bin/* fifo fifo_* pid_files/*
