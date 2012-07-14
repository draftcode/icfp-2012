#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <boost/unordered_map.hpp>
using namespace std;
static const int INF = 10000000;
static const int dx[] = {-1, 1, 0, 0, 0, 0};
static const int dy[] = {0, 0, -1, 1, 0, 0};

struct pos
{
  int x, y;
  pos() {}
  pos(int i, int j) : x(i), y(j) {}
  bool operator==(const pos& p) const { return x == p.x && y == p.y; }
  bool operator!=(const pos& p) const { return !(*this == p); }
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
}

enum move_type {
  LEFT,
  RIGHT,
  DOWN,
  UP,
  WAIT,
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
    case ABORT:
      return os << "A";
  }
  throw __LINE__;
}

struct result
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
}

static const int INVALID_MOVE = -1;
static const int NO_DIFFERENCE = -2;

typedef pair<vector<string>, int> memo_key_type;
typedef result memo_value_type;
typedef boost::unordered_map<memo_key_type, memo_value_type> memo_type;
memo_type memo;

struct grid
{
  int H, W;
  pos robot;
  vector<string> v;
  pos lambda_lift;
  bool winning, losing;
  int collected_lambda;
  int water, flooding, waterproof;
  int hp, water_turn;

  grid() {}

  grid(const vector<string>& x, int water_, int flooding_, int waterproof_)
    : water(water_), flooding(flooding_), waterproof(waterproof_), hp(waterproof), water_turn(0)
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
  }

  memo_key_type key(int d) const
  {
    vector<string>& vv = const_cast<vector<string>&>(v);
    const char orig = vv[robot.y][robot.x];
    vv[robot.y][robot.x] = 'R';
    memo_key_type k = make_pair(v, d);
    vv[robot.y][robot.x] = orig;
    return k;
  }

  int move(move_type m)
  {
    const int i = static_cast<int>(m);
    robot.x += dx[i];
    robot.y += dy[i];
    if (robot.x < 0 || robot.y < 0 || robot.x >= W || robot.y >= H) {
      return NO_DIFFERENCE; // This is the same as WAIT.
    } else if (valid(m)) {
      water_rise();

      int score = 0;
      const char orig = v[robot.y][robot.x];
      v[robot.y][robot.x] = ' ';
      if (orig == '\\') {
        score = 25;
        ++collected_lambda;
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
  }

  bool valid(move_type m)
  {
    if (m == LEFT && v[robot.y][robot.x] == '*' && v[robot.y][robot.x-1] == ' ') {
      v[robot.y][robot.x-1] = '*';
      v[robot.y][robot.x] = ' ';
      return true;
    } else if (m == RIGHT && v[robot.y][robot.x] == '*' && v[robot.y][robot.x+1] == ' ') {
      v[robot.y][robot.x+1] = '*';
      v[robot.y][robot.x] = ' ';
      return true;
    } else {
      return
        v[robot.y][robot.x] == ' '
        || v[robot.y][robot.x] == '.'
        || v[robot.y][robot.x] == '\\'
        || v[robot.y][robot.x] == 'O';
    }
  }

  void water_rise()
  {
    if (flooding == 0) {
      return;
    }
    ++water_turn;
    if (water_turn == flooding) {
      ++water;
      water_turn = 0;
    }
  }

  int update()
  {
    vector<string> old(v);
    bool lambda_exists = false;
    int cnt = 0;
    for (int y = 0; y < H; y++) {
      for (int x = 0; x < W; x++) {
        if (old[y][x] == '\\') {
          lambda_exists = true;
        }

        if (old[y][x] != '*') {
          continue;
        }
        if (empty(old[y-1][x], pos(x, y-1))) {
          v[y][x] = ' ';
          v[y-1][x] = '*';
          ++cnt;
          if (robot == pos(x, y-2)) {
            losing = true;
          }
        } else if (old[y-1][x] == '*'
            && empty(old[y][x+1], pos(x+1, y))
            && empty(old[y-1][x+1], pos(x+1, y-1))) {
          v[y][x] = ' ';
          v[y-1][x+1] = '*';
          ++cnt;
          if (robot == pos(x+1, y-2)) {
            losing = true;
          }
        } else if (old[y-1][x] == '*'
            && empty(old[y][x-1], pos(x-1, y))
            && empty(old[y-1][x-1], pos(x-1, y-1))) {
          v[y][x] = ' ';
          v[y-1][x-1] = '*';
          ++cnt;
          if (robot == pos(x-1, y-2)) {
            losing = true;
          }
        }
      }
    }
    if (!lambda_exists) {
      v[lambda_lift.y][lambda_lift.x] = 'O';
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
  }

  bool empty(char c, const pos& p) const
  {
    return c == ' ' && p != robot;
  }

  int estimate() const
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
        ans = min(ans, dist[p.y][p.x]);
      } else if (v[p.y][p.x] == 'O') {
        return dist[p.y][p.x];
      }
      const int d = dist[p.y][p.x];
      for (int i = 0; i < 4; i++) {
        const int x = p.x + dx[i];
        const int y = p.y + dy[i];
        const int dd = d+1;
        if ((v[y][x] == ' ' || v[y][x] == '.' || v[y][x] == '\\' || v[y][x] == 'O')
            && dd < dist[y][x]) {
          dist[y][x] = dd;
          q.push(pos(x, y));
        }
      }
    }
    return ans;
  }
  void show(ostream& os) const
  {
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
  }
};

