#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util/io.h"
#include "parse/lexer.h"
#include "parse/parse.h"
#include "parse/typecheck.h"
#include "gen.h"

const char *usage = "Usage: mycc -mode [-o outfile] [-h] infile\n"
                    "\tmode\t\tinteger from 0-5 specifying mode to run\n"
                    "\toutfile\t\tfile to write output to instead od stdout\n"
                    "\tinfile\t\tfile to read input from\n";

const char *version = "My C compiler\n"
                      "\tAuthor: Chris Johannsen\n"
                      "\tEmail: cgjohann@iastate.edu\n"
                      "\tVersion: 0.1\n"
                      "\tDate: 2-9-2022\n";

FILE *outfile;

int main(int argc, char **argv)
{
  int mode = -1;
  outfile = stdout;

  if(argc < 2) {
    fprintf(stderr, "%s", usage);
    exit(1);
  }

  // Extensible way to loop over CLI options
  char c;
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
        outfile = fopen(argv[optind], "w");
        if(outfile == NULL) {
          fprintf(stderr, "Could not open output file %s, writing to stdout\n", argv[optind]);
          outfile = stdout;
        }
        optind++;
        break;
      }
      case 'h': {
        fprintf(stderr, "%s", usage);
        exit(1);
      }
      case '?': {
        fprintf(stderr, "\n%s", usage);
        exit(1);
      }
      default: {
        exit(1); // something went wrong with getopt
      }
    }
  }

  if(mode < 0) {
    fprintf(stderr, "Mode not specified\n\n%s", usage);
    exit(1);
  }

  if(mode == 0) {
    fprintf(outfile, "%s", version);
    exit(0);
  }

  if(optind >= argc) {
    fprintf(stderr, "No input file provided\n\n%s", usage);
    exit(1);
  }

  char *filename = argv[optind];

  if(mode == 1) {
    tokenize(filename);
    exit(0);
  }

  if(mode == 3) {
    parse(filename);
    exit(0);
  }

  if(mode == 4) {
    typecheck(filename);
    exit(0);
  }

  if(mode == 5) {
    gen_code(filename);
    exit(0);
  }

}