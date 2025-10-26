#ifndef S2HTML_EVENT_H
#define S2HTML_EVENT_H

/* Token property definitions */
#define USER_DEFINED_HEADER		1
#define SYSTEM_HEADER_FILE		2
#define DATATYPE_KEYWORD		3
#define CONTROL_KEYWORD			4

#define MAX_TOKEN_LENGTH		1024

typedef enum
{
	TOKEN_UNKNOWN,
	TOKEN_PREPROCESSOR_DIRECTIVE,
	TOKEN_RESERVE_KEYWORD,
	TOKEN_NUMERIC_CONSTANT,
	TOKEN_STRING_LITERAL,
	TOKEN_HEADER_FILE,
	TOKEN_REGULAR_TEXT,			/* for: Variable names, Function names, User-defined type names, Spaces, tabs, newlines, Any character that's not an operator, symbol, or special token */
	TOKEN_SINGLE_LINE_COMMENT,
	TOKEN_MULTI_LINE_COMMENT,
	TOKEN_ASCII_CHAR,
	TOKEN_FORMAT_SPECIFIER,
	TOKEN_OPERATORS,
	TOKEN_SYMBOL,
	TOKEN_END_OF_FILE
}token_type_t;

typedef struct
{
	token_type_t type;				// token classification
	int attribute;					// additional token properties
	int size;						// length of token data
	char content[MAX_TOKEN_LENGTH];	// actual token string
}parser_token_t;

/********** function prototypes **********/

parser_token_t *extract_next_token(FILE *input_stream);

/********** state handlers **********/
parser_token_t * handle_idle_state(FILE *input_stream, int current_char);
parser_token_t * handle_single_comment_state(FILE *input_stream, int current_char);
parser_token_t * handle_multi_comment_state(FILE *input_stream, int current_char);
parser_token_t * handle_number_state(FILE *input_stream, int current_char);
parser_token_t * handle_string_state(FILE *input_stream, int current_char);
parser_token_t * handle_header_state(FILE *input_stream, int current_char);
parser_token_t * handle_ascii_state(FILE *input_stream, int current_char);
parser_token_t * handle_keyword_state(FILE *input_stream, int current_char);
parser_token_t * handle_preprocessor_state(FILE *input_stream, int current_char);
parser_token_t * handle_preprocessor_main_state(FILE *input_stream, int current_char);
parser_token_t * handle_format_specifier_state(FILE *input_stream, int current_char);
parser_token_t * handle_operator_state(FILE *input_stream, int current_char);
parser_token_t * handle_symbol_state(FILE *input_stream, int current_char);

/********** Utility functions **********/

#endif
/**** End of file ****/