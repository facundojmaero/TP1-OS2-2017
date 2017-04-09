CC = gcc
CFLAGS = -Wall -pedantic -Werror
ODIR=obj
LDIR=include
SRCDIR=src
BDIR=build

all: make_dirs sock_cli_i_cc sock_srv_i_cc

make_dirs:
	mkdir -p obj
	mkdir -p build

sock_cli_i_cc: sock_cli_i_cc.o
	gcc $(ODIR)/$< -o $(BDIR)/$@

sock_srv_i_cc: sock_srv_i_cc.o
	gcc $(ODIR)/$< -o $(BDIR)/$@

# $(PROG): $(OBJS)
#     $(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

sock_cli_i_cc.o: $(SRCDIR)/sock_cli_i_cc.c
	$(CC) $(CFLAGS) -c $< -o $(ODIR)/$@

sock_srv_i_cc.o: $(SRCDIR)/sock_srv_i_cc.c $(LDIR)/colors.h
	$(CC) $(CFLAGS) -c $< -o $(ODIR)/$@

cppcheck:
	@echo
	@echo Realizando verificacion CppCheck
	@echo Los resultados de la misma se encuentran en err.txt
	cppcheck --enable=all . 2> err.txt

clean:
	@echo Borrando archivos
	rm -rf *o $(ODIR) $(BDIR) $(LDIR)/*.gch err.txt
