#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <signal.h>
#include <cstdlib>
#include <boost/unordered_map.hpp>
using namespace std;
static const int INF = 10000000;
static const int dx[] = {-1, 1, 0, 0, 0, 0, 0};
static const int dy[] = {0, 0, -1, 1, 0, 0, 0};
static const int dx8[] = {-1, -1, -1, 0, 0, 1, 1, 1};
static const int dy8[] = {-1, 0, 1, -1, 1, -1, 0, 1};

struct pos/*{{{*/
{
  int x, y;
  pos() {}
  pos(int i, int j) : x(i), y(j) {}
  bool operator==(const pos& p) const { return x == p.x && y == p.y; }
  bool operator!=(const pos& p) const { return !(*this == p); }
  bool operator<(const pos &p) const { return y != p.y ? y < p.y : x < p.x; }
};
ostream& operator<<(ostream& os, const pos& p)
{
  return os << "<pos x=" << p.x << ", y=" << p.y << ">";
}
size_t hash_value(const pos& p)
{
  size_t seed = 0;
  boost::hash_combine(seed, p.x);
  boost::hash_combine(seed, p.y);
  return seed;
}/*}}}*/

enum move_type {/*{{{*/
  LEFT,
  RIGHT,
  DOWN,
  UP,
  WAIT,
  SHAVE,
  ABORT,
};
ostream& operator<<(ostream& os, move_type m)
{
  switch (m) {
    case LEFT:
      return os << "L";
    case RIGHT:
      return os << "R";
    case DOWN:
      return os << "D";
    case UP:
      return os << "U";
    case WAIT:
      return os << "W";
    case SHAVE:
      return os << "S";
    case ABORT:
      return os << "A";
  }
  throw __LINE__;
}/*}}}*/

struct result/*{{{*/
{
  int score;
  move_type move;

  result() {}
  result(int s, move_type m) : score(s), move(m) {}
  static result end() { return result(0, ABORT); }
  bool operator<(const result& r) const { return score < r.score; }
};
ostream& operator<<(ostream& os, const result& r)
{
  return os << "<result move=" << r.move << ", score=" << r.score << ">";
}/*}}}*/

static const int INVALID_MOVE = -1;
static const int NO_DIFFERENCE = -2;

typedef pair<vector<string>, int> memo_key_type;
typedef result memo_value_type;
typedef boost::unordered_map<memo_key_type, memo_value_type> memo_type;
memo_type memo;

struct trampoline/*{{{*/
{
  char mark;
  pos from, to;
  trampoline() {}
  trampoline(char m, const pos& f, const pos& t)
    : mark(m), from(f), to(t)
  {}
};
ostream& operator<<(ostream& os, const trampoline& t)
{
  return os << "<trampoline mark=" << t.mark << ", from=" << t.from << ", to=" << t.to << ">";
}/*}}}*/

struct grid/*{{{*/
{
  int H, W;
  pos robot;
  vector<string> v;
  set<pos> cells_to_update;
  pos lambda_lift;
  bool winning, losing;
  int collected_lambda, total_lambda;
  int water, flooding, waterproof;
  int hp, water_turn;
  vector<trampoline> trampolines;
  int growth_rate, beard_turn;
  int razors;

  grid() {}

  grid(const vector<string>& x, int water_, int flooding_, int waterproof_, const vector<pair<char,char> >& trampoline_spec, int growth_rate_, int razors_)/*{{{*/
    : water(water_), flooding(flooding_), waterproof(waterproof_), hp(waterproof), water_turn(0),
      growth_rate(growth_rate_), beard_turn(0), razors(razors_)
  {
    v = x;
    H = v.size();
    W = 0;
    for (int i = 0; i < H; i++) {
      W = max(W, int(v[i].size()));
    }
    for (int i = 0; i < H; i++) {
      v[i].resize(W, ' ');
      v[i] = "#" + v[i] + "#";
    }
    W += 2;
    H += 2;
    string t(W, '#');
    v.push_back(t);
    reverse(v.begin(), v.end());
    v.push_back(t);

    for (int i = 0; i < H; i++) {
      for (int j = 0; j < W; j++) {
        if (v[i][j] == 'L') {
          lambda_lift = pos(j, i);
        } else if (v[i][j] == 'R') {
          v[i][j] = ' ';
          robot = pos(j, i);
        }
      }
    }

    winning = losing = false;
    collected_lambda = 0;
    total_lambda = 0;
    map<char, pos> tramp_info;
    for (int i = 0; i < H; i++) {
      for (int j = 0; j < W; j++) {
        if (v[i][j] == '\\' || v[i][j] == '@') {
          ++total_lambda;
        } else if (is_trampoline(v[i][j]) || is_target(v[i][j])) {
          tramp_info.insert(make_pair(v[i][j], pos(j, i)));
        } else if (v[i][j] == 'W') {
          add_change_cell(pos(j, i));
        }

        if (is_rock_like(v[i][j])) {
          add_change_cell(pos(j, i));
        }
      }
    }
    for (vector<pair<char,char> >::const_iterator it = trampoline_spec.begin(); it != trampoline_spec.end(); ++it) {
      trampoline t(it->first, tramp_info[it->first], tramp_info[it->second]);
      trampolines.push_back(trampoline(it->first, tramp_info[it->first], tramp_info[it->second]));
    }
  }/*}}}*/

