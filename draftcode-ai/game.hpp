#ifndef GAME_HPP_INCLUDED
#define GAME_HPP_INCLUDED
#include <string>
#include <vector>
#include <tr1/memory>
#include <iostream>

namespace game {

typedef unsigned long long int B;
using std::string;
using std::vector;
using std::tr1::shared_ptr;

class Game {
  vector< shared_ptr<vector<B> > > map;
  int robot_x;
  int robot_y;
  int lambdas;
  string command;
  char end_state;

 public:
  static int board_x;
  static int board_y;
  static int board_lambdas;
  static vector<string> orig_map;

  static const int Down = 0;
  static const int Up = 1;
  static const int Right = 2;
  static const int Left = 3;
  static const int Wait = 4;
  static const int Abort = 5;

  static const char NotEnd = 0;
  static const char AbortState = 1;
  static const char WinningState = 2;
  static const char LosingState = 3;

  static const int Empty = 0;
  static const int Earth = 1;
  static const int Rock = 2;
  static const int Lambda = 3;
  static const int Wall = 4;
  static const int OpenGate = 5;
  static const int CloseGate = 6;
  static const int Robot = 7;

  Game() : map(), robot_x(0), robot_y(0), lambdas(0), command(), end_state(0) {
    board_x = orig_map[0].size();
    board_y = orig_map.size();
    board_lambdas = 0;
    int need_B = board_x / 32;
    if (board_x % 32 != 0) need_B++;

    for (int y = 0; y < board_y; y++) {
      shared_ptr< vector<B> > row(new vector<B>(need_B, 0));
      map.push_back(row);
      for (int x = 0; x < board_x; x++) {
        switch (orig_map[y][x]) {
          case 'R':
            robot_x = x;
            robot_y = y;
            break;
          case '*':
            (*row)[x/32] |= ((0ull | Rock) << ((x%32)*2));
            break;
          case '.':
            (*row)[x/32] |= ((0ull | Earth) << ((x%32)*2));
            break;
          case '\\':
            (*row)[x/32] |= ((0ull | Lambda) << ((x%32)*2));
            board_lambdas++;
            break;
          case 'L':
          case '#':
          case 'O':
          case ' ':
            break;
        }
      }
    }
  }
  Game(const Game &g) : map(g.map), robot_x(g.robot_x), robot_y(g.robot_y),
    lambdas(g.lambdas), command(g.command), end_state(0) {}
  Game(const Game &g, char end_state) : map(g.map), robot_x(g.robot_x),
    robot_y(g.robot_y), lambdas(g.lambdas), command(g.command),
    end_state(end_state) {}

  shared_ptr<Game> move(int dir);
  int point() const {
    switch (end_state) {
      case NotEnd:
        return lambdas * 25 - command.size();
      case AbortState:
        return lambdas * 50 - command.size();
      case WinningState:
        return lambdas * 75 - command.size();
      case LosingState:
        return 0;
    }
    return 0;
  }
  void visualize() const;
  bool is_end() {
    return end_state != 0;
  }

 private:
  bool valid_check(int dir) const;
  inline char at(int x, int y) const {
    if (orig_map[y][x] == '#') return Wall;
    if (orig_map[y][x] == 'L' && lambdas == board_lambdas) return OpenGate;
    if (orig_map[y][x] == 'L' && lambdas != board_lambdas) return CloseGate;
    if (x == robot_x && y == robot_y) return Robot;
    return ((*map[y])[x/32] >> ((x%32)*2)) & 3;
  }
  static void map_update(shared_ptr< vector<B> > &upd_upper,
                         shared_ptr< vector<B> > &upd_lower,
                         shared_ptr< vector<B> > &upper,
                         shared_ptr< vector<B> > &lower,
                         int y,
                         shared_ptr<Game> &g);
};

};

#endif /* end of include guard */
