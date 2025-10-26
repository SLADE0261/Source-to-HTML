#include <stdio.h>
#include "s2html_event.h"
#include "s2html_conv.h"

/* HTML document structure generation functions */
void generate_html_header(FILE* output_file, int mode) /* mode parameter for future extensibility */
{
	/* Write HTML document opening structure */
	fprintf(output_file, "<!DOCTYPE html>\n");
	fprintf(output_file, "<html lang=\"en-US\">\n");
	fprintf(output_file, "<head>\n");
	fprintf(output_file, "<title>%s</title>\n", "sode2html");
	fprintf(output_file, "<meta charset=\"UTF-8\">\n");
	fprintf(output_file, "<link rel=\"stylesheet\" href=\"styles.css\">\n");
	fprintf(output_file, "</head>\n");
	fprintf(output_file, "<body>\n");
	fprintf(output_file, "<pre>\n");
}

void generate_html_footer(FILE* output_file, int mode) /* mode parameter for future extensibility */
{
	/* Write HTML document closing structure */
	fprintf(output_file, "</pre>\n");
	fprintf(output_file, "</body>\n");
	fprintf(output_file, "</html>\n");
}

/* Token to HTML conversion function */
void convert_token_to_html(FILE* output_file, parser_token_t *token_data)
{
#ifdef DEBUG
	printf("%s", token_data->content);
#endif
	
	switch(token_data->type)
	{
		case TOKEN_PREPROCESSOR_DIRECTIVE:
			fprintf(output_file,"<span class=\"preprocess_dir\">%s</span>", token_data->content);
			break;
		case TOKEN_MULTI_LINE_COMMENT:
		case TOKEN_SINGLE_LINE_COMMENT:
			fprintf(output_file,"<span class=\"comment\">%s</span>", token_data->content);
			break;
		case TOKEN_STRING_LITERAL:
			fprintf(output_file,"<span class=\"string\">%s</span>", token_data->content);
			break;
		case TOKEN_HEADER_FILE:
			/* Handle user-defined vs system headers differently */
			if(token_data->attribute == USER_DEFINED_HEADER)
			{
				fprintf(output_file,"<span class=\"header_file\">%s</span>", token_data->content);
			}
			else
			{
				fprintf(output_file,"<span class=\"header_file\">&lt;%s&gt;</span>", token_data->content);
			}
			break;
		case TOKEN_REGULAR_TEXT:
		case TOKEN_END_OF_FILE :
			fprintf(output_file,"%s", token_data->content);
			break;
		case TOKEN_NUMERIC_CONSTANT:
			fprintf(output_file,"<span class=\"numeric_constant\">%s</span>", token_data->content);
			break;
		case TOKEN_RESERVE_KEYWORD:
			if(token_data->attribute == DATATYPE_KEYWORD)
			{
				fprintf(output_file,"<span class=\"reserved_key1\">%s</span>", token_data->content);
			}
			else
			{
				fprintf(output_file,"<span class=\"reserved_key2\">%s</span>", token_data->content);
			}
			break;
		case TOKEN_ASCII_CHAR:
			fprintf(output_file,"<span class=\"ascii_char\">%s</span>", token_data->content);
			break;
		case TOKEN_FORMAT_SPECIFIER:
			fprintf(output_file,"<span class=\"format_specifier\">%s</span>", token_data->content);
			break;
		case TOKEN_OPERATORS:
			fprintf(output_file,"<span class=\"operators\">%s</span>", token_data->content);
			break;
		case TOKEN_SYMBOL:
			fprintf(output_file,"<span class=\"symbols\">%s</span>", token_data->content);
			break;
		default :
			printf("ERROR: Unknown token type encountered\n");
			break;
	}
}