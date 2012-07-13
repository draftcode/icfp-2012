#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <algorithm>
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

template <class T>
string to_s(const T& val)
{
  ostringstream oss;
  oss << val;
  return oss.str();
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

struct grid
{
  int H, W;
  pos robot;
  vector<string> v;
  vector<pos> closed_lambdas;
  bool winning, losing;
  int collected_lambda;

  grid() {}

  grid(const vector<string>& x)
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
          closed_lambdas.push_back(pos(j, i));
        } else if (v[i][j] == 'R') {
          v[i][j] = ' ';
          robot = pos(j, i);
        }
      }
    }

    winning = losing = false;
    collected_lambda = 0;
  }

  int move(move_type m)
  {
    const int i = static_cast<int>(m);
    robot.x += dx[i];
    robot.y += dy[i];
    if (valid(m)) {
      int score = 0;
      if (v[robot.y][robot.x] == '\\') {
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
      for (vector<pos>::const_iterator it = closed_lambdas.begin(); it != closed_lambdas.end(); ++it) {
        v[it->y][it->x] = 'O';
      }
      closed_lambdas.clear();
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
  return r;
}

string solve(grid gr, int max_depth)
{
  ostringstream oss;
  while (true) {
    if (gr.winning) {
      cerr << "winning" << endl;
      return oss.str();
    } else if (gr.losing) {
      cerr << "losing" << endl;
      return oss.str();
    }
    const result r = dfs(gr, max_depth);
    cerr << r << endl;
    oss << r.move;
    if (r.move == ABORT) {
      return oss.str();
    }
    gr.move(r.move);
    gr.show(cout);
  }
}

void readlines(vector<string>& v, istream& is)
{
  for (string s; getline(is, s);) {
    v.push_back(s);
  }
}

int main(int argc, char *argv[])
{
  vector<string> v;
  if (argc == 1) {
    readlines(v, cin);
  } else {
    ifstream ifs(argv[1]);
    readlines(v, ifs);
  }
  int max_depth = 10;
  if (argc > 2) {
    istringstream iss(argv[2]);
    iss >> max_depth;
  }
  grid g(v);
  cout << solve(g, max_depth) << endl;
  return 0;
}
