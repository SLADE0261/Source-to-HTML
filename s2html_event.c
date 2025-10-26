#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "s2html_event.h"

#define SYMBOL_COUNT (sizeof(delimiter_chars))
#define OPERATOR_COUNT (sizeof(operator_chars))
#define TEMP_BUFFER_SIZE 100

/********** Parser state definitions **********/
typedef enum
{
	PARSE_STATE_IDLE,
	PARSE_STATE_PREPROCESSOR_DIRECTIVE,
	PARSE_STATE_SUB_PREPROCESSOR_MAIN,
	PARSE_STATE_SUB_PREPROCESSOR_RESERVE_KEYWORD,
	PARSE_STATE_SUB_PREPROCESSOR_ASCII_CHAR,
	PARSE_STATE_HEADER_FILE,
	PARSE_STATE_RESERVE_KEYWORD,
	PARSE_STATE_NUMERIC_CONSTANT,
	PARSE_STATE_STRING,
	PARSE_STATE_SINGLE_LINE_COMMENT,
	PARSE_STATE_MULTI_LINE_COMMENT,
	PARSE_STATE_FORMAT_SPECIFIER,
	PARSE_STATE_OPERATORS,
	PARSE_STATE_SYMBOLS,
	PARSE_STATE_ASCII_CHAR
} parser_state_t;

/********** Global state variables **********/

/* Current parser state */
static parser_state_t current_state = PARSE_STATE_IDLE;

/* Sub-state for preprocessor parsing */
static parser_state_t preprocessor_substate = PARSE_STATE_SUB_PREPROCESSOR_MAIN;

/* Token data storage */
static parser_token_t current_token;
static int token_buffer_index = 0;

static char temporary_word[TEMP_BUFFER_SIZE];
static int word_buffer_index = 0;

/* C language keyword tables */
static char *datatype_keywords[] = {"const", "volatile", "extern", "auto", "register",
									"static", "signed", "unsigned", "short", "long",
									"double", "char", "int", "float", "struct",
									"union", "enum", "void", "typedef", ""};

static char *control_keywords[] = {"goto", "return", "continue", "break",
								   "if", "else", "for", "while", "do",
								   "switch", "case", "default", "sizeof", ""};

static char operator_chars[] = {'/', '+', '*', '-', '%', '=', '<', '>', '~', '&', ',', '!', '^', '|'};
static char delimiter_chars[] = {'(', ')', '{', '}', '[', ']', ':'};

/* ********************************************************************************************************************** */
/* ********************************************************************************************************************** */
/* ********************************************************************************************************************** */
/* ********************************************************************************************************************** */

/********** Helper function implementations **********/

/* Determines if a word matches C language keywords */
static int classify_keyword(char *word_to_check)
{
	int array_index = 0;

	/* Check datatype keywords first */
	while (*datatype_keywords[array_index])
	{
		if (strcmp(datatype_keywords[array_index++], word_to_check) == 0)
		{
			return DATATYPE_KEYWORD;
		}
	}

	array_index = 0; /* Reset for next search */
	/* Check control flow keywords */
	while (*control_keywords[array_index])
	{
		if (strcmp(control_keywords[array_index++], word_to_check) == 0)
		{
			return CONTROL_KEYWORD;
		}
	}
	return TOKEN_REGULAR_TEXT; /* Not a keyword, treat as regular text */
}

/* Checks if character is a delimiter/symbol */
static int is_delimiter_char(char character)
{
	int i;
	for (i = 0; i < SYMBOL_COUNT; i++)
	{
		if (delimiter_chars[i] == character)
			return 1;
	}
	return 0;
}

/* Checks if character is an operator */
static int is_operator_char(char character)
{
	int i;
	for (i = 0; i < OPERATOR_COUNT; i++)
	{
		if (operator_chars[i] == character)
			return 1;
	}
	return 0;
}

/* Finalizes current token and prepares for next one */
static void finalize_token(parser_state_t next_state, token_type_t token_type)
{
	current_token.content[token_buffer_index] = '\0'; /* Null terminate string */
	current_token.size = token_buffer_index;		  /* Store string length */
	token_buffer_index = 0;							  /* Reset buffer index */
	current_state = next_state;						  /* Update parser state */
	current_token.type = token_type;				  /* Set token type */
}

