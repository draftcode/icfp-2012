#include <iostream>
#include <vector>
#include <string>
#include "game.hpp"
using namespace std;
using game::Game;
using tr1::shared_ptr;

static shared_ptr<Game> read_input(void)
{
  while (!cin.eof()) {
    string s;
    getline(std::cin, s);
    Game::orig_map.push_back(s);
    cin.peek();
  }
  return shared_ptr<Game>(new Game());
}

struct result
{
  int score;
  int move;

  result() {}
  result(int s, int m) : score(s), move(m) {}
  static result end() { return result(0, Game::Abort); }
  bool operator<(const result& r) const { return score < r.score; }
};

ostream& operator<<(ostream& os, const result& r)
{
  return os << "<result score=" << r.score << " move = " << r.move << ">";
}

const int moves[] = {
  Game::Down,
  Game::Up,
  Game::Right,
  Game::Left,
  Game::Wait,
  Game::Abort,
};

result dfs(shared_ptr<Game> game, int depth)
{
  result r = result::end();
  if (depth == 0) {
    return r;
  }

  for (int i = 0; i < 6; i++) {
    shared_ptr<Game> next_game = game->move(moves[i]);
    if (!next_game) continue;
    r = max(r, result(next_game->point(), moves[i]));
    if (!next_game->is_end()) {
      const result u = dfs(next_game, depth-1);
      r = max(r, result(u.score, moves[i]));
    }
  }
  return r;
}

void solve(shared_ptr<Game> game, int max_depth)
{
  while (true) {
    if (game->is_end()) return;

    const result r = dfs(game, max_depth);
    game = game->move(r.move);
    if (game) {
      game->visualize();
      if (game->is_end()) {
        return;
      }
    } else {
      return;
    }
  }
}

int main(void) {
  shared_ptr<Game> game = read_input();
  game->visualize();
  game = game->move(Game::Down);
  game = game->move(Game::Left);
  game->visualize();
  // solve(game, 10);
  // game->visualize();
  return 0;
}

