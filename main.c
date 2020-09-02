#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

#define kAnsiUnderline "\033[4m"
#define kAnsiReset     "\033[0m"

enum eColorMode {
	VgaColor,  // oldskool 256-color palette
	TrueColor, // full 24-bit color
};

void printHelp() {
	puts("hxdmp - the no-nonsense hexdumper");
	puts("");
	puts("Usage:");
	puts("    hxdmp [options] <filepath>");
	puts("");
	puts("Options:");
	puts("");
	puts("    -h,--help");
	puts("        Display this help menu.");
	puts("");
	puts("    -s,--start " kAnsiUnderline "offset" kAnsiReset);
	puts("        Skip " kAnsiUnderline "offset" kAnsiReset " bytes from the start of the input.");
	puts("");
	puts("    -n,--length " kAnsiUnderline "length" kAnsiReset);
	puts("        Only read " kAnsiUnderline "length" kAnsiReset " bytes of the input.");
	puts("");
	puts("    -w,--width " kAnsiUnderline "width" kAnsiReset);
	puts("        Display " kAnsiUnderline "width" kAnsiReset " bytes per row.");
	puts("        You can replace the default with the HXDMP_WIDTH environment variable.");
	puts("");
	puts("    -l,--layout " kAnsiUnderline "layout_str" kAnsiReset);
	puts("        Customize the column layout using a layout string.");
	puts("        Each character of " kAnsiUnderline "layout_str" kAnsiReset " represents a column.");
	puts("        Available column types are:");
	puts("        a: Ascii   Each byte is printed in ascii, or '.' for non-ascii values");
	puts("        c: Color   Each byte is printed as a █, colored according to its value");
	puts("        x: heX     Each byte is printed as two lowercase hex digits");
	puts("        X: heX     Each byte is printed as two uppercase hex digits");
	puts("        The default layout string is xa for consistency with most hex editors.");
	puts("        You can replace the default with the HXDMP_LAYOUT environment variable.");
	puts("");
	puts("    Numeric values can be provided in hexadecimal.");
	puts("    0xFF, $FF, and FFh syntaxes are all supported.");
}

bool tryParseHex(char const* str, int digits, int64_t* result)
{
	*result = 0;

	for (int i = 0; i < digits; ++i) {
		char const c = str[i];
		*result *= 16;
		if (c >= '0' && c <= '9') {
			*result += (c-'0');
		} else if (c >= 'A' && c <= 'F') {
			*result += 0xA+(c-'A');
		} else if (c >= 'a' && c <= 'f') {
			*result += 0xA+(c-'a');
		} else {
			return false;
		}
	}
	return true;
}

bool tryParseDecimal(char const* str, int digits, int64_t* result)
{
	*result = 0;

	for (int i = 0; i < digits; ++i) {
		char const c = str[i];
		*result *= 10;
		if (c >= '0' && c <= '9') {
			*result += (c-'0');
		} else {
			return false;
		}
	}
	return true;
}

bool tryParseNumber(char const* str, int64_t* result)
{
	*result = 0;

	if (strlen(str)>2 && str[0] == '0' && str[1] == 'x') {
		return tryParseHex(str+2, strlen(str)-2, result);
	}
	else if (strlen(str)>1 && str[0] == '$') {
		return tryParseHex(str+1, strlen(str)-1, result);
	}
	else if (strlen(str) > 1 && str[strlen(str)-1] == 'h') {
		return tryParseHex(str, strlen(str)-1, result);
	}
	else {
		return tryParseDecimal(str, strlen(str), result);
	}
}

int getDefaultWidth()
{
	char const* widthStr = getenv("HXDMP_WIDTH");
	if (widthStr && strlen(widthStr) > 0) {
		int64_t parsedValue;
		if (tryParseNumber(widthStr, &parsedValue)) {
			if (parsedValue > 0) {
				return parsedValue;
			} else {
				eprintf("Cannot use width of %lld from HXDMP_WIDTH, must be greater than zero.\n", parsedValue);
				exit(1);
			}
		} else {
			eprintf("Failed to parse '%s' from HXDMP_WIDTH as a number.\n", widthStr);
			exit(1);
		}
	}

	return 16;
}

char const* getDefaultLayout()
{
	char const* layoutStr = getenv("HXDMP_LAYOUT");
	if (layoutStr && strlen(layoutStr) > 0) {
		return layoutStr;
	}

	return "xa";
}

enum eColorMode determineColorSupport()
{
	const char* colorterm = getenv("COLORTERM");
	if (colorterm && (!strcmp(colorterm,"truecolor") || !strcmp(colorterm,"24bit"))) {
		return TrueColor;
	} else {
		return VgaColor;
	}
}

