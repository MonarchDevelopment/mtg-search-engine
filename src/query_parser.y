%{
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "testing_h/testing.h"
#undef lprintf
#define lprintf fprintf(LOG_STREAM, "(" ANSI_YELLOW "%s" ANSI_RESET \
                        ":" ANSI_YELLOW "%d" ANSI_RESET ") \t", __FILE__, __LINE__ ),\
                        __lprintf
#include "src/interpretor.h"
#include "src/query_parser.h"
#include "mse_query_lexer.h"
#include "mse_query_parser.h"

static pthread_mutex_t parser_lock = PTHREAD_MUTEX_INITIALIZER;

static void yyerror(mse_parser_status_t *__ret, const char *s)
{
    lprintf(LOG_ERROR, "Parse error: %s\n", s);
}
%}
%code requires {
    #include "src/query_parser.h"
}
%parse-param {mse_parser_status_t *ret}

%token LT
%token LT_INC
%token GT
%token GT_INC
%token INCLUDES
%token EQUALS

%token WORD
%token STRING

%token REGEX_STRING

%token AND
%token OR

%token WHITESPACE

%token OPEN_BRACKET
%token CLOSE_BRACKET

%token STMT_NEGATE
%{
#define COPY_TO_TMP_BUFFER \
    ret->tmp_buffer = (char*) malloc(sizeof(char) * (yyleng + 1)); \
    ASSERT(ret->tmp_buffer != NULL); \
    strncpy(ret->tmp_buffer, yytext, yyleng); \
    ret->tmp_buffer[yyleng] = '\0'; \

static int __mse_handle_set_generator(mse_parser_status_t *ret)
{
    mse_set_generator_t tmp;
    ASSERT(mse_init_set_generator(&tmp,
                                  ret->parser_gen_type,
                                  ret->parser_op_type,
                                  ret->argument_buffer,
                                  strlen(ret->argument_buffer)));
    ASSERT(mse_init_interp_node_generator(tmp));
    return 1;
}

/// Calls the handler for a set generator then cleans the internal state
static int mse_handle_set_generator(int negate, mse_parser_status_t *ret)
{
    
    ASSERT(ret->argument_buffer = strdup(ret->tmp_buffer));
    free(ret->tmp_buffer);
    ret->tmp_buffer = NULL;

    int r = __mse_handle_set_generator(ret);

    free(ret->op_name_buffer);
    ret->op_name_buffer = NULL;

    free(ret->argument_buffer);
    ret->argument_buffer = NULL;
    return r;
}

static int __mse_insert_node(mse_parser_status_t *state, mse_interp_node_t *node)
{
    ASSERT(node != NULL);
    if (state->root == NULL) {
        state->root = state->node = node;
    } else {
        if (state->node->l == NULL) {
             state->node->l = node;
        } else {
            state->node->r = node;
            state->node = node;
        }
    }
    return 1;
}

%}

// Token match definitions
%%
input: query
     ;

op_operator : LT_INC { ret->parser_op_type = MSE_SET_GENERATOR_OP_LT_INC; }
            | LT { ret->parser_op_type = MSE_SET_GENERATOR_OP_LT; }
            | GT { ret->parser_op_type = MSE_SET_GENERATOR_OP_GT; }
            | GT_INC { ret->parser_op_type = MSE_SET_GENERATOR_OP_GT_INC; }
            | INCLUDES { ret->parser_op_type = MSE_SET_GENERATOR_OP_INCLUDES; }
            | EQUALS { ret->parser_op_type = MSE_SET_GENERATOR_OP_EQUALS; }
            ;

word: WORD {
    COPY_TO_TMP_BUFFER
    }

op_name: word {
       ASSERT(ret->op_name_buffer = strdup(ret->tmp_buffer));
       free(ret->tmp_buffer);
       ret->tmp_buffer = NULL;
       }

string: STRING {
      COPY_TO_TMP_BUFFER
      }

regex_string: REGEX_STRING {
            COPY_TO_TMP_BUFFER
            }

op_argument: string { COPY_TO_TMP_BUFFER }
           | regex_string { COPY_TO_TMP_BUFFER }
           | word { COPY_TO_TMP_BUFFER }
           ;

set_generator: op_name op_operator op_argument { ASSERT(mse_handle_set_generator(0, ret)); }
             | STMT_NEGATE op_name op_operator op_argument { ASSERT(mse_handle_set_generator(1, ret)); }
             | word { ASSERT(mse_handle_set_generator(0, ret)); }
             | string { ASSERT(mse_handle_set_generator(0, ret)); }
             ;

operator : AND { ASSERT(ret->op_node = mse_init_interp_node_operation(MSE_SET_INTERSECTION)); }
         | OR { ASSERT(ret->op_node = mse_init_interp_node_operation(MSE_SET_UNION)); }
         ;

query: %empty
     | set_generator WHITESPACE operator WHITESPACE query {
     ASSERT(__mse_insert_node(ret, ret->op_node));
     ASSERT(__mse_insert_node(ret, ret->set_generator_node));
     ret->op_node = NULL;
     ret->set_generator_node = NULL;
     }
     | set_generator WHITESPACE query { 
     // Create a AND node and insert it
     ASSERT(ret->op_node = mse_init_interp_node_operation(MSE_SET_INTERSECTION));
     ASSERT(__mse_insert_node(ret, ret->op_node));
     ASSERT(__mse_insert_node(ret, ret->set_generator_node));
     ret->op_node = NULL;
     ret->set_generator_node = NULL;
     }
     | set_generator { 
     ASSERT(__mse_insert_node(ret, ret->set_generator_node));
     ret->set_generator_node = NULL;
     }
     | OPEN_BRACKET query CLOSE_BRACKET WHITESPACE query {
     // Create a AND node and insert it
     ASSERT(ret->op_node = mse_init_interp_node_operation(MSE_SET_INTERSECTION));
     ASSERT(__mse_insert_node(ret, ret->op_node));
     ret->op_node = NULL;
     }
     | OPEN_BRACKET query CLOSE_BRACKET WHITESPACE operator WHITESPACE query {
     ASSERT(__mse_insert_node(ret, ret->op_node));
     ret->op_node = NULL;
     }
     | OPEN_BRACKET query CLOSE_BRACKET {
     ASSERT(__mse_insert_node(ret, ret->set_generator_node));
     ret->set_generator_node = NULL;
     }
     | STMT_NEGATE OPEN_BRACKET query CLOSE_BRACKET WHITESPACE operator WHITESPACE query
     | STMT_NEGATE OPEN_BRACKET query CLOSE_BRACKET WHITESPACE query
     | STMT_NEGATE OPEN_BRACKET query CLOSE_BRACKET
     ;
%%

int parse_input_string(const char* input_string, mse_interp_node_t **root)
{
    *root = NULL;

    mse_parser_status_t ret;
    memset(&ret, 0, sizeof(ret));

    pthread_mutex_lock(&parser_lock);

    YY_BUFFER_STATE input_buffer = yy_scan_string(input_string);
    int result = yyparse(&ret);
    yy_delete_buffer(input_buffer);

    pthread_mutex_unlock(&parser_lock);

    // Cleanup
    if (ret.tmp_buffer != NULL) {
        free(ret.tmp_buffer);
    }
    if (ret.argument_buffer != NULL) {
        free(ret.argument_buffer);
    }
    if (ret.op_name_buffer != NULL) {
        free(ret.op_name_buffer);
    }

    *root = ret.root;
    ASSERT(root != NULL);
    return result == 0;
}
