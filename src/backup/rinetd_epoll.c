#define VERSION "0.70"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <getopt.h>
#include <errno.h>
#define INVALID_SOCKET (-1)
#include <sys/time.h>

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>


int get_file_line(FILE *in, char *line, int space, int *lnum);



typedef struct _rinetd_options RinetdOptions;
struct _rinetd_options
{
	char *conf_file;
};

RinetdOptions options = {
	"/etc/rinetd.conf"
};

int read_args (int argc,
	char **argv,
	RinetdOptions *options)
{
	int c;
  
	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			{"conf-file",  1, 0, 'c'},
			{"help",       0, 0, 'h'},
			{"version",    0, 0, 'v'},
			{0, 0, 0, 0}
		};
		c = getopt_long (argc, argv, "c:shv",
			long_options, &option_index);
		if (c == -1) {
			break;
		}
		switch (c) {
			case 'c':
			options->conf_file = (char *)malloc(strlen(optarg) + 1);
			if (!options->conf_file) {
				fprintf(stderr, "Not enough memory to "
					"launch rinetd.\n");
				exit(1);
			}
			strcpy(options->conf_file, optarg);
			break;
			case 'h':
			printf("Usage: rinetd [OPTION]\n"
				"  -c, --conf-file FILE   read configuration "
				"from FILE\n"
				"  -h, --help             display this help\n"
				"  -v, --version          display version "
				"number\n\n");
			printf("Most options are controlled through the\n"
				"configuration file. See the rinetd(8)\n"
				"manpage for more information.\n");
			exit (0);
			case 'v':
			printf ("rinetd %s\n", VERSION);
			exit (0);
			case '?':
			default:
			exit (1);
		}
	}
	return 0;
}


int read_config(){

    FILE *in;
    char line[16384];
	int lnum = 0;

    in = fopen(options.conf_file, "r");
	if (!in) {
		fprintf(stderr, "rinetd: can't open %s\n", options.conf_file);
		exit(1);
	}
	while (1) {
		char *t = 0;
		if (!get_file_line(in, line, sizeof(line), &lnum)) {
			break;
		}
		t = strtok(line, " \t\r\n");
		if (!strcmp(t, "logfile")) { 	
			continue;
		} else if (!strcmp(t, "pidlogfile")) { 	
			continue;
		} else if (!strcmp(t, "logcommon")) {
			continue;
		} else if (!strcmp(t, "allow")) {
			// allowRulesTotal++;
		} else if (!strcmp(t, "deny")) {		
			// denyRulesTotal++;
		} else {	
			/* A regular forwarding rule */
			// seTotal++;	
		}
	}	
	fclose(in);
}

int get_file_line(FILE *in, char *line, int space, int *lnum)
{
	char *p;
	while (1) {
		if (!fgets(line, space, in)) {
			return 0;
		}
		p = line;
		while (isspace(*p)) {
			p++;
		}
		if (!(*p)) {
			/* Blank lines are OK */
			continue;
		}
		if (*p == '#') {
			/* Comment lines are also OK */
			continue;
		}
		(*lnum)++;
		return 1;
	}
}

int main(int argc, char *argv[])
{

    // read_args(argc, argv, &options);

    // read_config();

    FILE *fp = fopen("rinetd.conf", "r");
    char *OneLine = (char *)malloc(sizeof(char) * 100);
     
    while (!feof(fp)) {
        fgets(OneLine, 100, fp);
        puts(OneLine);
    }
     
    free(OneLine);
    fclose(fp);

    printf("hello\n");
}