/* ********************************************************************************************************************** */
/* ********************************************************************************************************************** */
/* ********************************************************************************************************************** */
/* ********************************************************************************************************************** */

/********** Main parser function **********/

/* Extracts and classifies the next token from input stream */
parser_token_t *extract_next_token(FILE *input_stream)
{
	int current_char, previous_char;
	parser_token_t *token_pointer = NULL;

	/* Process file character by character */
	while ((current_char = fgetc(input_stream)) != EOF)
	{

#ifdef DEBUG
		//	putchar(current_char);
#endif
		switch (current_state)
		{
		case PARSE_STATE_IDLE:
			if ((token_pointer = handle_idle_state(input_stream, current_char)) != NULL)
				return token_pointer;
			break;
		case PARSE_STATE_SINGLE_LINE_COMMENT:
			if ((token_pointer = handle_single_comment_state(input_stream, current_char)) != NULL)
				return token_pointer;
			break;
		case PARSE_STATE_MULTI_LINE_COMMENT:
			if ((token_pointer = handle_multi_comment_state(input_stream, current_char)) != NULL)
				return token_pointer;
			break;
		case PARSE_STATE_PREPROCESSOR_DIRECTIVE:
			if ((token_pointer = handle_preprocessor_state(input_stream, current_char)) != NULL)
				return token_pointer;
			break;
		case PARSE_STATE_RESERVE_KEYWORD:
			if ((token_pointer = handle_keyword_state(input_stream, current_char)) != NULL)
				return token_pointer;
			break;
		case PARSE_STATE_NUMERIC_CONSTANT:
			if ((token_pointer = handle_number_state(input_stream, current_char)) != NULL)
				return token_pointer;
			break;
		case PARSE_STATE_STRING:
			if ((token_pointer = handle_string_state(input_stream, current_char)) != NULL)
				return token_pointer;
			break;
		case PARSE_STATE_HEADER_FILE:
			if ((token_pointer = handle_header_state(input_stream, current_char)) != NULL)
				return token_pointer;
			break;
		case PARSE_STATE_ASCII_CHAR:
			if ((token_pointer = handle_ascii_state(input_stream, current_char)) != NULL)
				return token_pointer;
			break;
		case PARSE_STATE_FORMAT_SPECIFIER:
			if ((token_pointer = handle_format_specifier_state(input_stream, current_char)) != NULL)
				return token_pointer;
			break;
		case PARSE_STATE_OPERATORS:
			if ((token_pointer = handle_operator_state(input_stream, current_char)) != NULL)
				return token_pointer;
			break;
		case PARSE_STATE_SYMBOLS:
			if ((token_pointer = handle_symbol_state(input_stream, current_char)) != NULL)
				return token_pointer;
			break;
		default:
			printf("ERROR: Unknown parser state encountered\n");
			current_state = PARSE_STATE_IDLE;
			break;
		}
	}

	/* Handle end of file */
	finalize_token(PARSE_STATE_IDLE, TOKEN_END_OF_FILE);
	return &current_token;
}

/* ********************************************************************************************************************** */
/* ********************************************************************************************************************** */
/* ********************************************************************************************************************** */
/* ********************************************************************************************************************** */

/********** State handler implementations **********/

