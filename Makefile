CC = gcc
CFLAGS = -Wall -pedantic -Werror
ODIR=obj
LDIR=include
SRCDIR=src
BDIR=build
PATHOBJECTS_SERVER=$(addprefix $(ODIR)/,$(OBJECTS_SERVER))
PATHOBJECTS_CLIENTE=$(addprefix $(ODIR)/,$(OBJECTS_CLIENTE))
OBJECTS_SERVER=sock_srv_i_cc.o funciones_server.o funciones_io.o
OBJECTS_CLIENTE=sock_cli_i_cc.o funciones_io.o funciones_cliente.o

all: make_dirs build/sock_cli_i_cc build/sock_srv_i_cc

make_dirs:
	mkdir -p obj
	mkdir -p build

$(BDIR)/sock_cli_i_cc: $(PATHOBJECTS_CLIENTE)
	gcc $(PATHOBJECTS_CLIENTE) -o $@

$(BDIR)/sock_srv_i_cc: $(PATHOBJECTS_SERVER)
	gcc $(PATHOBJECTS_SERVER) -o $@

$(ODIR)/sock_cli_i_cc.o: $(SRCDIR)/sock_cli_i_cc.c $(LDIR)/colors.h $(LDIR)/funciones_cliente_cc.h $(LDIR)/comunes.h
	$(CC) $(CFLAGS) -c $< -o $@

$(ODIR)/sock_srv_i_cc.o: $(SRCDIR)/sock_srv_i_cc.c $(LDIR)/colors.h $(LDIR)/funciones_servidor_cc.h $(LDIR)/comunes.h
	$(CC) $(CFLAGS) -c $< -o $@

$(ODIR)/funciones_server.o: $(SRCDIR)/funciones_server.c
	$(CC) $(CFLAGS) -c $< -o $@	

$(ODIR)/funciones_cliente.o: $(SRCDIR)/funciones_cliente.c $(SRCDIR)/funciones_io.c
	$(CC) $(CFLAGS) -c $< -o $@	

$(ODIR)/funciones_io.o: $(SRCDIR)/funciones_io.c
	$(CC) $(CFLAGS) -c $< -o $@	

cppcheck:
	@echo
	@echo Realizando verificacion CppCheck
	@echo Los resultados de la misma se encuentran en err.txt
	cppcheck --enable=all . 2> err.txt

clean:
	@echo Borrando archivos
	rm -rf *o $(ODIR) $(BDIR) $(LDIR)/*.gch err.txt
