#ifndef S2HTML_CONV_H
#define S2HTML_CONV_H

/* HTML generation mode constants */

#define HTML_DOCUMENT_START 1
#define HTML_DOCUMENT_END 0

/********** function prototypes **********/

void generate_html_header(FILE *output_file, int mode); /* mode => controls HTML tag generation behavior */
void generate_html_footer(FILE *output_file, int mode); /* mode => controls HTML tag generation behavior */
void convert_token_to_html(FILE *output_file, parser_token_t *token_data);

#endif