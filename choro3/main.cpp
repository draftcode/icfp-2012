// #include "simulator.cpp"
// #include "route.cpp"
#include "game.hpp"
#include "ai.hpp"

#include <string>
#include <vector>
#include <iostream>

int main() {

  using namespace choro3;
  using namespace std;

  Game g = scan();

  string op = AI().solve(g);
  cout << op << endl;

  for(int i = 0; i < (int)op.length(); i++) {
    g.move(op[i]);
    g.update();
    g.print();
    // printf("status: %d\n", g.ending());
  }

  return 0;
}