result dfs(const grid& gr, int depth)
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
  for (int i = 0; i <= WAIT; i++) {
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
  if (r.score == 0) {
    int a = INF;
    for (int i = 0; i <= WAIT; i++) {
      const move_type m = static_cast<move_type>(i);
      g = gr;
      const int t = g.move(m);
      if (t == INVALID_MOVE || t == NO_DIFFERENCE || g.losing) {
        continue;
      }
      const int e = g.estimate();
      if (e < a) {
        a = e;
        r = result(0, m);
      }
    }
  }
  memo.insert(make_pair(gr.key(depth), r));
  return r;
}

string solve(grid gr, int max_depth)
{
  int total = 0;
  ostringstream oss;
  while (true) {
    if (gr.winning) {
      cout << "winning" << endl;
      break;
    } else if (gr.losing) {
      cout << "losing" << endl;
      break;
    }
    --total;
    const result r = dfs(gr, max_depth);
    cout << r << endl;
    oss << r.move;
    if (r.move == ABORT) {
      total += 25 * gr.collected_lambda;
      break;
    }
    total += gr.move(r.move);
    cout << "Current total: " << total << endl;
    gr.show(cout);
  }
  cout << "Total score: " << total << endl;
  return oss.str();
}

void readlines(vector<string>& v, int& water, int& flooding, int& waterproof, istream& is)
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
    int val;
    if (iss >> key >> val) {
      if (key == "Water") {
        water = val;
      } else if (key == "Flooding") {
        flooding = val;
      } else if (key == "Waterproof") {
        waterproof = val;
      } else {
        cerr << "Warning: unknown parameter: " << key << " = " << val << endl;
      }
    } else {
      cerr << "Warning: unparsable: " << s << endl;
    }
  }
}

int main(int argc, char *argv[])
{
  vector<string> v;
  int water = 0, flooding = 0, waterproof = 10;
  if (argc == 1) {
    readlines(v, water, flooding, waterproof, cin);
  } else {
    ifstream ifs(argv[1]);
    readlines(v, water, flooding, waterproof, ifs);
  }
  int max_depth = 10;
  if (argc > 2) {
    istringstream iss(argv[2]);
    iss >> max_depth;
  }
  grid g(v, water, flooding, waterproof);
  cout << solve(g, max_depth) << endl;
  return 0;
}
