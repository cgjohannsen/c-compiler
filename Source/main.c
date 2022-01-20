#include <stdio.h>

const char *usage = "Usage: mycc -mode [-o outfile] [-h] infile\n"
                    "mode \t \n"
                    "outfile \t file to output into\n"
                    "infile \t file to read input from\n";

int main(int argc, char **argv)
{
    int mode = -1;
    char *outfile;

    // Extensible way to loop over CLI options
    while((c = getopt(argc, argv, "012345oh")) != -1) {
      switch(c) {
        case '0': {
            mode = 0;
            break;
        }
        case '1': {
            mode = 1;
            break;
        }
        case '2': {
            mode = 2;
            break;
        }
        case '3': {
            mode = 3;
            break;
        }
        case '4': {
            mode = 4;
            break;
        }
        case '5': {
            mode = 5;
            break;
        }
        case 'o': {
            printf("TODO");
            return 1;
        }
        case 'h': {
          fprintf(stderr, "%s", usage);
          return 1;
        }
        case '?': {
          fprintf(stderr, "Unknown option %x", optopt);
          return 1;
        }
        default: {
          return 1; // something went wrong with getopt
        }
      }
    }


}