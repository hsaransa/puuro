%{
#include "ast.hpp"
#include "lexer.hpp"
#include "list.hpp"
#include <stdio.h>
#include <stdexcept>

using namespace pr;

#define NEW_AST(t) new AST(Name(#t), FilePosition(yy_file, yy_lexer->get_line()))
#define NODE2(dst, type, arg1, arg2) { dst = NEW_AST(type); dst->add_child(arg1); dst->add_child(arg2); }
#define NODE1(dst, type, arg) { dst = NEW_AST(type); dst->add_child(arg); }
#define NODE(dst, type) { dst = NEW_AST(type); }
#define NODEO(dst, type, o) { dst = NEW_AST(type); dst->set_object(o); }

namespace pr
{
	Lexer* yy_lexer;
	int yy_file;
	AST* yy_ast;
}

static int yylex()
{
	return yy_lexer->next();
}

static void yyerror(const char*)
{
    throw new Exception("syntax_error",
        (ObjP)*new List(2, name_to_symbol(Name(yy_file)), int_to_fixnum(yy_lexer->get_line())));
    //fprintf(stderr, "syntax error in %s on line %d: %s\n", Name(yy_file).s(), yy_lexer->get_line(), err);
    //throw std::runtime_error("parse error");
}
%}

%union
{
	pr::AST* ast;
	pr::ObjP obj;
}

%term T_INTEGER T_IDENTIFIER T_STRING T_FLOAT
%term T_EQ T_NE T_LE T_GE T_TRUE T_FALSE T_NULL
%term T_EMPTY_PARENS T_TWO_DOTS

%left T_TWO_DOTS
%left T_AND2 T_OR2
%left T_EQ T_NE
%left T_LE T_GE '<' '>'
%left '&' '|'
%left '-' '+'
%left '*' '/' '%'
%left UMINUS

%type <ast> expr_list expr_list2 expr term_expr arg_list param param_list
%type <ast> call_expr  assign_target assign_list
%type <ast>  top_expr sink_param infix_expr

%type <obj> T_INTEGER T_IDENTIFIER T_STRING T_FLOAT

%%

start:
	expr_list
	{ yy_ast = $1; }
	;

expr_list:
	expr_list2
	{ $$ = $1; } |

	{ NODE($$, ExprList); }
	;

expr_list2:
	expr
	{ NODE1($$, ExprList, $1); } |

	expr_list2 ';' expr
	{ $$ = $1; $$->add_child($3); } |

	expr_list2 ';'
	{ $$ = $1; } |

	';' expr_list2
	{ $$ = $2; } |

	';'
	{ NODE($$, Nop); }
	;

expr:
	assign_list '=' call_expr
	{ NODE2($$, Assign, $1, $3); } |

	call_expr
	{ $$ = $1; }
	;

call_expr:
	top_expr
	{ $$ = $1; } |

	top_expr arg_list
	{ NODE2($$, Call, $1, $2); } |

	term_expr '.' T_IDENTIFIER arg_list
	{ NODEO($$, CallMethod, $3); $$->add_child($1); $$->add_child($4); }

	;

top_expr:
	infix_expr
	{ $$ = $1; }
	;