parser_token_t *handle_idle_state(FILE *input_stream, int current_char)
{
	int next_char;
	switch (current_char)
	{
	case '\'': /* ASCII character literal start */
		if (token_buffer_index)		/* single quote while it's already in the middle of accumulating regular text - eg: variable_name'a' */
		{
			fseek(input_stream, -1L, SEEK_CUR); /* Rewind one character */
			finalize_token(PARSE_STATE_IDLE, TOKEN_REGULAR_TEXT);
			return &current_token;
		}
		else
		{
			current_state = PARSE_STATE_ASCII_CHAR;
			current_token.content[token_buffer_index++] = current_char;
			break;
		}
	case '/':
		next_char = current_char;
		if ((current_char = fgetc(input_stream)) == '*') /* Multi-line comment start */
		{
			if (token_buffer_index) /* Process pending regular text first */
			{
				fseek(input_stream, -2L, SEEK_CUR); /* Rewind both characters */
				finalize_token(PARSE_STATE_IDLE, TOKEN_REGULAR_TEXT);
				return &current_token;
			}
			else /* Begin multi-line comment parsing */
			{
#ifdef DEBUG
				printf("Multi line comment Begin : /*\n");
#endif
				current_state = PARSE_STATE_MULTI_LINE_COMMENT;
				current_token.content[token_buffer_index++] = next_char;
				current_token.content[token_buffer_index++] = current_char;
			}
		}
		else if (current_char == '/') /* Single-line comment start */
		{
			if (token_buffer_index) /* Process pending regular text first */
			{
				fseek(input_stream, -2L, SEEK_CUR); /* Rewind both characters */
				finalize_token(PARSE_STATE_IDLE, TOKEN_REGULAR_TEXT);
				return &current_token;
			}
			else /* Begin single-line comment parsing */
			{
#ifdef DEBUG
				printf("Single line comment Begin : //\n");
#endif
				current_state = PARSE_STATE_SINGLE_LINE_COMMENT;
				current_token.content[token_buffer_index++] = next_char;
				current_token.content[token_buffer_index++] = current_char;
			}
		}
		else /* Regular division operator */
		{
			current_token.content[token_buffer_index++] = next_char;
			current_token.content[token_buffer_index++] = current_char;
		}
		break;
	case '#': /* Preprocessor directive */
		if (token_buffer_index)
		{
			fseek(input_stream, -1L, SEEK_CUR); /* Rewind character */
			finalize_token(PARSE_STATE_IDLE, TOKEN_REGULAR_TEXT);
			return &current_token;
		}
		else
		{
			current_state = PARSE_STATE_PREPROCESSOR_DIRECTIVE;
			current_token.content[token_buffer_index++] = current_char;
		}
		break;
	case '\"': /* String literal start */
		if (token_buffer_index)
		{
			fseek(input_stream, -1L, SEEK_CUR); /* Rewind character */
			finalize_token(PARSE_STATE_IDLE, TOKEN_REGULAR_TEXT);
			return &current_token;
		}
		else
		{
			current_state = PARSE_STATE_STRING;
			current_token.content[token_buffer_index++] = current_char;
		}
		break;

	case '0' ... '9': /* Numeric constant detection */
		if (token_buffer_index)
		{
			fseek(input_stream, -1L, SEEK_CUR); /* Rewind character */
			finalize_token(PARSE_STATE_IDLE, TOKEN_REGULAR_TEXT);
			return &current_token;
		}
		else
		{
			current_state = PARSE_STATE_NUMERIC_CONSTANT;
			current_token.content[token_buffer_index++] = current_char;
		}
		break;

	case 'a' ... 'z': /* Potential keyword detection */
	case 'A' ... 'Z':
	case '_':
		if (token_buffer_index)
		{
			fseek(input_stream, -1L, SEEK_CUR); /* Rewind character */
			finalize_token(PARSE_STATE_IDLE, TOKEN_REGULAR_TEXT);
			return &current_token;
		}
		else
		{
			current_state = PARSE_STATE_RESERVE_KEYWORD;
			current_token.content[token_buffer_index++] = current_char;
		}
		break;
	default: /* Handle operators, symbols, and other text */
		if (is_operator_char(current_char) && token_buffer_index > 0)
		{
			fseek(input_stream, -1L, SEEK_CUR); /* Rewind character */
			finalize_token(PARSE_STATE_OPERATORS, TOKEN_REGULAR_TEXT);
			return &current_token;
		}
		else if (is_delimiter_char(current_char) && token_buffer_index > 0)
		{
			fseek(input_stream, -1L, SEEK_CUR); /* Rewind character */
			finalize_token(PARSE_STATE_SYMBOLS, TOKEN_REGULAR_TEXT);
			return &current_token;
		}
		else if (is_operator_char(current_char))
		{
			current_state = PARSE_STATE_OPERATORS;
			current_token.content[token_buffer_index++] = current_char;
		}
		else if (is_delimiter_char(current_char))
		{
			current_state = PARSE_STATE_SYMBOLS;
			current_token.content[token_buffer_index++] = current_char;
		}
		else
		{
			current_token.content[token_buffer_index++] = current_char;
		}
		break;
	}
	return NULL;
}