  memo_key_type key(int d) const/*{{{*/
  {
    vector<string>& vv = const_cast<vector<string>&>(v);
    const char orig = vv[robot.y][robot.x];
    vv[robot.y][robot.x] = 'R';
    memo_key_type k = make_pair(v, d);
    vv[robot.y][robot.x] = orig;
    return k;
  }/*}}}*/

  int move(move_type m)/*{{{*/
  {
    const int i = static_cast<int>(m);
    // 古い点も変化する点
    add_change_cell(pos(robot.x, robot.y));

    robot.x += dx[i];
    robot.y += dy[i];
    if (robot.x < 0 || robot.y < 0 || robot.x >= W || robot.y >= H) {
      return NO_DIFFERENCE; // This is the same as WAIT.
    } else if (m == SHAVE) {
      if (razors == 0) {
        return INVALID_MOVE;
      } else {
        --razors;
        int cnt = 0;
        for (int i = 0; i < 8; i++) {
          const int x = robot.x + dx8[i];
          const int y = robot.y + dy8[i];
          if (v[y][x] == 'W') {
            v[y][x] = ' ';
            ++cnt;
            add_change_cell(pos(x, y));
          }
        }
        if (cnt == 0) {
          return NO_DIFFERENCE;
        } else {
          water_rise();
          update();
          return 0;
        }
      }
    } else if (valid(m)) {
      add_change_cell(pos(robot.x, robot.y));
      water_rise();

      int score = 0;
      const char orig = v[robot.y][robot.x];
      v[robot.y][robot.x] = ' ';
      if (orig == '\\') {
        score = 25;
        ++collected_lambda;
      } else if (orig == '!') {
        ++razors;
      }
      const int cnt = update();
      if (m == WAIT && cnt == 0) {
        // meaningless wait
        return NO_DIFFERENCE;
      }
      if (v[robot.y][robot.x] == 'O') {
        score = 50 * collected_lambda;
        winning = true;
      }
      v[robot.y][robot.x] = ' ';
      return score;
    } else {
      return INVALID_MOVE;
    }
  }/*}}}*/

  bool valid(move_type m)/*{{{*/
  {
    if (m == LEFT && is_rock_like(v[robot.y][robot.x]) && v[robot.y][robot.x-1] == ' ') {
      v[robot.y][robot.x-1] = v[robot.y][robot.x];
      v[robot.y][robot.x] = ' ';
      return true;
    } else if (m == RIGHT && is_rock_like(v[robot.y][robot.x]) && v[robot.y][robot.x+1] == ' ') {
      v[robot.y][robot.x+1] = v[robot.y][robot.x];
      v[robot.y][robot.x] = ' ';
      return true;
    } else if (is_trampoline(v[robot.y][robot.x])) {
      jump_from(v[robot.y][robot.x]);
      return true;
    } else {
      return
        v[robot.y][robot.x] == ' '
        || v[robot.y][robot.x] == '.'
        || v[robot.y][robot.x] == '\\'
        || v[robot.y][robot.x] == '!'
        || v[robot.y][robot.x] == 'O';
    }
  }/*}}}*/

  static bool is_trampoline(char c) { return 'A' <= c && c <= 'I'; }

  static bool is_target(char c) { return '1' <= c && c <= '9'; }

  static bool is_rock_like(char c) { return c == '*' || c == '@'; }