int main(int argc, char** argv)
{
	// set up default state
	int width = getDefaultWidth();
	int64_t start = 0;
	int64_t length = -1; // -1 is our magic value to say "read to the end"
	char const* layout = getDefaultLayout();
	enum eColorMode colorMode = determineColorSupport();

	// parse commandline arguments
	while (1) {
		static struct option long_options[]={
			{ "width",  required_argument, 0, 'w' },
			{ "start",  required_argument, 0, 's' },
			{ "length", required_argument, 0, 'n' },
			{ "layout", required_argument, 0, 'l' },
			{ "help",         no_argument, 0, 'h' },
			{ 0, 0, 0, 0 }
		};
		int option_index = 0;
		int c = getopt_long(argc, argv, "w:s:n:l:h", long_options, &option_index);
		if (c == -1) {
			break;
		}
		switch(c) {
			case 'w': {
				int64_t result;
				if (tryParseNumber(optarg, &result)) {
					if (result > 0) {
						width = result;
					} else {
						eprintf("Cannot use width of %lld from --width, must be greater than zero.\n", result);
						exit(1);
					}
				} else {
					eprintf("Failed to parse '%s' from --width as a number.\n", optarg);
					exit(1);
				}
				break;
			}	
			case 's': {
				int64_t result;
				if (tryParseNumber(optarg, &result)) {
					start = result;
				} else {
					eprintf("Failed to parse '%s' from --start as a number.\n", optarg);
					exit(1);
				}
				break;
			}
			case 'n': {
				int64_t result;
				if (tryParseNumber(optarg, &result)) {
					length = result;
				} else {
					eprintf("Failed to parse '%s' from --length as a number.\n", optarg);
					exit(1);
				}
				break;
			}
			case 'l':
				if (optarg && strlen(optarg) > 0) {
					layout = optarg;
				} else {
					eprintf("Cannot use an empty layout string.\n");
					exit(1);
				}
				break;
			case 'h':
				printHelp();
				return 0;
		}
	}
	
	// if we don't have a file to read, print help and exit
	if (optind >= argc) {
		printHelp();
		return 1;
	}

	FILE* fh = fopen(argv[optind], "rb");
	if (!fh) {
		eprintf("Failed to open '%s' for reading.\n", argv[optind]);
		return 1;
	}

	uint8_t* rowBuffer = (uint8_t*)malloc(width);
	size_t totalBytesRead = 0;
	bool shouldBreak;
	do {
		size_t const rowAddress = ftell(fh);
		shouldBreak = false;
		int bytesRead = fread(rowBuffer, 1, width, fh);
		totalBytesRead += bytesRead;
		if (length >= 0 && totalBytesRead > (size_t)length) {
			bytesRead -= (totalBytesRead-length);
			shouldBreak = true;
		}
		if (bytesRead == 0) {
			break;
		}
		printf("%08lx |", rowAddress);
		for (unsigned int l = 0; l < strlen(layout); ++l) {
			switch (layout[l]) {
				case 'x':
					printf(" ");
					for (int i = 0; i < width; ++i) {
						if (i < bytesRead) {
							printf("%02x ", rowBuffer[i]);
						} else {
							printf("   ");
						}
					}
					printf("|");
					break;
				case 'X':
					printf(" ");
					for (int i = 0; i < width; ++i) {
						if (i < bytesRead) {
							printf("%02X ", rowBuffer[i]);
						} else {
							printf("   ");
						}
					}
					printf("|");
					break;
				case 'a':
					printf(" ");
					for (int i = 0; i < width; ++i) {
						if (i < bytesRead) {
							printf("%c", rowBuffer[i] >= ' ' && rowBuffer[i] < 0x7f ? rowBuffer[i] : '.');
						} else {
							printf(" ");
						}
					}
					printf(" |");
					break;
				case 'c':
					printf(" ");
					for (int i = 0; i < width; ++i) {
						if (i < bytesRead) {
							if (i == 0 || rowBuffer[i] != rowBuffer[i-1]) {
								if (colorMode == TrueColor) {
									printf("\x1b[38;2;%d;%d;%dm", rowBuffer[i], rowBuffer[i], rowBuffer[i]);
								} else if (colorMode == VgaColor) {
									printf("\x1b[38;5;%dm", 232+rowBuffer[i]/11);
								}
							}
							printf("█");
						} else {
							printf(" ");
						}
					}
					printf("\x1b[0m");
					printf(" |");
					break;
			}
		}
		printf("\n");
	} while (!shouldBreak);
	free(rowBuffer);
	fclose(fh);
}