infix_expr:
	infix_expr T_EQ infix_expr
	{ NODE2($$, Eq, $1, $3); } |

	infix_expr T_NE infix_expr
	{ NODE2($$, NE, $1, $3); } |

	infix_expr '<' infix_expr
	{ NODE2($$, LT, $1, $3); } |

	infix_expr '>' infix_expr
	{ NODE2($$, GT, $1, $3); } |

	infix_expr T_LE infix_expr
	{ NODE2($$, LE, $1, $3); } |

	infix_expr T_GE infix_expr
	{ NODE2($$, GE, $1, $3); } |

	infix_expr '+' infix_expr
	{ NODE2($$, Add, $1, $3); } |

	infix_expr '-' infix_expr
	{ NODE2($$, Sub, $1, $3); } |

	infix_expr '*' infix_expr
	{ NODE2($$, Mul, $1, $3); } |

	infix_expr '/' infix_expr
	{ NODE2($$, Div, $1, $3); } |

	infix_expr '%' infix_expr
	{ NODE2($$, Mod, $1, $3); } |

	infix_expr '&' infix_expr
	{ NODE2($$, And, $1, $3); } |

	infix_expr T_AND2 infix_expr
	{ NODE2($$, And, $1, $3); } |

	infix_expr '|' infix_expr
	{ NODE2($$, Or, $1, $3); } |

	infix_expr T_OR2 infix_expr
	{ NODE2($$, Or, $1, $3); } |

	//term_expr '[' expr ']'
	//{ NODE2($$, GetItem, $1, $3); } |

	'-' infix_expr
	{ NODE1($$, Neg, $2); } |

	'!' infix_expr
	{ NODE1($$, Not, $2); } |

	infix_expr T_TWO_DOTS T_IDENTIFIER
	{ NODEO($$, CallMethod, $3); $$->add_child($1); $$->add_child(NEW_AST(ArgList)); } |

	term_expr
	{ $$ = $1; }
	;

term_expr:
	'(' expr ')'
	{ $$ = $2; } |

	T_INTEGER
	{ NODEO($$, Integer, $1); } |

	T_IDENTIFIER
	{ NODEO($$, Variable, $1); } |

	T_STRING
	{ NODEO($$, String, $1); } |

	T_FLOAT
	{ NODEO($$, Float, $1); } |

	T_TRUE
	{ NODE($$, True); } |

	T_FALSE
	{ NODE($$, False); } |

	T_NULL
	{ NODE($$, Null); } |

	'\'' T_IDENTIFIER
	{ NODEO($$, Symbol, $2); } |

	term_expr ':' T_IDENTIFIER
	{ NODE1($$, Method, $1); $$->set_object($3); } |

	term_expr '.' T_IDENTIFIER
	{ NODEO($$, CallMethod, $3); $$->add_child($1); $$->add_child(NEW_AST(ArgList)); } |

	'[' ']'
	{ NODE1($$, List, NEW_AST(ArgList)); } |

	'[' arg_list ']'
	{ NODE1($$, List, $2); } |

	'{' '<' param_list '>' expr_list '}'
	{ NODE2($$, Code, $3, $5); } |

	'{' sink_param expr_list '}'
	{ AST* ast;
	  NODE1(ast, ParamList, $2);
	  NODE2($$, Code, ast, $3); } |

	'{' '<' '>' expr_list '}'
	{ NODE2($$, Code, NEW_AST(ParamList), $4); } |

	'{' expr_list '}'
	{ NODE2($$, Code, NEW_AST(ParamList), $2); } |

	term_expr T_EMPTY_PARENS
	{ NODE2($$, Call, $1, NEW_AST(ArgList)); }
	;

assign_target:
	T_IDENTIFIER
	{ NODEO($$, AssignVariable, $1); } |

	'*' T_IDENTIFIER
	{ NODEO($$, AssignSinkVariable, $2); } |

	term_expr '.' T_IDENTIFIER
	{ NODEO($$, AssignMember, $3); $$->add_child($1); } |

	'*' term_expr '.' T_IDENTIFIER
	{ NODEO($$, AssignSinkMember, $4); $$->add_child($2); }
	;

assign_list:
	assign_target
	{ NODE1($$, AssignList, $1); } |

	assign_list ',' assign_target
	{ $$ = $1; $$->add_child($3); }
	;

arg_list:
	top_expr
	{ NODE1($$, ArgList, $1); } |

	arg_list top_expr
	{ $$ = $1; $$->add_child($2); }
	;

/*
comma_arg_list:
	expr
	{ NODE1($$, ArgList, $1); } |

	comma_arg_list ',' expr
	{ $$ = $1; $$->add_child($3); }
	;
*/

sink_param:
	'*' T_IDENTIFIER
	{ NODEO($$, ParamSink, $2); }
	;

param:
	T_IDENTIFIER
	{ NODEO($$, Param, $1); } |

	sink_param
	{ $$ = $1; }
	;

param_list:
	param
	{ NODE1($$, ParamList, $1); } |

	param_list param
	{ $$ = $1; $$->add_child($2); }
	;

%%
