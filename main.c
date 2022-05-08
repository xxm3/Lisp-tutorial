#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

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

int main(int argc, char **argv)
{
	mpc_parser_t *Number = mpc_new("number");
	mpc_parser_t *Operator = mpc_new("operator");
	mpc_parser_t *Expr = mpc_new("expr");
	mpc_parser_t *Lispy = mpc_new("lispy");

	mpca_lang(MPCA_LANG_DEFAULT,
  	"                                                     \
  	  number   : /-?[0-9]+/ ;                             \
  	  operator : '+' | '-' | '*' | '/' ;                  \
  	  expr     : <number> | '(' <operator> <expr>+ ')' ;  \
  	  lispy    : /^/ <operator> <expr>+ /$/ ;             \
  	",
  	Number, Operator, Expr, Lispy);

	puts("LISP version 0.0.0.0.1\n");
	puts("Press ctrl+c to exit\n");
	
	while (1)
	{
		char *input = readline("lispy> ");
		add_history(input);
		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lispy, &r))
		{
			mpc_ast_print(r.output);
			mpc_ast_delete(r.output);
		}
		else
		{
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
		free(input);

	}

	mpc_cleanup(4, Number, Operator, Expr, Lispy);
	return 0;
}
