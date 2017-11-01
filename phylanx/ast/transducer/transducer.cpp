#include "transducer.hpp"
#include <iostream>
#include <tuple>
#include <map>
#include <functional>

// function that mutates the input value
// to another value of interest
//
void default_logic(size_t & s) {
  s = 42;
}
 
int main(int argc, char ** argv) {

  std::function<void (size_t &)> logic(&default_logic);

  transition_table< size_t, std::function<void (size_t &)> > table;
  table.add(0,1, 2, logic);

  transducer< size_t, std::function<void (size_t &)> > t(table);

  size_t i = 2;

  std::cout << "value of input symbol is now " << i << std::endl;
  std::cout << "\texpected " << 1 << "\tgot: " << t.consume(i) << std::endl;
  std::cout << "value of input symbol is now " << i << std::endl;
  std::cout << "\texpected " << 0 << "\tgot: " << t.consume(i) << std::endl;
  std::cout << "value of input symbol is now " << i << std::endl;

}
