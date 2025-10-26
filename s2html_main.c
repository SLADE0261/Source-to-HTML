#include <stdio.h>
#include "s2html_event.h"
#include "s2html_conv.h"

/********** main program entry point **********/

int main(int argc, char *argv[])
{
	FILE *source_stream, *destination_stream; // input and output file handles
	parser_token_t *parsed_token;
	char output_filename[100];

	/* Validate command line arguments */
	if (argc < 2)
	{
		printf("\nError ! please enter file name and mode\n");
		printf("Usage: <executable> <file name> \n");
		printf("Example : ./a.out abc.txt\n\n");
		return 1;
	}

#ifdef DEBUG
	printf("File to be opened : %s\n", argv[1]);
#endif

	/* Open source file for reading */
	if (NULL == (source_stream = fopen(argv[1], "r")))
	{
		printf("Error! File %s could not be opened\n", argv[1]);
		return 2;
	}
	
	/* Determine output filename */
	if (argc > 2)
	{
		sprintf(output_filename, "%s.html", argv[2]);
	}
	else
	{
		sprintf(output_filename, "%s.html", argv[1]);
	}
	
	/* Create destination file for writing */
	if (NULL == (destination_stream = fopen(output_filename, "w")))
	{
		printf("Error! could not create %s output file\n", output_filename);
		return 3;
	}

	/* Generate HTML document header */
	generate_html_header(destination_stream, HTML_DOCUMENT_START);

	/* Parse source file and convert tokens to HTML */
	do
	{
		parsed_token = extract_next_token(source_stream);
		/* Convert each token to appropriate HTML markup */
		convert_token_to_html(destination_stream, parsed_token);
	} while (parsed_token->type != TOKEN_END_OF_FILE);

	/* Generate HTML document footer */
	generate_html_footer(destination_stream, HTML_DOCUMENT_END);

	printf("\nOutput file %s generated\n", output_filename);
	
	/* Clean up file resources */
	fclose(source_stream);
	fclose(destination_stream);

	return 0;
}