  trampoline get_trampoline(char t) const/*{{{*/
  {
    trampoline tramp;
    for (vector<trampoline>::const_iterator it = trampolines.begin(); it != trampolines.end(); ++it) {
      if (it->mark == t) {
        return *it;
      }
    }
    throw "Not a trampoline?: " + t;
  }/*}}}*/

  void jump_from(char t)/*{{{*/
  {
    const trampoline tramp = get_trampoline(t);
    robot = tramp.to;
    for (vector<trampoline>::const_iterator it = trampolines.begin(); it != trampolines.end(); ++it) {
      if (it->to == robot) {
        const pos& p = it->from;
        v[p.y][p.x] = ' ';
        add_change_cell(p);
      }
    }
  }/*}}}*/

  void water_rise()/*{{{*/
  {
    if (flooding == 0) {
      return;
    }
    ++water_turn;
    if (water_turn == flooding) {
      ++water;
      water_turn = 0;
    }
  }/*}}}*/

  void add_change_cell(const pos &p)/*{{{*/
  {
    for(int dx = -1; dx <= 1; ++dx) {
      for(int dy = -1; dy <= 1; ++dy) {
        cells_to_update.insert(pos(p.x+dx, p.y+dy));
      }
    }
  }/*}}}*/

  int update()/*{{{*/
  {
    bool beard_growth = false;
    ++beard_turn;
    if (beard_turn == growth_rate) {
      beard_turn = 0;
      beard_growth = true;
    }

    set<pos> old_cells_to_update;
    old_cells_to_update.swap(cells_to_update);
    cells_to_update.clear();
    vector<string> old(v);
    bool lambda_exists = (collected_lambda < total_lambda);
    int cnt = 0;
    for(set<pos>::const_iterator pos_it = old_cells_to_update.begin(); pos_it != old_cells_to_update.end(); ++pos_it) {
      const int x = pos_it->x;
      const int y = pos_it->y;
      /*
      if (old[y][x] == '\\') {
        lambda_exists = true;
      }
      */

      if (is_rock_like(old[y][x])) {
        int xx = -1, yy = -1;
        if (empty(old[y-1][x], pos(x, y-1))) {
          v[y][x] = ' ';
          xx = x;
          yy = y-1;
          ++cnt;
        } else if ((is_rock_like(old[y-1][x]) || old[y-1][x] == '\\')
            && empty(old[y][x+1], pos(x+1, y))
            && empty(old[y-1][x+1], pos(x+1, y-1))) {
          v[y][x] = ' ';
          xx = x+1;
          yy = y-1;
          ++cnt;
        } else if (is_rock_like(old[y-1][x])
            && empty(old[y][x-1], pos(x-1, y))
            && empty(old[y-1][x-1], pos(x-1, y-1))) {
          v[y][x] = ' ';
          xx = x-1;
          yy = y-1;
          ++cnt;
        }
        if (xx != -1 && yy != -1) {
          if (v[yy][xx] == '@' && !empty(old[yy+1][xx], pos(xx, yy-1))) {
            v[yy][xx] = '\\';
          } else {
            v[yy][xx] = old[y][x];
          }
          add_change_cell(pos(x, y));
          add_change_cell(pos(xx, yy));
        }
      } else if (old[y][x] == 'W') {
        if (beard_growth) {
          for (int i = 0; i < 8; i++) {
            const int xx = x + dx8[i];
            const int yy = y + dy8[i];
            if (empty(old[yy][xx], pos(xx, yy))) {
              v[yy][xx] = 'W';
              add_change_cell(pos(xx, yy));
            }
          }
        }
        // beard_growthの時だけチェックすれば十分だが，別管理するのが面倒なので
        // 毎回Wはチェックするようにする．
        add_change_cell(pos(x, y));
      }
    }
    if (!lambda_exists) {
      v[lambda_lift.y][lambda_lift.x] = 'O';
    }
    // 上に岩かλ(Higher order rockが壊れた場合)が落ちてきたら死ぬ．
    if((old[robot.y+1][robot.x] == ' ')
        && (v[robot.y+1][robot.x] == '*' || v[robot.y+1][robot.x] == '\\')) {
      losing = true;
    }

    if (robot.y <= water) {
      --hp;
      if (hp < 0) {
        losing = true;
      }
    } else {
      hp = waterproof;
    }
    return cnt;
  }/*}}}*/

