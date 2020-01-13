#pragma once

#include <string.h>
#include <stdlib.h>

#define SYM_TABLE_INIT_SIZE 128

// once a symbol has been made, it will live on
// until it has no more references

// sym_table is sorted by char* string pointers integer values

typedef struct symbol {
	char * str;
	int ref_count;
} symbol;

symbol* CreateSymbol(char * str);
void    FreeSymbol(symbol * sym);

extern symbol ** sym_table;
extern size_t sym_table_count;
extern size_t sym_table_size;

// 1 for success
// 0 for error
int InitSymTable(size_t init_size);
int ResizeSymTable(size_t new_size);
void FreeSymTable(void);

void EraseSymTable(size_t index);
int GetSymTableIndex(symbol * sym);

symbol * GetSymbol(const char * str);

// if str already exists in the symbol table, it
// returns the alreadyy existing symbol and
// increments its ref_count
symbol * AddSymbol(char * str);

void ReferenceSymbol(symbol ** pointer, symbol * sym);
void DereferenceSymbol(symbol ** sym);

void DisplaySymbolTable(void);
