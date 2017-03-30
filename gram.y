/*
 *
 * yacc/bison input for simplified C++ parser
 *
 */

%{

#include "defs.h"
#include "types.h"
#include "symtab.h"
#include "bucket.h"
#include "message.h"
#include "encode.h"
#include "tree.h"
#include "backend-x86.h"

    int yylex();
    int yyerror(char *s);

%}

%union {
	int	y_int;
	double	y_double;
	char *	y_string;
	BUCKET_PTR y_bucket;
	ST_ID y_id; //ST_ID is in defs.h
	TNODE y_tnode;
	TYPE y_type;
	TYPE_SPECIFIER y_type_spec;
	PARAM_LIST y_param_list;
	BOOLEAN y_bool;
	};

%type <y_id> identifier
%type <y_tnode> declarator direct_declarator
%type <y_bool> pointer

%token <y_string> IDENTIFIER STRING_LITERAL
%token <y_int> INT_CONSTANT
%token <y_double> DOUBLE_CONSTANT
%token SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token TYPEDEF EXTERN STATIC AUTO REGISTER
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token STRUCT UNION ENUM ELIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token BAD

%start translation_unit
%%

 /*******************************
  * Expressions                 *
  *******************************/

primary_expr
	: identifier
	| INT_CONSTANT		{$<y_int>$ = $1;}
	| DOUBLE_CONSTANT	{$<y_double>$ = $1;}
	| STRING_LITERAL
	| '(' expr ')'
	;

postfix_expr
	: primary_expr
	| postfix_expr '[' expr ']'
	| postfix_expr '(' argument_expr_list_opt ')'
	| postfix_expr '.' identifier
	| postfix_expr PTR_OP identifier
	| postfix_expr INC_OP
	| postfix_expr DEC_OP
	;

argument_expr_list_opt
	: /* null derive */
	| argument_expr_list
	;

argument_expr_list
	: assignment_expr
	| argument_expr_list ',' assignment_expr
	;

unary_expr
	: postfix_expr
	| INC_OP unary_expr
	| DEC_OP unary_expr
	| unary_operator cast_expr
	| SIZEOF unary_expr
	| SIZEOF '(' type_name ')'
	;

unary_operator
	: '&' | '*' | '+' | '-' | '~' | '!'
	;

cast_expr
	: unary_expr
	| '(' type_name ')' cast_expr
	;

multiplicative_expr
	: cast_expr
	| multiplicative_expr '*' cast_expr
	| multiplicative_expr '/' cast_expr
	| multiplicative_expr '%' cast_expr
	;

additive_expr
	: multiplicative_expr
	| additive_expr '+' multiplicative_expr
	| additive_expr '-' multiplicative_expr
	;

shift_expr
	: additive_expr
	| shift_expr LEFT_OP additive_expr
	| shift_expr RIGHT_OP additive_expr
	;

relational_expr
	: shift_expr
	| relational_expr '<' shift_expr
	| relational_expr '>' shift_expr
	| relational_expr LE_OP shift_expr
	| relational_expr GE_OP shift_expr
	;

equality_expr
	: relational_expr
	| equality_expr EQ_OP relational_expr
	| equality_expr NE_OP relational_expr
	;

and_expr
	: equality_expr
	| and_expr '&' equality_expr
	;

exclusive_or_expr
	: and_expr
	| exclusive_or_expr '^' and_expr
	;

inclusive_or_expr
	: exclusive_or_expr
	| inclusive_or_expr '|' exclusive_or_expr
	;

logical_and_expr
	: inclusive_or_expr
	| logical_and_expr AND_OP inclusive_or_expr
	;

logical_or_expr
	: logical_and_expr
	| logical_or_expr OR_OP logical_and_expr
	;

conditional_expr
	: logical_or_expr
	| logical_or_expr '?' expr ':' conditional_expr
	;

assignment_expr
	: conditional_expr
	| unary_expr assignment_operator assignment_expr
	;

assignment_operator
	: '=' | MUL_ASSIGN | DIV_ASSIGN | MOD_ASSIGN | ADD_ASSIGN | SUB_ASSIGN
	| LEFT_ASSIGN | RIGHT_ASSIGN | AND_ASSIGN | XOR_ASSIGN | OR_ASSIGN
	;

expr
	: assignment_expr
	| expr ',' assignment_expr
	;

constant_expr
	: conditional_expr { $<y_int>$ = $<y_int>1; }
	;

expr_opt
	: /* null derive */
	| expr
	;

 /*******************************
  * Declarations                *
  *******************************/

declaration
	: declaration_specifiers ';' {error("No declarator listed in declaration.");}
	| declaration_specifiers init_declarator_list ';'
	;

