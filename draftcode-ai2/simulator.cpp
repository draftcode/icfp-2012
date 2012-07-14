#include <iostream>
#include "game.hpp"
using namespace std;
using game::Game;
typedef tr1::shared_ptr<Game> spG;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    return -1;
  }
  spG game(new Game(cin));
  game->visualize();

  for (int i = 0; argv[1][i]; i++) {
    switch (argv[1][i]) {
      case 'D':
        game = game->move(Game::Down);
        break;
      case 'U':
        game = game->move(Game::Up);
        break;
      case 'R':
        game = game->move(Game::Right);
        break;
      case 'L':
        game = game->move(Game::Left);
        break;
      case 'W':
        game = game->move(Game::Wait);
        break;
      case 'A':
        game = game->move(Game::Abort);
        game->visualize(cout);
        return 0;
    }
    if (game == NULL) {
      cout << "Invalid Move :" << argv[1][i] << endl;
      return 1;
    }
    game->visualize(cout);
    if (game->is_end()) break;
  }
  return 0;
}

