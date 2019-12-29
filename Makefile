IDIR =include
CC=gcc
CFLAGS=-I$(IDIR) -std=c11 -O3

ODIR=src/obj
SDIR=src

LIBS=-lm -pthread

_DEPS = lexer.h parser.h list.h object.h error.h list.h scheme.h map.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o lexer.o parser.o list.o object.o error.o list.o scheme.o map.c 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

OUTPUT = scheme

$(ODIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
		
debug: $(OBJ)
	$(CC) -g -o $(OUTPUT) $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~

-include $(OBJ:.o=.d)
