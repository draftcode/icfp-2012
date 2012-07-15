#ifndef __ICFP2012_GAME
#define __ICFP2012_GAME

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <vector>
#include <string>
#include <queue>
#include <iostream>

namespace choro3 {

  using namespace std;

  enum Cell { EMPTY, EARTH, WALL, LAMBDA, ROBOT, ROCK, LIFT };
  const char ICON[] = " .#\\R*L";

  Cell which(char c) {
    for(int i = 0; i < 7; i++) {
      if(c == ICON[i]) return (Cell)i;
    }
    return (Cell)7;
  }

  struct Pos {
    int x, y;
    Pos(int xx, int yy): x(xx), y(yy) {}
    Pos(): x(-1), y(-1) {}
    bool operator ==(const Pos& rhs) {
      return x == rhs.x && y == rhs.y;
    }
    bool operator !=(const Pos& rhs) {
      return !(*this == rhs);
    }
  };

  class Game {

  public:

    int height, width;
    int step;
    int lambda;
    vector<int> map[2];
    Pos pos;
    int score, got;
    
    bool got_lambda;
    int move_rock_count;

    char __last_op;
    bool __on_lift;

    Game(vector<string>& m):
      height(m.size()), step(0), lambda(0), pos(-1, -1), score(0), got(0), got_lambda(false), move_rock_count(0), __on_lift(false) {

      width = 0;
      for(int i = 0; i < height; i++) {
        width = max(width, (int)m[i].size());
      }

      map[0] = vector<int>(height * width / 8 + 1, 0);

      for(int i = 0; i < height; i++) {
      // for(int i = height - 1; i >= height; i--) {
        for(int j = 0; j < (int)m[i].size(); j++) {
          int div = (i * width + j) / 8;
          int mod = (i * width + j) % 8;
          map[0][div] |= (which(m[i][j]) << (mod * 4));
          if(m[i][j] == 'R') {
            pos = Pos(j, height - 1 - i);
          } else if(m[i][j] == '\\') {
            lambda++;
          }
        }
      }

      map[1] = vector<int>(map[0]);      
    }

    inline Cell at(int x, int y, int s) const {
      if(x < 0 || y < 0 || x >= width || y >= height) {
          cerr << "Warning: access (" << x << ", " << y << ")" << endl;
        }
      if(x < 0 || y < 0 || x >= width || y >= height) return WALL;
      y = height - 1 - y;
      int div = (y * width + x) / 8;
      int mod = (y * width + x) % 8;
      return (Cell)((map[s % 2][div] >> (mod * 4)) & 0x0f);
    }

    inline Cell set(int x, int y, int s, Cell val) {      
      assert(!(x < 0 || y < 0 || x >= width || y >= height));
      y = height - 1 - y;
      int div = (y * width + x) / 8;
      int mod = (y * width + x) % 8;
      map[s % 2][div] &= ~(0x0f << (mod * 4));
      map[s % 2][div] |= (val << (mod * 4));
      return val;
    }        

    void print(int st=-1) {
        if(st < 0) st = step;
        printf("height: %d width: %d\n", height, width);
      printf("step: %d\nMine robot: (%d, %d)\n", st, pos.x, pos.y);
      printf("score: %d (got %d, remain %d, total %d)\n", score, got, lambda - got, lambda);
      printf("---------------------------------------\n");
      for(int i = height - 1; i >= 0; i--) {
        for(int j = 0; j < width; j++) {
          if(at(j, i, st) == LIFT && got == lambda) putchar('O');
          else putchar(ICON[at(j, i, st)]);
        }
        putchar('\n');
      }
    }

    bool is_move(int x, int y, int st) {
      assert(at(x, y, st) == ROCK);
      if(at(x, y - 1, st) == EMPTY) return true;
      return false;
    }

    bool valid_move(char op) {

      const int dx[] = {1, 0,-1, 0, 0};
      const int dy[] = {0, 1, 0,-1, 0};
      int dir = (op == 'U' ? 1 : op == 'D' ? 3 : -1);
      dir = (op == 'R' ? 0 : op == 'L' ? 2 : (op == 'W' ? 4 : dir));
      assert(dir >= 0);

      int nx = pos.x + dx[dir], ny = pos.y + dy[dir];

      // if(nx < 0 || ny < 0 || nx >= width || ny >= height) return false;

      if(at(nx, ny, step) == ROCK) {
        return (dir == 0 || dir == 2) && at(nx + dx[dir], ny + dy[dir], step) == EMPTY;
      } else {
        return !(at(nx, ny, step) == WALL || (at(nx, ny, step) == LIFT && got != lambda));
      }
    }

