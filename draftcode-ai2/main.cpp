#include <iostream>
#include <climits>
#include "game.hpp"
using namespace std;
using game::Game;
typedef tr1::shared_ptr<Game> spG;

Game::Action actions[] = {
  Game::Down,
  Game::Up,
  Game::Right,
  Game::Left,
};

void dfs(spG game, size_t depth, vector<spG> &v, int &v_point) {
  if (depth == 0 || game->is_end()) {
    if (game->point() > v_point) {
      v.clear();
      v_point = game->point();
      v.push_back(game);
    } else if (game->point() == v_point) {
      v.push_back(game);
    }
    return;
  }

  for (int i = 0; i < sizeof(actions)/sizeof(actions[0]); i++) {
    spG next_game = game->move(actions[i]);
    if (next_game) {
      if (next_game->point() > v_point) {
        v.clear();
        v_point = next_game->point();
        v.push_back(next_game);
      } else if (next_game->point() == v_point) {
        v.push_back(next_game);
      }
      dfs(next_game, depth-1, v, v_point);
    }
  }
}

spG solve(spG game, size_t depth) {
  vector<spG> v;
  int v_point = INT_MIN;
  for (int i = 0; i < sizeof(actions)/sizeof(actions[0]); i++) {
    spG next_game = game->move(actions[i]);
    if (next_game) {
      dfs(next_game, depth, v, v_point);
    }
  }

  while (v.size() != 0 && !v[0]->is_end()) {
    vector<spG> nextV;
    int v_point = INT_MIN;
    for (size_t i = 0; i < min(depth, v.size()); i++) {
      v[i]->visualize();
      dfs(v[i], depth, nextV, v_point);
    }
    cerr << v_point << endl;
    v = nextV;
  }

  return v[0];
}

int main(void) {
  spG game(new Game(cin));
  game = solve(game, 10);
  if (game == NULL) {
    cout << "Solve returns null" << endl;
  } else {
    game->visualize();
  }
  return 0;
}