/* ********************************************************************************************************************** */
/* ********************************************************************************************************************** */
/* ********************************************************************************************************************** */
/* ********************************************************************************************************************** */

parser_token_t *handle_preprocessor_state(FILE *input_stream, int current_char)
{
	switch (preprocessor_substate)
	{
	case PARSE_STATE_SUB_PREPROCESSOR_MAIN:
		return handle_preprocessor_main_state(input_stream, current_char);
	case PARSE_STATE_SUB_PREPROCESSOR_RESERVE_KEYWORD:
		return handle_keyword_state(input_stream, current_char);
	case PARSE_STATE_SUB_PREPROCESSOR_ASCII_CHAR:
		return handle_ascii_state(input_stream, current_char);
	default:
		printf("ERROR: Invalid preprocessor substate\n");
		current_state = PARSE_STATE_IDLE;
	}
	return NULL;
}

parser_token_t *handle_preprocessor_main_state(FILE *input_stream, int current_char)
{
	static int space_encountered = 1;
	switch (current_char)
	{
	case ' ': /* Space in preprocessor directive */
		space_encountered = 0;
		current_token.content[token_buffer_index++] = current_char;
		break;
	case '<': /* System header file start */
		space_encountered = 1;
		finalize_token(PARSE_STATE_HEADER_FILE, TOKEN_PREPROCESSOR_DIRECTIVE);
		return &current_token;
		break;
	case '"': /* User-defined header file start */
		space_encountered = 1;
		fseek(input_stream, -1L, SEEK_CUR);
		finalize_token(PARSE_STATE_HEADER_FILE, TOKEN_PREPROCESSOR_DIRECTIVE);
		current_token.attribute = USER_DEFINED_HEADER;
		return &current_token;
		break;
	case '\n': /* End of preprocessor directive */
		finalize_token(PARSE_STATE_IDLE, TOKEN_PREPROCESSOR_DIRECTIVE);
		current_token.attribute = USER_DEFINED_HEADER;
		return &current_token;
		break;
	case 'a' ... 'z': /* Macro or directive name */
	case 'A' ... 'Z':
		if (space_encountered)
		{
			current_token.content[token_buffer_index++] = current_char;
			return NULL;
		}
		space_encountered = 1;
		fseek(input_stream, -1L, SEEK_CUR);
		finalize_token(PARSE_STATE_IDLE, TOKEN_PREPROCESSOR_DIRECTIVE);
		return &current_token;
		break;
	default:
		current_token.content[token_buffer_index++] = current_char;
		break;
	}
	return NULL;
}

parser_token_t *handle_header_state(FILE *input_stream, int current_char)
{
	switch (current_char)
	{
	case '>': /* System header file end */
		finalize_token(PARSE_STATE_IDLE, TOKEN_HEADER_FILE);
		return &current_token;
		break;
	case '"': /* User header file handling */
		if (token_buffer_index == 0)
		{
			current_token.content[token_buffer_index++] = current_char;
			break;
		}
		current_token.content[token_buffer_index++] = current_char;
		finalize_token(PARSE_STATE_IDLE, TOKEN_HEADER_FILE);
		current_token.attribute = USER_DEFINED_HEADER;
		return &current_token;
		break;
	default: /* Accumulate header file name */
		current_token.content[token_buffer_index++] = current_char;
		break;
	}
	return NULL;
}

