IDIR =include
CC=clang
CFLAGS=-I$(IDIR) -O3

ODIR=src/obj
SDIR=src

LIBS=-lm -pthread

_DEPS = lexer.h parser.h list.h object.h error.h list.h scheme.h scope.h std.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o lexer.o parser.o list.o object.o error.o list.o scheme.o scope.o std.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

OUTPUT = scheme

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) -g -c -o $@ $<
		
debug: $(OBJ)
	$(CC) -g -o $(OUTPUT) $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~

-include $(OBJ:.o=.d)
