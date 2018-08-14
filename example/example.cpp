#include <argparser.hpp>

int main(int argc, char* argv[]) {
  
  // DODO: add string to name the app.
  cli::ArgParser app;

  std::string a;
  std::string b;
  std::string c;
  std::string s;
  
  // The following argument was not expected: --versiof
  // Run with --help for more information.
  
  // TODO: 
  // (1) we don't need require here -> force to at least...
  app.add_option("-f, -friend,--qw,--flag", "this is an example option", a, b, c);
  app.add_option("--string", "this is an example option", s);

  ARG_PARSE(app, argc, argv);

  std::cout << a << '\n'
            << b << '\n'
            << c << '\n';

  return 0;
}