declaration_specifiers
	: storage_class_specifier
	| storage_class_specifier declaration_specifiers
	| type_specifier {$<y_bucket>$ = update_bucket(NULL, $<y_type_spec>1, NULL);}
	| type_specifier declaration_specifiers { $<y_bucket>$ = update_bucket($<y_bucket>2, $<y_type_spec>1, NULL);}
	| type_qualifier
	| type_qualifier declaration_specifiers
	;

init_declarator_list
	: init_declarator
	| init_declarator_list ',' { $<y_type>$ = $<y_type>0; } init_declarator
	;

init_declarator
	: declarator	{/*Alloc STDR, try to install, backend*/
						TYPE decl_specs = build_base($<y_bucket>0);
						ST_ID id = get_id($1); //new method in tree
						char * id_str = st_get_id_str(id);
						TYPE t = get_type(decl_specs, $1);
						TYPETAG tag = ty_query(t);
						ST_DR rec = stdr_alloc();
						rec->tag = GDECL;
						rec->u.decl.type = t;
						rec->u.decl.sc=NO_SC;
						if (!st_install(id, rec)) {
							error("Duplicate declaration for %s.", id_str);
							stdr_free(rec);
				  		}
				  		else if(tag == TYFUNC) {
				  			//find size and alignment
				  			unsigned int size = get_size(t);
			  				unsigned int alignment = get_alignment(t);
				  		}
				  		else {
				  			//find size and alignment
				  			unsigned int size = get_size(t);
				  			unsigned int alignment = get_alignment(t);
							b_global_decl(id_str, alignment, size);
							b_skip(size);
				  		}
				  		$<y_type>$ = t;
					  	}	
	| declarator '=' initializer
	;

storage_class_specifier
	: TYPEDEF | EXTERN | STATIC | AUTO | REGISTER
	;

type_specifier
	: VOID 		{/*for 100%*/ $<y_type_spec>$ = VOID_SPEC; }
	| CHAR 		{/*for 80%*/ $<y_type_spec>$ = CHAR_SPEC; }
	| SHORT 	{/*for 90%*/ $<y_type_spec>$ = SHORT_SPEC;}
	| INT 		{/*for 80%*/ $<y_type_spec>$ = INT_SPEC; }
	| LONG		{/*for 90%*/ $<y_type_spec>$ = LONG_SPEC;}
	
	| FLOAT 	{/*for 80%*/ $<y_type_spec>$ = FLOAT_SPEC;}
	| DOUBLE 	{/*for 80%*/ $<y_type_spec>$ = DOUBLE_SPEC;}
	| SIGNED 	{/*for 90%*/ $<y_type_spec>$ = SIGNED_SPEC;}
	| UNSIGNED	{/*for 90%*/ $<y_type_spec>$ = UNSIGNED_SPEC;}
	
	| struct_or_union_specifier
	| enum_specifier
	| TYPE_NAME
	;

struct_or_union_specifier
	: struct_or_union '{' struct_declaration_list '}'
	| struct_or_union identifier '{' struct_declaration_list '}'
	| struct_or_union identifier
	;

struct_or_union
	: STRUCT
	| UNION
	;

struct_declaration_list
	: struct_declaration
	| struct_declaration_list struct_declaration
	;

struct_declaration
	: specifier_qualifier_list struct_declarator_list ';'
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list_opt
	| type_qualifier specifier_qualifier_list_opt
	;

specifier_qualifier_list_opt
	: /* null derive */
	| specifier_qualifier_list
        ;

struct_declarator_list
	: struct_declarator
	| struct_declarator_list ',' struct_declarator
	;

struct_declarator
	: declarator
	| ':' constant_expr
	| declarator ':' constant_expr
	;

enum_specifier
	: ENUM '{' enumerator_list '}'
	| ENUM identifier '{' enumerator_list '}'
	| ENUM identifier
	;

enumerator_list
	: enumerator
	| enumerator_list ',' enumerator
	;

enumerator
	: identifier
	| identifier '=' constant_expr
	;

type_qualifier
	: CONST | VOLATILE
	;

declarator
	: direct_declarator		{$$ = $1;}
	| pointer declarator	{if($1){
								$$ = new_node($2, REFTN);
							}
							else{
								$$ = new_node($2, PNTRTN);
							}
							}
	;

direct_declarator
	: identifier		{TNODE n = new_node(NULL, IDTN);
						n->u.id = $1;
						$$ = n;}
	| '(' declarator ')'	{$$ = $2;}
	| direct_declarator '[' ']'	{warning("No array dimensions included.\n");}
	| direct_declarator '[' constant_expr ']'	{
												TNODE n = new_node($1, ARRAYTN);
												n->u.arr_size = $<y_int>3;
												$$ = n;}
	| direct_declarator '(' parameter_type_list ')' {
													TNODE n = new_node($1, FUNCTN);
													n->u.plist = $<y_param_list>3;
													$$ = n;
													}
	| direct_declarator '(' ')'	{
								TNODE n = new_node($1, FUNCTN);
								n->u.plist = NULL;
								$$ = n;}
	;

