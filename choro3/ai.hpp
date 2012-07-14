#ifndef __ICFP2012_AI
#define __ICFP2012_AI

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <vector>
#include <string>
#include <queue>

#include "game.hpp"

namespace choro3 {

  using namespace std;

  class AI {

    // typedef pair<int, int> PII;
    typedef pair<int, string> Res;
    static const int inf = 1 << 25;

    int __dir(Pos p0, Pos p1) {
      static const int dx[] = {1, 0,-1, 0};
      static const int dy[] = {0, 1, 0,-1};
      int diff_x = p1.x - p0.x, diff_y = p1.y - p0.y;
      for(int i = 0; i < 4; i++) {
        if(dx[i] == diff_x && dy[i] == diff_y) return i;
      }
      return -1;
    }

    // stable: trueで状態を変えない制限のもとで経路探索
    Pos __bfs(Game g, vector<vector<int> >& memo, vector<vector<Pos> >& bt, bool stable=false) {

      static const int dx[] = {1, 0,-1, 0};
      static const int dy[] = {0, 1, 0,-1};
      // static const char ops[] = "RULD";

      memo = vector< vector<int> >(g.height, vector<int>(g.width, inf));
      bt = vector< vector<Pos> >(g.height, vector<Pos>(g.width, Pos(-1, -1)));

      queue<Pos> q;
      q.push(g.pos);
      memo[g.pos.y][g.pos.x] = 0;

      while(true) {
        if(q.empty()) {
          return Pos(-1, -1);
        }

        Pos p = q.front(); q.pop();
        // printf("(%d, %d)\n", p.x, p.y);

        if(g.at(p.x, p.y, g.step) == LAMBDA) { return p; }

        for(int i = 0; i < 4; i++) {
          int nx = p.x + dx[i], ny = p.y + dy[i];
          // validation
          Cell c = g.at(nx, ny, g.step);
          if(c == WALL || (c == ROCK && (i == 1 || i == 3 || ((i == 0 || i == 2) && g.at(nx + dx[i], ny, g.step) != EMPTY)))) continue;
          if(stable && (c == LAMBDA || c == ROCK || g.at(nx, ny + 1, g.step) == ROCK)) continue;
          if(memo[ny][nx] <= memo[p.y][p.x] + 1) continue;
          //
          q.push(Pos(nx, ny));
          memo[ny][nx] = memo[p.y][p.x] + 1;
          bt[ny][nx] = p;
        }
      }
    }

    string nearest_lambda(Game g, bool stable=false) {

      vector< vector<int> > memo;
      vector< vector<Pos> > bt;

      static const char ops[] = "RULD";

      Pos p = __bfs(g, memo, bt, stable);
      if(p == Pos(-1, -1)) return "";
      string res("");
      for(; bt[p.y][p.x] != Pos(-1, -1); p = bt[p.y][p.x]) {
        int dir = __dir(bt[p.y][p.x], p);
        res = ops[dir] + res;
      }

      return res;
    }

    Res __naive_game_dfs(Game g, int N) {

      joutaisuu++;

      if(N == 0) return Res(g.score, "");

      // if(N >= 5) printf("state: %d\n", joutaisuu);

      const char ops[] = "UDRLWA";

      Res opt(-inf, "");

      for(int i = 0; i < 6; i++) {
        if(ops[i] == 'A' && !g.got_lambda) continue;
        Game gg(g);
        gg.move(ops[i]);
        gg.update();
        // 待つ意味がないところ。 1'18" -> 0'18" @ sample7
        if(ops[i] == 'W' && g.move_rock_count == 0) continue;
        // (岩の位置, ロボットの位置)
        Res r = gg.ending() ? Res(gg.score, "") : __naive_game_dfs(gg, N - 1);
        if(opt.first < r.first || opt.first == r.first && opt.second.length() > r.second.length() + 1) {
          opt = r;
          opt.second = ops[i] + opt.second;
        }
      }

      if(N >= 3) {
        // cout << "__naive_game_dfs (N=" << N << "): " << opt.second << " - score: " << opt.first << endl;
      }

      return opt;
    }

    string naive_game(Game g) {
      string res("");
      bool flag = false;
      while(!flag && (int)res.length() <= g.width * g.height) {
        string ops = __naive_game_dfs(g, 8).second;
        // cout << "got: " << ops << " ------------------------------" << endl;
        res += ops[0];
        g.move(ops[0]);
        g.update();
        flag = g.ending() != 0;
      }
      return res;
    }

    string greedy_sp(Game g) {
      string res(""), tmp;
      while(true) {
        tmp = nearest_lambda(g);
        if(tmp == "") return res + 'A';
        // res = tmp + res;
        cerr << "[ " << tmp << " ]" << endl;
        for(int i = 0; i < (int)tmp.length(); i++) {
          if(g.move(tmp[i])) res = res + tmp[i];
          g.update();
          if(g.ending() == 3) return res.substr(0, res.length() - 1) + 'A';
          if(g.ending()) return res;
        }
      }
    }

  public:
    AI(): joutaisuu(0){}
    int joutaisuu;
    string solve(Game g) {
      return greedy_sp(g);
      // return naive_game(g);
    }
  };
}

#endif
