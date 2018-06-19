#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

int g_width = 16;
int g_start = 0;
int g_length = -1;
int g_visual = 0;

int main(int argc, char** argv) {
	while (1) {
		static struct option long_options[]={
			{ "width",  required_argument, 0, 'w' },
			{ "start",  required_argument, 0, 's' },
			{ "length", required_argument, 0, 'n' },
			{ "visual",       no_argument, 0, 'v' },
			{ 0, 0, 0, 0 }
		};
		int option_index = 0;
		int c = getopt_long(argc, argv, "w:s:n:v", long_options, &option_index);
		if (c == -1) {
			break;
		}
		switch(c) {
			case 'w': g_width  = strtol(optarg, NULL, 10); break;
			case 's': g_start  = strtol(optarg, NULL, 10); break;
			case 'n': g_length = strtol(optarg, NULL, 10); break;
			case 'v': {
				const char* colorterm = getenv("COLORTERM");
				if (colorterm && (!strcmp(colorterm,"truecolor") || !strcmp(colorterm,"24bit"))) {
					g_visual = 2;
				} else {
					//puts("Visual mode is not available on your terminal.");
					//return 1;
					g_visual = 1;
				}
				break;
			}
		}
	}
	if (g_width <= 0) {
		puts("Width must be a positive integer.");
		return 1;
	}
	if (g_start < 0) {
		puts("Start must be a positive integer or zero.");
		return 1;
	}
	if (optind >= argc) {
		puts("Expected a filename.");
		return 1;
	}
	FILE* fh = fopen(argv[optind], "rb");
	if (!fh) {
		printf("Failed to open %s\n", argv[optind]);
		return 1;
	}
	fseek(fh, 0, SEEK_END);
	int fileLength = ftell(fh);
	fseek(fh, g_start, SEEK_SET);
	uint8_t* buffer = (uint8_t*)malloc(g_width);
	int totalBytesRead = 0;
	while (ftell(fh) < fileLength) {
		bool shouldBreak = false;
		printf("%08lx  ", ftell(fh));
		int bytesRead = fread(buffer, 1, g_width, fh);
		totalBytesRead += bytesRead;
		if (totalBytesRead > g_length && g_length != -1) {
			bytesRead -= (totalBytesRead-g_length);
			shouldBreak = true;
		}
		if (bytesRead) {
			for (int i = 0; i < g_width; ++i) {
				if (i < bytesRead) {
					printf("%02x ",buffer[i]);
				} else {
					printf("   ");
				}
			}
			printf(" | ");
			for (int i = 0; i < g_width; ++i) {
				if (i < bytesRead) {
					printf("%c", buffer[i] >= ' ' && buffer[i] < 0x7f ? buffer[i] : '.');
				} else {
					printf(" ");
				}
			}
			if (g_visual) {
				printf(" | ");
				for (int i = 0; i < g_width; ++i) {
					if (i < bytesRead) {
						if (i == 0 || buffer[i] != buffer[i-1]) {
							if (g_visual == 2) {
								printf("\x1b[38;2;%d;%d;%dm",buffer[i],buffer[i],buffer[i]);
							} else {
								printf("\x1b[38;5;%dm",232+buffer[i]/11);
							}
						}
						printf("█");
					} else {
						printf(" ");
					}
				}
				printf("\x1b[0m");
			}
			printf(" |");
		}
		printf("\n");
		if (shouldBreak) {
			break;
		}
	}
	free(buffer);
	fclose(fh);
}