parser_token_t *handle_keyword_state(FILE *input_stream, int current_char)
{
	/* Handle operators within keywords */
	if (is_operator_char(current_char))
	{
		fseek(input_stream, -1L, SEEK_CUR);
		finalize_token(PARSE_STATE_OPERATORS, TOKEN_RESERVE_KEYWORD);
		int keyword_classification = classify_keyword(current_token.content);
		if (keyword_classification == DATATYPE_KEYWORD)
			current_token.attribute = DATATYPE_KEYWORD;
		else if (keyword_classification == CONTROL_KEYWORD)
			current_token.attribute = CONTROL_KEYWORD;
		else
			current_token.type = TOKEN_REGULAR_TEXT;
		return &current_token;
	}

	/* Handle symbols within keywords */
	if (is_delimiter_char(current_char))
	{
		fseek(input_stream, -1L, SEEK_CUR);
		finalize_token(PARSE_STATE_SYMBOLS, TOKEN_RESERVE_KEYWORD);
		int keyword_classification = classify_keyword(current_token.content);
		if (keyword_classification == DATATYPE_KEYWORD)
			current_token.attribute = DATATYPE_KEYWORD;
		else if (keyword_classification == CONTROL_KEYWORD)
			current_token.attribute = CONTROL_KEYWORD;
		else
			current_token.type = TOKEN_REGULAR_TEXT;
		return &current_token;
	}

	switch (current_char)
	{
	case '(':
	case ')':
	case '[':
	case ' ': /* Keyword termination characters */
	case '\n':
	case '\t':
	case ';':
		fseek(input_stream, -1L, SEEK_CUR);
		finalize_token(PARSE_STATE_IDLE, TOKEN_RESERVE_KEYWORD);
		int keyword_classification = classify_keyword(current_token.content);
		if (keyword_classification == DATATYPE_KEYWORD)
			current_token.attribute = DATATYPE_KEYWORD;
		else if (keyword_classification == CONTROL_KEYWORD)
			current_token.attribute = CONTROL_KEYWORD;
		else
			current_token.type = TOKEN_REGULAR_TEXT;
		return &current_token;
		break;
	case '0' ... '9': /* Number within identifier */
		fseek(input_stream, -1L, SEEK_CUR);
		finalize_token(PARSE_STATE_NUMERIC_CONSTANT, TOKEN_RESERVE_KEYWORD);
		int keyword_type = classify_keyword(current_token.content);
		if (keyword_type == DATATYPE_KEYWORD)
			current_token.attribute = DATATYPE_KEYWORD;
		else if (keyword_type == CONTROL_KEYWORD)
			current_token.attribute = CONTROL_KEYWORD;
		else
			current_token.type = TOKEN_REGULAR_TEXT;
		return &current_token;
		break;
	default:
		current_token.content[token_buffer_index++] = current_char;
		break;
	}
	return NULL;
}

parser_token_t *handle_number_state(FILE *input_stream, int current_char)
{
	switch (current_char)
	{
	case '\n':
	case ' ':
	case '\t':
	case ';':
	case ')':
	case '}':
	case ']':
	case ',': /* Numeric constant termination */
		fseek(input_stream, -1L, SEEK_CUR);
		finalize_token(PARSE_STATE_IDLE, TOKEN_NUMERIC_CONSTANT);
		return &current_token;
	default: /* Continue accumulating digits */
		current_token.content[token_buffer_index++] = current_char;
		break;
	}
	return NULL;
}

parser_token_t *handle_string_state(FILE *input_stream, int current_char)
{
	switch (current_char)
	{
	case '\"': /* String literal end */
		current_token.content[token_buffer_index++] = current_char;
		finalize_token(PARSE_STATE_IDLE, TOKEN_STRING_LITERAL);
		return &current_token;
		break;
	case '%': /* Format specifier detection */
		fseek(input_stream, -1L, SEEK_CUR);
		finalize_token(PARSE_STATE_FORMAT_SPECIFIER, TOKEN_STRING_LITERAL);
		return &current_token;
	default: /* Accumulate string content */
		current_token.content[token_buffer_index++] = current_char;
		return NULL;
		break;
	}
}

/**** End of file ****/

parser_token_t *handle_single_comment_state(FILE *input_stream, int current_char)
{
	switch (current_char)
	{
	case '\n': /* Single-line comment termination */
#ifdef DEBUG
		printf("\nSingle line comment end\n");
#endif
		current_token.content[token_buffer_index++] = current_char;
		finalize_token(PARSE_STATE_IDLE, TOKEN_SINGLE_LINE_COMMENT);
		return &current_token;
	default: /* Accumulate comment text */
		current_token.content[token_buffer_index++] = current_char;
		break;
	}
	return NULL;
}