    Pos __next_pos(char op) {
      const int dx[] = {1, 0,-1, 0, 0};
      const int dy[] = {0, 1, 0,-1, 0};
      int dir = (op == 'U' ? 1 : op == 'D' ? 3 : -1);
      dir = (op == 'R' ? 0 : op == 'L' ? 2 : (op == 'W' ? 4 : dir));
      assert(dir >= 0);
      int nx = pos.x + dx[dir], ny = pos.y + dy[dir];
      return Pos(nx, ny);
    }

    bool move(char op) {

      if(__on_lift) {
        cerr << "the game is already over." << endl;
        assert(false);
      }

      // step++;

      __last_op = op;

      if(op == 'A') {
        score += 25 * got;
        return true;
      }

      if(!valid_move(op)) return false;

      Pos np = __next_pos(op);
      int diff_x = np.x - pos.x, diff_y = np.y - pos.y;

      if(at(np.x, np.y, step) == ROCK) {
        set(np.x + diff_x, np.y + diff_y, step, ROCK);
      } else if(at(np.x, np.y, step) == LAMBDA) {
        got++; score += 25;
        got_lambda = true;
      } else {
        got_lambda = false;
      }

      set(pos.x, pos.y, step, __on_lift ? LIFT : EMPTY);

      if(at(np.x, np.y, step) == LIFT) {
        score += 50 * got;
        __on_lift = true;
      } else {
        __on_lift = false;
      }
      set(np.x, np.y, step, ROBOT);

      pos = np;
      score--;

      return true;
    }

    void update() {

      map[(step + 1) % 2] = map[step % 2];

      int mvrock = 0;

      for(int i = 0; i < width; i++) {
        for(int j = 0; j < height; j++) {

          if(at(i, j, step) == ROCK) {
            // 1.
            if(at(i, j - 1, step) == EMPTY) {
              // printf("hello-1\n");
              set(i, j, step + 1, EMPTY);
              set(i, j - 1, step + 1, ROCK);
              mvrock++;
            }
            // 2.
            else if(at(i, j - 1, step) == ROCK && 
                    (at(i + 1, j, step) == EMPTY &&
                     at(i + 1, j - 1, step) == EMPTY)) {
              // printf("hello-2\n");
              set(i, j, step + 1, EMPTY);
              set(i + 1, j - 1, step + 1, ROCK);
              mvrock++;
            }
            // 3.
            else if(at(i, j - 1, step) == ROCK && 
                    (at(i - 1, j, step) == EMPTY &&
                     at(i - 1, j - 1, step) == EMPTY)) {
              // printf("hello-3\n");
              set(i, j, step + 1, EMPTY);
              set(i - 1, j - 1, step + 1, ROCK);
              mvrock++;
            }
            // 4.
            else if(at(i, j - 1, step) == LAMBDA && 
                    (at(i + 1, j, step) == EMPTY &&
                     at(i + 1, j - 1, step) == EMPTY)) {
              set(i, j, step + 1, EMPTY);
              set(i + 1, j - 1, step + 1, ROCK);
              mvrock++;
            }
          }

          // 5.
          // nothing

          // 6.
          // nothing
        }
      }

      step++;
      move_rock_count = mvrock;
    }

    bool __check_lose() {
      return at(pos.x, pos.y + 1, step) == ROCK &&
        at(pos.x, pos.y + 1, step - 1) != ROCK;
    }

    bool __check_win() {
      return __on_lift;
    }

    int ending() {

      // winning
      if(__check_win()) { 
        return 1;
      }
      // abort
      else if(__last_op == 'A') {
        return 2;
      }
      // losing
      else if(__check_lose()) {
        return 3;
      }

      return 0;
    }

    int go(string op, bool debug=false) {
      for(int i = 0; i < (int)op.length(); i++) {
        move(op[i]);
        update();
        if(debug) print();
        switch(ending()) {
        case 1:
          printf("YOU WIN!\n");
          print();
          break;
        case 2:
          printf("ABORT\n");
          print();
          break;
        case 3:
          printf("YOU LOSE\n");
          print();
          break;          
        default:
          break;
        }
      }

      return 0;
    }

  };

  Game scan() {
    using namespace std;
    string buf;
    vector<string> field;
    while(getline(cin, buf)) field.push_back(buf);
    return Game(field);
  }  
}

#endif
