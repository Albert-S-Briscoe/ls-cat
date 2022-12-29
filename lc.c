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

	// process command line arguments
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

	// need this for dircolors to work correctly with my bashrc. It tests for interactivity.
#define SHELL "bash -ci '"
#define SHELL_END "'"

	// I should probably explain the '\"'\"' insanity.
	// First, the c compiler takes the quoted string and applies the \ escape sequences.
	// the result is '"'"'
	// then, at runtime, system() sees: bash -c '... '"'"' ... '"'"' ...'
	// it then calls bash, but first it needs to get the argumets.
	// it takes '... ', "'", ' ...', "'", and ' ...', removes the quotes and concats it into ... ' ... ' ...
	// the center ... is the path, a string the user has full control of, and as such, should be quoted for the entirety of the process until ls or cat has it.
	// TODO: sanitize path input by replacing every ' with '"'"'
	char* cat_fmt        = SHELL "cat   '\"'\"'%s'\"'\"' 2>/dev/null" SHELL_END;
	char* ls_fmt         = SHELL "ls    '\"'\"'%s'\"'\"' 2>/dev/null" SHELL_END;
	char* lsl_fmt        = SHELL "ls -l '\"'\"'%s'\"'\"' 2>/dev/null" SHELL_END;
	char* silent_cat_fmt = SHELL "cat   '\"'\"'%s'\"'\"' 2>/dev/null >/dev/null" SHELL_END;
	char* silent_ls_fmt  = SHELL "ls    '\"'\"'%s'\"'\"' 2>/dev/null >/dev/null" SHELL_END;
	char* silent_lsl_fmt = SHELL "ls -l '\"'\"'%s'\"'\"' 2>/dev/null >/dev/null" SHELL_END;

	if (!systemf(silent_cat_fmt, path)) {
		printf("cat:\n");
		return systemf(cat_fmt, path);
	} else {
		if (!systemf(lsl ? silent_lsl_fmt : silent_ls_fmt, path)) {
			printf("ls:\n");
			return systemf(lsl ? lsl_fmt : ls_fmt, path);
		} else {
			fprintf(stderr, "Error: cat and ls both failed\n");
			exit(1);
		}
	}

	return 0;
}