pointer
	: '*' specifier_qualifier_list_opt {$$ = FALSE;}
        | '&'	{$$ = TRUE;}
	;

parameter_type_list
	: parameter_list		{$<y_param_list>$ = $<y_param_list>1;}
	| parameter_list ',' ELIPSIS
	;

parameter_list
	: parameter_declaration 	{ $<y_param_list>$ = $<y_param_list>1;}
	| parameter_list ',' parameter_declaration {
			if($<y_param_list>1->next == NULL)
			{
				//error("next is null");
				$<y_param_list>3->prev = $<y_param_list>1;
				$<y_param_list>1->next = $<y_param_list>3;
			}
			else
			{
				//error("next is not null");
				PARAM_LIST p = $<y_param_list>1->next;
				while(p->next != NULL)
				{
					//error("while != NULL");
					p = p->next;
					//error("next");
				}
				$<y_param_list>3->prev = p;
				//error("assigned prev");
				p->next = $<y_param_list>3;
				//error("assigned next");
			}
			$<y_param_list>$ = $<y_param_list>1;}
	;

parameter_declaration
	: declaration_specifiers declarator {//error("Making type"); 
			PARAM_LIST p;
			p = (PARAM_LIST)malloc(sizeof(PARAM));
			//error("Getting id");
			p->id = get_id($2);
			//error("Getting Type");
			p->type = get_type(build_base($<y_bucket>1),$2);
			//error("Got type");
			p->sc = NO_SC;
			p->err = FALSE;
			p->is_ref = is_reference($2);
			//error("Filled parameter");
			//p->next = NULL;
			$<y_param_list>$ = p;
			}
	| declaration_specifiers {error("No id in parameter list");}
	| declaration_specifiers abstract_declarator {}
	;

type_name
	: specifier_qualifier_list
	| specifier_qualifier_list abstract_declarator
	;

abstract_declarator
	: pointer
	| direct_abstract_declarator
	| pointer abstract_declarator
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'
	| '[' ']'
	| '[' constant_expr ']'
	| direct_abstract_declarator '[' ']'
	| direct_abstract_declarator '[' constant_expr ']'
	| '(' ')'
	| '(' parameter_type_list ')'
	| direct_abstract_declarator '(' ')'
	| direct_abstract_declarator '(' parameter_type_list ')'
	;

initializer
	: assignment_expr
	| '{' initializer_list comma_opt '}'
	;

comma_opt
	: /* Null derive */
	| ','
	;

initializer_list
	: initializer
	| initializer_list ',' initializer
	;

 /*******************************
  * Statements                  *
  *******************************/

statement
	: labeled_statement
	| compound_statement
	| expression_statement
	| selection_statement
	| iteration_statement
	| jump_statement
	;

labeled_statement
	: identifier ':' statement
	| CASE constant_expr ':' statement
	| DEFAULT ':' statement
	;

compound_statement
	: '{' '}'
	| '{' statement_list '}'
	| '{' declaration_list '}'
	| '{' declaration_list statement_list '}'
	;

declaration_list
	: declaration
	| declaration_list declaration
	;

statement_list
	: statement
	| statement_list statement
	;

expression_statement
	: expr_opt ';'
	;

selection_statement
	: IF '(' expr ')' statement
	| IF '(' expr ')' statement ELSE statement
	| SWITCH '(' expr ')' statement
	;

iteration_statement
	: WHILE '(' expr ')' statement
	| DO statement WHILE '(' expr ')' ';'
	| FOR '(' expr_opt ';' expr_opt ';' expr_opt ')' statement
	;

jump_statement
	: GOTO identifier ';'
	| CONTINUE ';'
	| BREAK ';'
	| RETURN expr_opt ';'
	;

 /*******************************
  * Top level                   *
  *******************************/

translation_unit
	: external_declaration
	| translation_unit external_declaration
	;

external_declaration
	: function_definition
	| declaration
	;

function_definition
	: declarator compound_statement
	| declaration_specifiers declarator compound_statement
	;

 /*******************************
  * Identifiers                 *
  *******************************/

identifier
	: IDENTIFIER	{/*enroll the ID into the sym table*/
					$$ = st_enter_id($1);}
	;
%%

extern int column;

int yyerror(char *s)
{
	error("%s (column %d)",s,column);
        return 0;  /* never reached */
}