  bool empty(char c, const pos& p) const { return c == ' ' && p != robot; }

  int estimate() const/*{{{*/
  {
    queue<pos> q;
    q.push(robot);
    vector<vector<int> > dist(H, vector<int>(W, INF));
    dist[robot.y][robot.x] = 0;
    int ans = INF;
    while (!q.empty()) {
      const pos p = q.front();
      q.pop();
      if (v[p.y][p.x] == '\\') {
        return dist[p.y][p.x];
      } else if (v[p.y][p.x] == 'O') {
        return dist[p.y][p.x];
      }
      const int d = dist[p.y][p.x];
      if (is_trampoline(v[p.y][p.x])) {
        const trampoline tramp = get_trampoline(v[p.y][p.x]);
        const pos pp = tramp.to;
        const int dd = d;
        if (dd < dist[pp.y][pp.x]) {
          dist[pp.y][pp.x] = dd;
          q.push(pp);
        }
      } else {
        const int dd = d+1;
        for (int i = 0; i < 4; i++) {
          const int x = p.x + dx[i];
          const int y = p.y + dy[i];
          if ((v[y][x] == ' ' || v[y][x] == '.' || v[y][x] == '\\' || v[y][x] == 'O' || is_trampoline(v[y][x]))
              && dd < dist[y][x]) {
            dist[y][x] = dd;
            q.push(pos(x, y));
          }
        }
      }
    }
    return ans;
  }/*}}}*/

  int heuristic() const/*{{{*/
  {
    const int base = W + H;
    int cost = 0;
    vector<vector<int> > dist(H, vector<int>(W, INF));
    queue<pos> q;
    q.push(robot);
    dist[robot.y][robot.x] = 0;
    int dist_sum = 0;
    while (!q.empty()) {
      const pos p = q.front();
      q.pop();
      const int d = dist[p.y][p.x];
      for (int i = 0; i < 4; i++) {
        const int x = p.x + dx[i];
        const int y = p.y + dy[i];
        const int dd = d+1;
        if ((v[y][x] == ' ' || v[y][x] == '.' || v[y][x] == '\\' || v[y][x] == 'O')
            && dd < dist[y][x]) {
          dist[y][x] = dd;
          q.push(pos(x, y));
          if (v[y][x] == '\\') {
            dist_sum += dist[y][x];
          } else if (v[y][x] == 'O') {
            cost += (collected_lambda+base-dist[y][x]) * 50;
          }
        }
      }
    }
    return cost + collected_lambda*25 + 25*(total_lambda-dist_sum);
  }/*}}}*/

  void show(ostream& os) const/*{{{*/
  {
    os << "[Razors = " << razors << ", Growth = " << beard_turn << "/" << growth_rate << "]" << endl;
    for (int i = H-1; i >= 0; i--) {
      for (int j = 0; j < W; j++) {
        if (robot == pos(j, i)) {
          os << 'R';
        } else {
          os << v[i][j];
        }
      }
      if (i == water) {
        os << " ~~~(HP " << hp << ")~~~";
      }
      os << endl;
    }
  }/*}}}*/
};/*}}}*/

struct best_keeper/*{{{*/
{
  string sequence;
  int score;

  best_keeper() : sequence("A"), score(0) {}
  void update(const grid &g, const string &seq)/*{{{*/
  {
    int grid_score = g.collected_lambda * 25 - seq.size();
    if(g.winning) {
      grid_score += g.collected_lambda * 50;
    } else if(g.losing) {
    } else {
      // ここでAbortするものとして点数計算する．
      grid_score += g.collected_lambda * 25;
    }
    if(grid_score > score) {
      score = grid_score;
      sequence = seq;
    }
  } /*}}}*/
} global_best;/*}}}*/

result dfs(const grid& gr, int depth)/*{{{*/
{
  result r = result::end();
  if (depth == 0) {
    return r;
  }
  memo_type::const_iterator it = memo.find(gr.key(depth));
  if (it != memo.end()) {
    return it->second;
  }

  grid g;
  for (int i = 0; i < ABORT; i++) {
    const move_type m = static_cast<move_type>(i);
    g = gr;
    const int t = g.move(m);
    if (t == INVALID_MOVE || t == NO_DIFFERENCE) {
      continue;
    }
    if (g.winning) {
      r = max(r, result(t, m));
    } else if (g.losing) {
    } else {
      const result u = dfs(g, depth-1);
      r = max(r, result(t - 1 + u.score, m));
    }
  }
  memo.insert(make_pair(gr.key(depth), r));
  return r;
}/*}}}*/

