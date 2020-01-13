#include "symbol.h"
#include "error.h"

symbol ** sym_table = NULL;
size_t sym_table_count = 0;
size_t sym_table_size  = 0;

symbol * CreateSymbol(char * str) {
	symbol * sym = malloc(sizeof(symbol));
	sym->str = str;
	sym->ref_count = 1;
	return sym;
}

void FreeSymbol(symbol * sym) {
	if (sym) {
		if (sym->str) {
			free(sym->str);
			sym->str = NULL;
		}
		free(sym);
	}
}

int InitSymTable(size_t init_size) {
	sym_table = malloc(sizeof(symbol*) * init_size);
	if (!sym_table) {
		Scheme_SetError("InitSymTable : malloc() error");
		return 0;
	}

	sym_table_count = 0ll;
	sym_table_size = init_size;
	return 1;
}

int ResizeSymTable(size_t new_size) {
	sym_table = realloc(sym_table, sizeof(symbol*) * new_size);
	if (!sym_table) {
		Scheme_SetError("ResizeSymTable : realloc() error");
		return 0;
	}

	sym_table_size = new_size;
	return 1;
}

void FreeSymTable(void) {
	if (!sym_table) return;

	size_t i;
	for (i = 0; i < sym_table_count; ++i) {
		FreeSymbol(sym_table[i]);
	}

	free(sym_table);
}

symbol * GetSymbol(const char * str) {
	size_t i;
	for (i = 0; i < sym_table_count; ++i) {
		int comp = strcmp(str, sym_table[i]->str);
		if (!comp) return sym_table[i];
	}
	return NULL;
}

// if str already exists in the symbol table, it
// returns the already existing symbol and
// increments its ref_count
symbol * AddSymbol(char * str) {
	// check if symbol already exists
	symbol * get = GetSymbol(str);
	if (get) {
		free(str);
		++get->ref_count;
		return get;
	}

	if (sym_table_count + 1 == sym_table_size) {
		if (!ResizeSymTable(sym_table_size * 2))
			return NULL;
	}

	symbol * new_sym = CreateSymbol(str);

	symbol ** l = sym_table;
	symbol ** r = sym_table + sym_table_count;

	// binary insert on sorted sym_table array
	while (1) {
		if (r == l) {
			symbol ** move,
			       ** end = sym_table + sym_table_count;
			for (move = end-1; move >= l; --move) {
				move[1] = move[0];
			}

			*l = new_sym;
			++sym_table_count;
			return *l;
		}

		symbol ** m = l + (r-l)/2;

		if (m[0]->str < str) {
			l = m + 1;
		} else if (m[0]->str > str) {
			r = m;
		} else if (str == m[0]->str) { // this shouldn't happen ever
			return NULL;
		}
	}
}

void ReferenceSymbol(symbol ** pointer, symbol * sym) {
	*pointer = sym;
	++(*pointer)->ref_count;
}

void DereferenceSymbol(symbol ** sym) {
	if (*sym) {
		--(*sym)->ref_count;
		if ((*sym)->ref_count == 0) {
			size_t index = GetSymTableIndex(*sym);
			EraseSymTable(index);
		}
	}
	*sym = NULL;
}

void EraseSymTable(size_t index) {
	if (index == -1) return;
	FreeSymbol(sym_table[index]);

	size_t i;
	for (i = index; i < sym_table_count - 1; ++i) {
		sym_table[i] = sym_table[i+1];
	}
	--sym_table_count;
}

int GetSymTableIndex(symbol * sym) {
	symbol ** l = sym_table;
	symbol ** r = sym_table + sym_table_count;

	while (1) {
		if (r == l) {
			return -1;
		}

		symbol ** m = l + (r-l)/2;

		if (sym->str > m[0]->str) {
			l = m + 1;
		} else if (sym->str < m[0]->str) {
			r = m;
		} else if (sym->str == m[0]->str) {
			return m - sym_table;
		}
	}
}

#include <stdio.h>
void DisplaySymbolTable(void) {
	puts("-----");
	size_t i;
	for (i = 0; i < sym_table_count; ++i) {
		symbol * s = sym_table[i];
		printf("'%s' address %lli\n", s->str, (long long int)s);
	}
	puts("-----");
}