parser_token_t *handle_multi_comment_state(FILE *input_stream, int current_char)
{
	int previous_char;
	switch (current_char)
	{
	case '*': /* Potential comment end */
		previous_char = current_char;
		current_token.content[token_buffer_index++] = current_char;
		if ((current_char = fgetc(input_stream)) == '/')
		{
#ifdef DEBUG
			printf("\nMulti line comment End : */\n");
#endif
			previous_char = current_char;
			current_token.content[token_buffer_index++] = current_char;
			finalize_token(PARSE_STATE_IDLE, TOKEN_MULTI_LINE_COMMENT);
			return &current_token;
		}
		else /* Comment continues */
		{
			current_token.content[token_buffer_index++] = current_char;
		}
		break;
	case '/':								 /* Check for comment end pattern */
		fseek(input_stream, -2L, SEEK_CUR);	 /* Look back two positions */
		previous_char = fgetc(input_stream); /* Read previous character */
		fgetc(input_stream);				 /* Return to current position */

		current_token.content[token_buffer_index++] = current_char;
		if (previous_char == '*') /* Comment actually ends */
		{
			finalize_token(PARSE_STATE_IDLE, TOKEN_MULTI_LINE_COMMENT);
			return &current_token;
		}
		break;
	default: /* Accumulate comment content */
		current_token.content[token_buffer_index++] = current_char;
		break;
	}
	return NULL;
}

parser_token_t *handle_ascii_state(FILE *input_stream, int current_char)
{
	char next_char;
	if (current_char == '\'') /* Potential ASCII char end */
	{
		if ((next_char = fgetc(input_stream)) == '\'') /* Handle consecutive quotes (empty char or adjacent char literals) - eg: '', ''', 'a''a', 'a'' */
		{
			current_token.content[token_buffer_index++] = next_char;
		}
		else
		{
			fseek(input_stream, -1L, SEEK_CUR); /* Rewind if not closing quote */
		}
		current_token.content[token_buffer_index++] = current_char;
		finalize_token(PARSE_STATE_IDLE, TOKEN_ASCII_CHAR);
		return &current_token;
	}
	current_token.content[token_buffer_index++] = current_char; /* Accumulate character content */
	return NULL;
}

parser_token_t *handle_format_specifier_state(FILE *input_stream, int current_char)
{
	char next_char;
	switch (current_char)
	{
	case ' ':
	case '"':
	case '\\': /* Format specifier termination */
		fseek(input_stream, -1L, SEEK_CUR);
		finalize_token(PARSE_STATE_STRING, TOKEN_FORMAT_SPECIFIER);
		return &current_token;
	case '.': /* Precision specifier (e.g., %.2f) */
		next_char = fgetc(input_stream);
		if (next_char >= '0' && next_char <= '9')
		{
			current_token.content[token_buffer_index++] = current_char;
			current_token.content[token_buffer_index++] = next_char;
			return NULL;
		}
		else
		{
			fseek(input_stream, -2L, SEEK_CUR);
			finalize_token(PARSE_STATE_STRING, TOKEN_FORMAT_SPECIFIER);
			return &current_token;
		}
	default: /* Accumulate format specifier */
		current_token.content[token_buffer_index++] = current_char;
		return NULL;
	}
}

parser_token_t *handle_operator_state(FILE *input_stream, int current_char)
{
	if (is_operator_char(current_char)) /* Continue operator sequence */
	{
		current_token.content[token_buffer_index++] = current_char;
		return NULL;
	}
	else /* End of operator sequence */
	{
		fseek(input_stream, -1L, SEEK_CUR);
		finalize_token(PARSE_STATE_IDLE, TOKEN_OPERATORS);
		return &current_token;
	}
}

parser_token_t *handle_symbol_state(FILE *input_stream, int current_char)
{
	if (is_delimiter_char(current_char)) /* Continue symbol sequence */
	{
		current_token.content[token_buffer_index++] = current_char;
		return NULL;
	}
	else /* End of symbol sequence */
	{
		fseek(input_stream, -1L, SEEK_CUR);
		finalize_token(PARSE_STATE_IDLE, TOKEN_SYMBOL);
		return &current_token;
	}
}