/*******************************
 * Author: Albert Briscoe
 * Creation date: 2022-12-29
 * Description: tool that cats file paths when it can (when it is a file or link to a file) and runs ls on them otherwise
 ******************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

void print_usage(void) {
	printf("Usage: lc [OPTION]... [PATH]\n"
		"OPTIONS is a space separated list of one or more of the following:\n"
		"    -l  Passed to ls (long format)\n"
		"    -h  Print this message\n");
}

// chatgpt is amazing. Credit for this function goes to it.
// blame it for any memory leaks/weird behavior here.
int systemf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    // Determine the size of the resulting string
    int size = vsnprintf(NULL, 0, format, args);
    if (size < 0) {
        va_end(args);
        return size;
    }

    // Allocate memory for the resulting string
    char* result = malloc(size + 1);
    if (result == NULL) {
        va_end(args);
        return -1;
    }

    // Write the resulting string to the allocated memory
    vsprintf(result, format, args);
    va_end(args);

    // Run system() on the resulting string and return the result
    int system_result = system(result);
    free(result);
    return system_result;
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Error: no path specified\n");
		print_usage();
		exit(1);
	}

	char* path = NULL;
	int lsl = 0;

	for (int arg = 1; arg < argc; arg++) {
		if (argv[arg][0] == '-') {
			switch (argv[arg][1]) {
			case 'l':
				lsl = 1;
				break;
			case 'h':
				print_usage();
				exit(0);
			default:
				fprintf(stderr, "Error: unrecognized option \"%s\"\n", argv[arg]);
				exit(1);
			}
		} else {
			if (path) {
				fprintf(stderr, "Error: multiple paths specified.\n");
				exit(1);
			} else
				path = argv[arg];
		}
	}

#define SHELL "bash -ci '"
#define SHELL_END "'"

	char* cat_fmt = SHELL "cat %s 2>/dev/null" SHELL_END;
	char* ls_fmt  = SHELL "ls %s 2>/dev/null" SHELL_END;
	char* lsl_fmt = SHELL "ls -l %s 2>/dev/null" SHELL_END;

	if (systemf(cat_fmt, path)) {
		if (systemf(lsl ? lsl_fmt : ls_fmt, path)) {
			fprintf(stderr, "Error: cat and ls both failed\n");
			exit(1);
		}
	}

	return 0;
}