void solve(grid gr, int max_depth)/*{{{*/
{
  static const int DAMEPO = -1000;
  int total = 0;
  ostringstream oss;
  int last_total = 0;
  while (true) {
    if (gr.winning) {
      cout << "winning" << endl;
      break;
    } else if (gr.losing) {
      cout << "losing" << endl;
      break;
    }
    result r = dfs(gr, max_depth);
    if (r.score == 0) {
      int a = INF;
      for (int i = 0; i < ABORT; i++) {
        const move_type m = static_cast<move_type>(i);
        grid g = gr;
        const int t = g.move(m);
        if (t == INVALID_MOVE || t == NO_DIFFERENCE || g.losing) {
          continue;
        }
        //const int e = g.heuristic();
        const int e = g.estimate();
        if (e < a) {
          a = e;
          r = result(0, m);
        }
      }
    }
    cout << r << endl;
    oss << r.move;
    if (r.move == ABORT) {
      total += 25 * gr.collected_lambda;
      break;
    }
    const int t = gr.move(r.move);
    --total;
    total += t;
    if (t > 0) {
      last_total = total + 25 * gr.collected_lambda;
      global_best.update(gr, oss.str());
    }
    if (total < DAMEPO) {
      cout << "Rollback & Abort" << endl;
      cout << "Total score: " << last_total << endl;
      return;
    }
    cout << "Current total: " << total << endl;
    gr.show(cout);
  }
  cout << "Total score: " << total << endl;
  return;
}/*}}}*/

void readlines(vector<string>& v, int& water, int& flooding, int& waterproof, vector<pair<char,char> >& trampoline_spec, int& growth_rate, int& razors, istream& is)/*{{{*/
{
  for (string s; getline(is, s);) {
    if (s.empty()) {
      break;
    }
    v.push_back(s);
  }
  for (string s; getline(is, s);) {
    istringstream iss(s);
    string key;
    if (iss >> key) {
      if (key == "Water") {
        iss >> water;
      } else if (key == "Flooding") {
        iss >> flooding;
      } else if (key == "Waterproof") {
        iss >> waterproof;
      } else if (key == "Growth") {
        iss >> growth_rate;
      } else if (key == "Razors") {
        iss >> razors;
      } else if (key == "Trampoline") {
        string from, dummy, to;
        iss >> from >> dummy >> to;
        trampoline_spec.push_back(make_pair(from[0], to[0]));
      } else {
        //cerr << "Warning: unknown parameter: " << key << " = " << val << endl;
      }
    } else {
      //cerr << "Warning: unparsable: " << s << endl;
    }
  }
}/*}}}*/

void sigint_handler(int sig)/*{{{*/
{
  exit(0);
}/*}}}*/

void print_answer()
{
  cout << "Final score: " << global_best.score << endl;
  cout << global_best.sequence << endl;
  _exit(0);
}

int main(int argc, char *argv[])/*{{{*/
{
  signal(SIGINT, sigint_handler);
  atexit(print_answer);

  vector<string> v;
  int water = 0, flooding = 0, waterproof = 10;
  int growth_rate = 25;
  int razors = 0;
  vector<pair<char,char> > trampoline_spec;
  if (argc == 1) {
    readlines(v, water, flooding, waterproof, trampoline_spec, growth_rate, razors, cin);
  } else {
    ifstream ifs(argv[1]);
    readlines(v, water, flooding, waterproof, trampoline_spec, growth_rate, razors, ifs);
  }
  int max_depth = 5;
  if (argc > 2) {
    istringstream iss(argv[2]);
    iss >> max_depth;
  }
  grid g(v, water, flooding, waterproof, trampoline_spec, growth_rate, razors);
  static const int MAX_DEPTH = 50;
  //static const int MAX_DEPTH = 6;
  while (max_depth < MAX_DEPTH) {
    cout << "Solving with max_depth=" << max_depth << endl;
    solve(g, max_depth);
    ++max_depth;
  }
  return 0;
}/*}}}*/

/* vim: set et sw=2 fdm=marker:*/
