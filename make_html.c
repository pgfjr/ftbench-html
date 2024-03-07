#include <stdio.h>
#include <string.h>
#include <ctype.h>

static char buffer[256];
static char* ptr;
static FILE* input_file = NULL;
static FILE* output_file = NULL;

static char* read_line(char* buf, int buf_size)
{
	ptr = fgets(buffer, sizeof(buffer), input_file);

	if (ptr)
	{
		size_t len;

		while (*ptr && (*ptr == ' ' || *ptr == '\t'))
			++ptr;

		len = strlen(ptr);

		if (len > 0)
		{
			ptr[len - 1] = 0;
		}
		return ptr;
	}
	return NULL;
}

static int read_title()
{
	int title = 0;

	while (!feof(input_file))
	{
		char *ptr = read_line(buffer, sizeof(buffer), input_file);

		if (ptr)
		{
			if (*ptr == '\0')
				continue;
			else
			{
				// no hyphen in the log file, but just in case
				if (*ptr == '-')
				{
					return title;
				}
				else
				{
					// should be found first
					fprintf(output_file, "<h1>%s</h1>\n", ptr);
					return ++title;
				}
			}
		}
		else
		{
			return 0;
		}
	}
	return title;
}

static void
write_cells(const char* ptr)
{
	const char* colon = strchr(ptr, ':');

	if (colon)
	{
		fprintf(output_file, "<tr><td>%.*s</td>", (int)(colon - ptr), ptr);

		++colon;

		while (*colon && isspace(*colon))
		{
			++colon;
		}
		if (*colon)
		{
			fprintf(output_file, "<td>%s</td></tr>\n", colon);
		}
		else
		{
			fputs("<td></td></tr>\n", output_file);
		}
	}
	else
	{
		fprintf(output_file, "<tr><td><pre>%s</pre></td></tr>\n", ptr);
	}
}


static int is_header(const char* ptr)
{
	const char* colon = strchr(ptr, ':');

	if (!colon)
	{
		return 1;
	}
	else
	{
		ptr = ++colon;

		while (*ptr)
		{
			if (isspace(*ptr))
			{
				++ptr;
			}
			else
			{
				return 0;
			}
		}
		return 1;
	}
}

static void
write_table_data(int row_count, const char* ptr)
{
	if (row_count == 1)
	{
		fputs("<table border='1' style='border-collapse: collapse;'>", output_file);

		if (is_header(ptr))
		{
			fprintf(output_file, "<tr><th style='text-align: left;'>%s</th></tr>\n", ptr);
			return;
		}
	}
	write_cells(ptr);
}

static void 
read_tables()
{
	int row_count = 0;
	int table_count = 0;

	while (!feof(input_file))
	{
		ptr = read_line(buffer, sizeof(buffer), input_file);

		if (ptr)
		{
			if (*ptr == '\0')
			{
				if (row_count > 0)
				{
					fputs("</table>\n", output_file);

				}
				row_count = 0;
			}
			else
			{
				if (++row_count == 1)
				{
					if (++table_count > 1)
						fputs("<br>\n", output_file);
				}

				write_table_data(row_count, ptr);
			}
		}
		else
		{
			break;
		}
	}
	if (row_count > 0)
	{
		fputs("</table>\n", output_file);
	}
}

static void
	write_html_header(const char *fontfile)
{
	fprintf(output_file, 
		"<html>\n"
		"<head>\n"
		"<title>\n"
		"ftbench %s\n"
		"</title>\n"
		"<style>\n"
		"\ttable {\n"
		"\tborder-collapse: separate;\n"
		"\tborder-spacing: 0 20px;\n"
		"\t}\n"
		"\tth, td {\n"
		"\tpadding: 10px;\n"
		"}\n"
		"\ttr {\n"
		"\tborder-bottom: 1px solid #ddd;\n"
		"}\n"
		"</style>\n"
		"</head>\n"
		"<body>\n", 
		fontfile);
}

static void
write_html_ender()
{
	fputs("</body>\n</html>", output_file);
}
void make_html(const char* log_filename, const char *fontfile)
{
	input_file = fopen(log_filename, "r");

	if (!input_file)
	{
		fprintf(stderr, "Unable to open %s\n", log_filename);
		return;
	}
	else
	{
		output_file = fopen("log.html", "w");

		if (!output_file)
		{
			fputs("Unable to create log.html", stderr);
			fclose(input_file);
			input_file = NULL;
			return;

		}
		write_html_header(fontfile);

		if (read_title())
		{
			fputs("<p>\n", output_file);
			read_tables();
			fputs("</p>\n", output_file);
		}
		else
		{
			fputs("<p>Failed to read the log file</p>\n", output_file);
		}
		write_html_ender();
		fclose(input_file);
		fclose(output_file);
		input_file = output_file = NULL;
	}
}