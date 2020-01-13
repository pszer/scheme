#include "scope.h"
#include "scheme.h"

scheme_define Scheme_CreateDefine(symbol * sym, scheme_object * obj) {
	scheme_define def;
	def.sym = sym;
	def.object = obj;
	return def;
}

scheme_define Scheme_CreateDefineString(char * string, scheme_object * obj) {
	symbol * sym = AddSymbol(string);
	return Scheme_CreateDefine(sym, obj);
}

void Scheme_FreeDefine(scheme_define * def) {
	if (!def) return;
	if (def->sym) DereferenceSymbol(&def->sym);
	Scheme_DereferenceObject(&def->object);
}

void Scheme_OverwriteDefine(scheme_define * def, scheme_object * obj) {
	if (def->object) Scheme_DereferenceObject(&def->object);
	def->object = obj;
}

/*char LexigraphicCompare(const char * a, const char * b) {
	const char * c = a, * d = b;
	while (1) {
		if (!*c &&  *d) return -1; // ab  < abc
		if ( *c && !*d) return  1; // abc > ab
		if (!*c && !*d) return  0; // abc = abc

		if (*c < *d) return -1; // a < b
		if (*c > *d) return  1; // b > a

		// *c==*d if no checks above happened

		// check next character
		++c;
		++d;
	};
}*/

scheme_env Scheme_CreateEnv(scheme_object * parent, int init_size) {
	scheme_env env;
	if (parent) {
		Scheme_ReferenceObject(&env.parent, parent);
	} else {
		env.parent = NULL;
	}

	env.defs = NULL;
	Scheme_ResizeEnv(&env, init_size);
	env.count = 0;

	return env;
}

void Scheme_FreeEnv(scheme_env * env) {
	if (env->defs) {
		size_t i;
		for (i = 0; i < env->count; ++i) {
			Scheme_FreeDefine(env->defs + i);
		}
		free(env->defs);
	}

	if (env->parent) {
		Scheme_DereferenceObject(&env->parent);
	}
}

void Scheme_ResizeEnv(scheme_env * env, int new_size) {
	env->defs = realloc(env->defs, new_size * sizeof(scheme_define));
	env->size = new_size;
}

void Scheme_DefineEnv(scheme_env * env, scheme_define def) {
	// expand size if necessary
	if (env->count + 1 == env->size)
		Scheme_ResizeEnv(env, env->size * 2);

	scheme_define * defs = env->defs;

	if (env->count == 0) {
		defs[0] = def;
		++env->count;
		return;
	}

	scheme_define * l = defs,
	              * r = defs + env->count;
	while (1) {
		if (r == l) {
			// move every entry >= l up 1 spot to make
			// room for new definition
			scheme_define * move, * end = defs + env->count;
			for (move = end-1; move >= l; --move) {
				move[1] = move[0];
			}

			*l = def;
			goto done;
		}

		scheme_define * m = l + (r-l)/2;

		if (m->sym->str == def.sym->str) {
			DereferenceSymbol(&m->sym);
			Scheme_OverwriteDefine(m, def.object);
			m->sym = def.sym;
			return;
		} else if (m->sym->str < def.sym->str) {
			l = m + 1;
		} else {
			r = m;
		}
	}

done:
	++env->count;
}

scheme_define * Scheme_GetEnv(scheme_env * env, symbol * sym) {
	if (!env) return NULL;

	scheme_define * defs = env->defs;
	scheme_define * l = defs,
	              * r = defs + env->count;
	while (1) {
		if (r == l) {
			if (env->parent) {
				scheme_env * deeper_env = Scheme_GetEnvObj(env->parent);
				return Scheme_GetEnv(deeper_env, sym);
			} else {
				return NULL;
			}
		}

		scheme_define * m = l + (r-l)/2;

		if (m->sym->str == sym->str) {
			return m;
		} else if (m->sym->str < sym->str) {
			l = m + 1;
		} else {
			r = m;
		}
	}
}

#include <stdio.h>
void Scheme_DisplayEnv(scheme_env * env) {
	int i;
	printf("{ ");
	for (i = 0; i < env->count; ++i) {
		scheme_define * def = env->defs + i;
		printf("%s: ", def->sym->str);
		Scheme_Display(def->object);
		if (i != env->count-1) putchar(',');
		putchar(' ');
	}
	printf("}\n");
}
