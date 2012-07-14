#include "game.hpp"

namespace game {
using std::cout;
using std::endl;
using std::vector;
using std::endl;
using std::tr1::shared_ptr;
typedef shared_ptr<Game> spG;

// Field constants {{{
size_t Game::board_x = 0;
size_t Game::board_y = 0;
size_t Game::board_lambdas = 0;
size_t Game::water = 0;
size_t Game::flooding = 0;
size_t Game::waterproof = 10;
size_t Game::goal_x = 0;
size_t Game::goal_y = 0;
// }}}

namespace { // Auxiliaty objects {{{
const int dx[] = { 0, 0, 1, -1, 0, 0 };
const int dy[] = { -1, 1, 0, 0, 0, 0 };
const char command_str[][2] = { "D", "U", "R", "L", "W", "A" };

inline bool valid_addr(int x, int y) {
  return !(x < 0 || y < 0 || x >= Game::board_x || y >= Game::board_y);
}

void map_update(vector<string> &nmap,
                const int nx, const int ny)
{
  // !!! Empty cells can be Robot !!!
  // !!! Check both Empty and not Robot !!!
  for (int y = 0; y < Game::board_y; y++) {
    for (int x = 0; x < Game::board_x; x++) {
      // Pattern 1
      if (y+1 < Game::board_y &&
          nmap[y+1][x] == Game::Rock && nmap[y][x] == Game::Empty && !(y == ny && x == nx)) {
        nmap[y+1][x] = Game::Empty; nmap[y][x] = Game::Rock;
      }
      // Pattern 2, 4
      if (y+1 < Game::board_y && x+1 < Game::board_x &&
          nmap[y+1][x] == Game::Rock &&
          (nmap[y][x] == Game::Rock || nmap[y][x] == Game::Lambda) &&
          nmap[y+1][x+1] == Game::Empty && !(y+1 == ny && x+1 == nx) &&
          nmap[y][x+1] == Game::Empty && !(y == ny && x+1 == nx)) {
        nmap[y+1][x] = Game::Empty; nmap[y][x+1] = Game::Rock;
      }
      // Pattern 3
      if (y+2 < Game::board_y && x+1 < Game::board_x &&
          nmap[y+1][x] == Game::Empty && nmap[y][x] == Game::Empty &&
          nmap[y+1][x+1] == Game::Rock && nmap[y][x+1] == Game::Rock &&
          (x+2 < Game::board_x ||
           (nmap[y+1][x+2] != Game::Empty || (y+1 == ny && x+2 == nx)) ||
           (nmap[y][x+2] != Game::Empty || (y == ny && x+2 == nx)))) {
        nmap[y+1][x+1] = Game::Empty; nmap[y][x] = Game::Rock;
      }
    }
  }
}
}; // }}}

// ================ public ===================

spG Game::move(Action act) const // {{{
{
  if (!valid_check(act)) return spG();
  vector<string> nmap(map);
  int nx = robot_x + dx[act];
  int ny = robot_y + dy[act];
  int nl = lambdas;
  int nhp = hp;
  string nc(command + command_str[act]);

  {
    // 2. Push rocks
    if ((act == Right || act == Left) && at(nx, ny) == Rock) {
      // We have already checked this move is valid.
      nmap[ny][nx] = Empty;
      nmap[ny][nx+dx[act]] = Rock;
    }

    // 2. Capture lambda
    if (at(nx, ny) == Lambda) {
      nmap[ny][nx] = Empty;
      nl++;
    }

    // 2. Erase Earth
    if (at(nx, ny) == Earth) {
      nmap[ny][nx] = Empty;
    }
  }

  // 3. HP calculation
  // {
  //   int water_level = water + (command.size() / flooding);
  //   if (ny > water_level)
  //     nhp = waterproof;
  //   else
  //     nhp--;
  // }

  // 4. Map update
  map_update(nmap, nx, ny);

  // 5. Water raise
  // We have nothing to do because water level is calculated based on the number
  // of turns;

  // 6. Winning condition check
  if (at(nx, ny) == OpenGate) {
    return spG(new Game(nmap, nx, ny, nl, nc, nhp, WinState));
  }

  // 7. Abort condition check
  if (act == Abort) {
    return spG(new Game(nmap, nx, ny, nl, nc, nhp, AbortState));
  }

  // 8. Crash condition check
  // We already checked this in valid_check.

  // 9. Drown condition check
  // if (nhp < 0) {
  //   return spG(new Game(nmap, nx, ny, nl, nc, nhp, DrownState));
  // }

  // 10. Add a turn.
  // We included this in the point method. (i.e. abort)

  return spG(new Game(nmap, nx, ny, nl, nc, nhp));
} // }}}
void Game::visualize(std::ostream &os) const { // {{{
  os << "==============================" << endl;
  for (int y = board_y-1; y >= 0; y--) {
    for (int x = 0; x < board_x; x++) {
      os << at(x, y);
    }
    os << endl;
  }
  os << "Point: " << point() << endl;
  os << "Command: " << command << endl;
  if (end_state != NotEnd) {
    if (end_state == WinState) {
      os << "State: Win" << endl;
    } else if (end_state == AbortState) {
      os << "State: Abort" << endl;
    } else if (end_state == DrownState) {
      os << "State: Drown" << endl;
    }
  }
} // }}}

// ================ private ===================

bool Game::valid_check(Action act) const // {{{
{
  if (is_end()) return false;

  int nx = robot_x + dx[act];
  int ny = robot_y + dy[act];
  // This is same as WAIT. We will ignore this.
  if (!valid_addr(nx, ny)) return false;

  {
    // Normal valid check
    char s = at(nx, ny);
    if (s == Empty || s == Earth || s == Lambda || s == OpenGate || s == Robot) {
    } else if ((act == Right || act == Left) && s == Rock) {
      int nnx = nx + dx[act];
      int nny = ny + dy[act];
      if (!valid_addr(nnx, nny)) return false;
      char ss = at(nnx, nny);
      if (ss != Empty) return false;
    } else {
      return false;
    }
  }

  // We will check early death state check.
  if (act != Abort) {
    {
      // *
      // _
      // R
      if (valid_addr(nx, ny+2) &&
          (at(nx, ny+1) == Empty || at(nx, ny+1) == Robot) &&
          at(nx, ny+2) == Rock)
        return false;
    }

    {
      // *_  *_
      // *_  \_
      //  R   R
      if (valid_addr(nx-1, ny+2) &&
          at(nx-1, ny+2) == Rock &&
          at(nx, ny+2) == Empty &&
          (at(nx, ny+1) == Empty || at(nx, ny+1) == Robot) &&
          (at(nx-1, ny+1) == Rock || at(nx-1, ny+1) == Lambda)) {
        return false;
      }
    }

    {
      // _*@
      // _*@
      // R
      if (valid_addr(nx+1, ny+2) &&
          (at(nx, ny+1) == Empty || at(nx, ny+1) == Robot) &&
          at(nx, ny+2) == Empty &&
          at(nx+1, ny+1) == Rock &&
          at(nx+1, ny+2) == Rock) {
        if (!valid_addr(nx+2, ny+2)) return false;

        if (at(nx+2, ny+1) != Empty || at(nx+2, ny+2) != Empty)
          return false;
      }
    }
  }

  return true;
} // }}}

};

