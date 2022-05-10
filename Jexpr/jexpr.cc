
#include "parser.h"
#include "exprs.h"

int usage(const char* arg0, int code)
{
  using namespace std;

  cerr << "\nUsage: " << arg0 << " (switches) infile.j\n\n";
  cerr << "Analyze a Java assembly file. Valid switches:\n";
  cerr << "\n";
  cerr << "\t-h: Display this help and exit, regardless of other switches\n";
  cerr << "\t-j: Output a stripped .j file.\n";
	cerr << "\t-L line  : Restrict output to annotation(s) from given line.\n";
  cerr << "\t-m method: Restrict output to one method.\n";
  cerr << "\t-T: Show more type information.\n";
  cerr << "\t-t: Show less type information (default).\n";
  cerr << "\t-V: With versioning for variables.\n";
  cerr << "\t-v: Without versioning for variables (default).\n";
  cerr << "\t-x: Examine expressions.\n";
  cerr << "\n";
  cerr << "Options -j and -x cancel each other out.\n";
  cerr << "Reads from standard input if no input file is given.\n\n";

  return code;
}

int main(int argc, const char** argv)
{
  const char* infile = 0;
  char outtype = 'x';
  const char* method = 0;
	long line = -1;

  expr::setShowTypes(false);
  expr::setShowVersion(false);

  /*
    Parse command line
  */
  for (int i=1; i<argc; i++) {
    if ('-' != argv[i][0]) {
      // must be the input file
      if (infile) return usage(argv[0], 1);
      infile = argv[i];
      continue;
    }
    // must be a switch
    if (0 != argv[i][2]) {
      std::cerr << "Unknown switch: " << argv[i] << "\n";
      return usage(argv[0], 1);
    }
    switch (argv[i][1]) {
      case 'h':
                return usage(argv[0], 0);

      case 'j':
      case 'x':
                outtype = argv[i][1];
                continue;

			case 'L':
								line = atol(argv[++i]);
								continue;

      case 'm':
                method = argv[++i];
                continue;

      case 't':
      case 'T':
                expr::setShowTypes('T' == argv[i][1]);
                continue;

      case 'v':
      case 'V':
                expr::setShowVersion('V' == argv[i][1]);
                continue;

      default:  std::cerr << "Unknown switch: " << argv[i] << "\n";
                return usage(argv[0], 1);
    }
  }

  dotjfile* J = parse(infile);
  if (0==J) return 1;

  if ('j' == outtype) {
    J->writeDotj(std::cout, method, line, true);
  } else {
    J->writeExprs(std::cout, method, line);
  }
  return 0;
}
