#include "gen.h"
#include "parser.h"

int main() {

  Person real = make_file("add_age.bin");

  Parser parser("add_age.bin");

  parser.print_difference(real);
  return 0;
}