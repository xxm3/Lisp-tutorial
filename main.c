#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

typedef struct lval{
	int type;
	long num;
	char* err;
	char* sym;
	int count;
	struct lval** cell;
} lval;

enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR };
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

lval* lval_num(int x)
{
	lval* res = (lval*)malloc(sizeof(lval));
	res->num = x;
	res->type = LVAL_NUM;
	return res;
}

lval* lval_err(char* m) 
{
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_ERR;
	v->err = malloc(strlen(m) + 1);
	strcpy(v->err, m);
	return v;
}

lval* lval_sym(char* s)
{
  	lval* v = (lval*)malloc(sizeof(lval));
  	v->type = LVAL_SYM;
  	v->sym = (char*)malloc(strlen(s) + 1);
  	strcpy(v->sym, s);
  	return v;
}

lval* lval_sexpr(void) 
{
  	lval* v = (lval*)malloc(sizeof(lval));
  	v->type = LVAL_SEXPR;
  	v->count = 0;
  	v->cell = NULL;
  	return v;
}

void lval_del(lval* v)
{
	switch (v->type)
	{
	case LVAL_NUM:
		break;
	
	case LVAL_ERR:
		free(v->err);
		break;

	case LVAL_SYM:
		free(v->sym);
		break;

	case LVAL_SEXPR:
		for (int i = 0; i < v->count; i++)
		{
			lval_del(v->cell[i]);
		}
		free(v->cell);
		break;
	}
	free(v);
}

lval* lval_read_num(mpc_ast_t* t) 
{
  	errno = 0;
  	long x = strtol(t->contents, NULL, 10);
  	return errno != ERANGE ?
  		lval_num(x) : lval_err("invalid number");
}

lval* lval_add(lval* v, lval* x) 
{
 	v->count++;
 	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
 	v->cell[v->count-1] = x;
 	return v;
}

void lval_print(lval* v);
void lval_expr_print(lval* v, char open, char close) 
{
  	putchar(open);
  	for (int i = 0; i < v->count; i++) {
	  
  	  /* Print Value contained within */
  	  lval_print(v->cell[i]);
	
  	  /* Don't print trailing space if last element */
  	  if (i != (v->count-1)) {
  	    putchar(' ');
  	  }
  	}
  	putchar(close);
}

void lval_print(lval* v) 
{
  	switch (v->type) 
  	{
  	  	case LVAL_NUM:   printf("%li", v->num); break;
  	  	case LVAL_ERR:   printf("Error: %s", v->err); break;
  	  	case LVAL_SYM:   printf("%s", v->sym); break;
  	  	case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
  	}
}

lval* lval_read(mpc_ast_t* t) 
{
  	/* If Symbol or Number return conversion to that type */
  	if (strstr(t->tag, "number")) { return lval_read_num(t); }
  	if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

  	/* If root (>) or sexpr then create empty list */
  	lval* x = NULL;
  	if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
  	if (strstr(t->tag, "sexpr"))  { x = lval_sexpr(); }

  	/* Fill this list with any valid expression contained within */
  	for (int i = 0; i < t->children_num; i++) {
  	  	if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
  	  	if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
  	  	if (strcmp(t->children[i]->tag,  "regex") == 0) { continue; }
  	  	x = lval_add(x, lval_read(t->children[i]));
  	}

  	return x;
}

void lvlal_expr_print(lval* v, char open, char close)
{
	putchar(open);
	for (int i = 0; i < v->count; i++)
	{
		lval_print(v->cell[i]);

		// Don't print trailing space if last element
		if (i != v->count-1)
		{
			putchar(' ');
		}
	}
	putchar(close);
}

#ifdef _WIN32
	#include <string.h>
	static char buffer[2048];

	char *readline(char *input)
	{
		fputs(input, stdout);
		fgets(buffer, sizeof(buffer), stdin);
		char *cpy = (char*)malloc(strlen(buffer)+1);
		strcpy(cpy, buffer);
		cpy[strlen(cpy)-1] = '\0';
		return cpy;
	}
	void add_history(char *in) {}
#else
	#include <readline/readline.h>
	#include <readline/history.h>
#endif

// lval eval_op(lval x, char* op, lval y) 
// {
// 	if (x.type == LVAL_ERR) { return x; }
//   	if (y.type == LVAL_ERR) { return y; }
  	
// 	if (strcmp(op, "+") == 0) { return lval_num(x.num + y.num); }
//   	if (strcmp(op, "-") == 0) { return lval_num(x.num - y.num); }
//   	if (strcmp(op, "*") == 0) { return lval_num(x.num * y.num); }
//   	if (strcmp(op, "/") == 0) 
// 	{
//     	return y.num == 0
//     	  ? lval_err(LERR_DIV_ZERO)
//     	  : lval_num(x.num / y.num);
//   	}

//   	return lval_err(LERR_BAD_OP);
// }

// lval eval(mpc_ast_t *p)
// {
	// if (strstr(p->tag, "number"))
	// {	
		// errno = 0;
		// long x = strtol(p->contents, NULL, 10);
		// return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
	// }
// 
	// char *op = p->children[1]->contents;
	// lval x = eval(p->children[2]);
	// 
	// int i = 3;
	// while (strstr(p->children[i]->tag, "expr"))
	// {
		// x = eval_op(x, op, eval(p->children[i]));
		// i++;
	// }
	// return x;
// }



void lval_println(lval* v) { lval_print(v); putchar('\n'); }

int main(int argc, char **argv)
{
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Symbol = mpc_new("symbol");
	mpc_parser_t* Sexpr  = mpc_new("sexpr");
	mpc_parser_t* Expr   = mpc_new("expr");
	mpc_parser_t* Lispy  = mpc_new("lispy");

	mpca_lang(MPCA_LANG_DEFAULT,
	  "                                          \
	    number : /-?[0-9]+/ ;                    \
	    symbol : '+' | '-' | '*' | '/' ;         \
	    sexpr  : '(' <expr>* ')' ;               \
	    expr   : <number> | <symbol> | <sexpr> ; \
	    lispy  : /^/ <expr>* /$/ ;               \
	  ",
	  Number, Symbol, Sexpr, Expr, Lispy);

	puts("LISP version 0.0.0.0.1\n");
	puts("Press ctrl+c to exit\n");
	
	while (1)
	{
		char *input = readline("lispy> ");
		add_history(input);
		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lispy, &r))
		{
			lval *x = lval_read(r.output);
			lval_println(x);
			lval_del(x);
			// lval result = eval(r.output);
			// lval_println(result);
			// mpc_ast_delete(r.output);
		}
		else
		{
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
		free(input);
	}

	mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);
	return 0;
}
