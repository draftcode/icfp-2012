#include "game.hpp"
#include <iostream>

namespace game {
using std::cout;
using std::endl;
using std::vector;
using std::tr1::shared_ptr;

int Game::board_x = 0;
int Game::board_y = 0;
int Game::board_lambdas = 0;
vector<string> Game::orig_map;

static const int dx[] = { 0, 0, 1, -1, 0, 0 };
static const int dy[] = { 1, -1, 0, 0, 0, 0 };
static const char command_str[][2] = { "D", "U", "R", "L", "W", "A" };

void Game::map_update(shared_ptr< vector<B> > &upd_upper,
                      shared_ptr< vector<B> > &upd_lower,
                      shared_ptr< vector<B> > &upper,
                      shared_ptr< vector<B> > &lower,
                      int y,
                      shared_ptr<Game> &game)
{
#define us(x) (((*upper)[(x)/32] >> (((x)%32)*2)) & 3)
#define ls(x) (((*lower)[(x)/32] >> (((x)%32)*2)) & 3)
#define ensure_updated() \
  upd_upper = shared_ptr< vector<B> >(new vector<B>(*upd_upper)); \
  upd_lower = shared_ptr< vector<B> >(new vector<B>(*upd_lower));
#define upd_up(x, state) \
  (*upd_upper)[(x)/32] &= ~(3ull << (((x)%32)*2)); \
  (*upd_upper)[(x)/32] |= ((0ull | state) << (((x)%32)*2))
#define upd_lo(x, state) \
  (*upd_lower)[(x)/32] &= ~(3ull << (((x)%32)*2)); \
  (*upd_lower)[(x)/32] |= ((0ull | state) << (((x)%32)*2))

  for (int x = 0; x < board_x; x++) {
    if (game->at(x, y) == Empty && us(x) == Rock) {
      // Case 1
      ensure_updated();
      upd_up(x, Empty);
      upd_lo(x, Rock);
    }
    if ((ls(x) == Rock || ls(x) == Lambda) && us(x) == Rock &&
        x < board_x-1 &&
        game->at(x+1, y) == Empty && game->at(x+1, y-1) == Empty) {
      // Case 2, 4
      ensure_updated();
      upd_up(x, Empty);
      upd_lo(x, Rock);
    }
    if (game->at(x, y) == Empty && game->at(x, y-1) == Empty &&
        x < board_x-1 &&
        ls(x+1) == Rock && us(x+1) == Rock &&
        (x == board_x-2 ||
         game->at(x+2, y) != Empty ||
         game->at(x+2, y-1) != Empty)) {
      // Case 3
      ensure_updated();
      upd_up(x+1, Empty);
      upd_lo(x, Rock);
    }
  }
}

inline bool valid_addr(int x, int y) {
  return !(x < 0 || y < 0 || x >= Game::board_x || y >= Game::board_y);
}

bool Game::valid_check(int dir) const
{
  int nx = robot_x + dx[dir];
  int ny = robot_y + dy[dir];
  if (!valid_addr(nx, ny)) return false;

  {
    char s = at(nx, ny);
    if (s == Empty || s == Earth || s == Lambda || s == OpenGate || s == Robot) {
    } else if ((dir == Right || dir == Left) && s == Rock) {
      int nnx = nx + dx[dir];
      int nny = ny + dy[dir];
      if (!valid_addr(nnx, nny)) return false;
      char ss = at(nnx, nny);
      if (ss != Empty) return false;
    } else {
      return false;
    }
  }

  {
    // *
    // _
    // R
    if (valid_addr(nx, ny-2) && at(nx, ny-1) == Empty && at(nx, ny-2) == Rock)
      return false;
  }

  {
    // *_  *_
    // *_  \_
    //  R   R
    if (valid_addr(nx-1, ny-2)) {
      if (at(nx-1, ny-2) == Rock && at(nx, ny-2) == Empty &&
          at(nx, ny-1) == Empty) {
        char s = at(nx-1, ny-1);
        if (s == Rock || s == Lambda) {
          return false;
        }
      }
    }
  }

  {
    // _*@
    // _*@
    // R
    if (valid_addr(nx+1, ny-2)) {
      if (at(nx, ny-1) == Empty && at(nx, ny-2) == Empty &&
          at(nx+1, ny-1) == Rock && at(nx+1, ny-2) == Rock) {
        if (!valid_addr(nx+2, ny-2)) return false;

        if (at(nx+2, ny-1) != Empty || at(nx+2, ny-2) != Empty)
          return false;
      }
    }
  }

  return true;
}

void Game::visualize() const {
  cout << "==============================" << endl;
  for (int y = 0; y < board_y; y++) {
    for (int x = 0; x < board_x; x++) {

      char s = at(x, y);
      switch (s) {
        case Empty: cout << ' '; break;
        case Earth: cout << '.'; break;
        case Rock:  cout << '*'; break;
        case Lambda: cout << '\\'; break;
        case Wall:  cout << '#'; break;
        case OpenGate: cout << 'O'; break;
        case CloseGate: cout << 'L'; break;
        case Robot: cout << 'R'; break;
        default: break;
      }
    }
    cout << endl;
  }
  cout << "Point: " << point() << endl;
  cout << "Command: " << command << endl;
}


shared_ptr<Game> Game::move(int dir)
{
  if (!valid_check(dir)) return shared_ptr<Game>();

  int nx = robot_x + dx[dir];
  int ny = robot_y + dy[dir];

  if (dir == Abort) {
    shared_ptr<Game> ret(new Game(*this, AbortState));
    ret->robot_x = nx;
    ret->robot_y = ny;
    ret->command += command_str[dir];
    return ret;
  } else if (at(nx, ny) == OpenGate) {
    shared_ptr<Game> ret(new Game(*this, WinningState));
    ret->robot_x = nx;
    ret->robot_y = ny;
    ret->command += command_str[dir];
    return ret;
  }

  shared_ptr<Game> ret(new Game(*this));
  ret->robot_x = nx;
  ret->robot_y = ny;
  ret->command += command_str[dir];

  bool updated = false;
  if ((dir == Left || dir == Right) &&
      at(nx, ny) == Rock) {
    if (!updated) {
      ret->map[ny] = shared_ptr< vector<B> >(new vector<B>(*ret->map[ny]));
      updated = true;
    }
    (*ret->map[ny])[(nx)/32] &= ~(3ull << (((nx)%32)*2));
    (*ret->map[ny])[(nx+dx[dir])/32] &= ~(3ull << (((nx+dx[dir])%32)*2));
    (*ret->map[ny])[(nx+dx[dir])/32] |= ((0ull | Rock) << (((nx+dx[dir])%32)*2));
  }
  if (at(nx, ny) == Lambda) {
    if (!updated) {
      ret->map[ny] = shared_ptr< vector<B> >(new vector<B>(*ret->map[ny]));
      updated = true;
    }
    ret->lambdas++;
    (*ret->map[ny])[(nx)/32] &= ~(3ull << (((nx)%32)*2));
  }
  if (at(nx, ny) == Earth) {
    if (!updated) {
      ret->map[ny] = shared_ptr< vector<B> >(new vector<B>(*ret->map[ny]));
      updated = true;
    }
    (*ret->map[ny])[(nx)/32] &= ~(3ull << (((nx)%32)*2));
  }

  for (int i = board_y-1; i > 0; i--) {
    map_update(ret->map[i-1], ret->map[i], map[i-1], map[i],
               i, ret);
  }

  return ret;
}

};

