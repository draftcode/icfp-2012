#ifndef GAME_HPP_INCLUDED
#define GAME_HPP_INCLUDED
#include <string>
#include <vector>
#include <tr1/memory>
#include <algorithm>
#include <iostream>

namespace game {

using std::string;
using std::vector;
using std::tr1::shared_ptr;

class Game {
 public:
  enum Action { // {{{
    Down,
    Up,
    Right,
    Left,
    Wait,
    Abort,
  }; // }}}
  enum EndState { // {{{
    NotEnd,
    WinState,
    AbortState,
    LoseState,
    DrownState,
  }; // }}}
  // Field char {{{
  static const char Empty = ' ';
  static const char Earth = '.';
  static const char Rock = '*';
  static const char Lambda = '\\';
  static const char Wall = '#';
  static const char OpenGate = 'O';
  static const char CloseGate = 'L';
  static const char Robot = 'R';
  // }}}
  // Field constants {{{
  static size_t board_x;
  static size_t board_y;
  static size_t board_lambdas;
  static size_t water;
  static size_t flooding;
  static size_t waterproof;
  static size_t goal_x;
  static size_t goal_y;
  // }}}

 private:
  vector<string> map;
  int robot_x;
  int robot_y;
  int lambdas;
  int hp;
  string command;
  EndState end_state;

 public:
  Game(std::istream &is) // {{{
      : map(), robot_x(0), robot_y(0), lambdas(0), command(), end_state(NotEnd)
  {
    // Read from input.
    board_lambdas = board_x = board_y = water = flooding = 0;
    waterproof = 10;
    while (!is.eof()) {
      string s;
      getline(is, s);
      if (s == "") break;
      s = string(1, Wall) + s;
      map.push_back(s);
      board_x = std::max(board_x, s.size());
    }
    map.push_back(string(board_x, Wall));
    std::reverse(map.begin(), map.end());
    map.push_back(string(board_x, Wall));
    board_y = map.size();

    while (!is.eof()) {
      string s; is >> s;
      if (s == "Water") {
        is >> water;
      } else if (s == "Flooding") {
        is >> flooding;
      } else if (s == "Waterproof") {
        is >> waterproof;
      }
    }
    hp = waterproof;
    std::cerr << hp << std::endl;

    for (int y = 0; y < board_y; y++) {
      for (int x = 0; x < map[y].size(); x++) {
        if (map[y][x] == Robot) {
          robot_x = x;
          robot_y = y;
          map[y][x] = Empty;
        } else if (map[y][x] == Lambda) {
          board_lambdas++;
        } else if (map[y][x] == CloseGate) {
          goal_x = x;
          goal_y = y;
        }
      }
      map[y] += string(board_x - map[y].size(), Empty) + string(1, Wall);
    }
    board_x++;
  } // }}}
  Game(vector<string> map, int x, int y, int l, string c, int hp, EndState s = NotEnd) // {{{
      : map(map), robot_x(x), robot_y(y), lambdas(l), command(c), hp(hp), end_state(s)
  {} // }}}

  shared_ptr<Game> move(Action act) const;
  void visualize(std::ostream &os = std::cerr) const;
  inline int point() const { // {{{
    switch (end_state) {
      case NotEnd:
        return lambdas * 25 - command.size();
      case AbortState:
        return lambdas * 50 - command.size() + 1;
      case WinState:
        return lambdas * 75 - command.size();
      case LoseState:
      case DrownState:
      default:
        return lambdas * 25 - command.size();
    }
  } // }}}
  inline bool is_end() const { return end_state != NotEnd; }
  inline char at(int x, int y) const { // {{{
    if (x == robot_x && y == robot_y) return Robot;
    if (map[y][x] == 'L' && lambdas == board_lambdas) return OpenGate;
    if (map[y][x] == 'L' && lambdas != board_lambdas) return CloseGate;
    return map[y][x];
  } // }}}

 private:
  bool valid_check(Action act) const;
};

};

#endif /* end of include guard */
