BUILDDIR := objects/
SRCFILES = $(wildcard *.c)
OBJFILES = $(SRCFILES:%.c=$(BUILDDIR)%.o)

program: $(OBJFILES) $(SRCFILES) # só compila o programa (o que for necessário)
	gcc tracer.c -g -o tracer
	gcc monitor.c -g -o monitor

objects: # constroi a diretoria com os objects
	mkdir -p $@

$(BUILDDIR)%.o: %.c | objects # cria os objetos
	$(CC) -o "$@" -c "$<"

clean: #apaga td o que foi criado
	rm -rf $(BUILDDIR) monitor tracer fifo fifo_*
