#include<cstdio>
#include<cstdlib>
#include<cmath>
#include<climits>
#include<iostream>
#include<algorithm>
#include<complex>
#include<string>
#include<sstream>
#include<vector>
#include<stack>
#include<queue>
#include<set>
#include<utility>
#include<map>

using namespace std;

typedef long long ll;
typedef pair<int,int> PII;

#define FOR(i,a,b) for(int i=(a);i<(b);i++)
#define REP(i,n) FOR(i,0,n)
#define MP make_pair

int width,height;

void printMap(vector<string> &map){
  REP(i,map.size()){  
    cout<<map[i]<<endl;
  }
  cout<<endl;
}

vector<string> update(vector<string> &map){
  vector<string> newMap;

  REP(y,height)
    REP(x,width){
      newMap.push_back(map[y]);
      if(map[y][x] == '*'){
        if (map[y+1][x] == ' '){
          newMap[y][x] = ' ';
          newMap[y+1][x] = '*';
        }
        else if(map[y+1][x] == '*'){
          if(map[y][x+1] == ' ' && map[y+1][x+1] == ' '){
            newMap[y][x] = ' ';
            newMap[y+1][x+1] = '*';
          }
          else if(map[y][x-1] == ' ' && map[y+1][x-1] == ' '){
            newMap[y][x] = ' ';
            newMap[y+1][x-1] = '*';
          }
        }
        else if(map[y+1][x] == '\\' && map[y][x+1] == ' ' && map[y+1][x+1] == ' '){
          newMap[y][x] = ' ';
          newMap[y+1][x+1] = '*';
        }
      }
    }

  return newMap;
}

int main(int argc,char *argv[]){

  vector<string> init_map;
  string line;
  while(cin>>line){
    init_map.push_back(line);
  }

  vector<PII> robotHistory;
  vector<vector<string> > history;

  width = init_map[0].length();
  height = init_map.size();

  PII robot;

  REP(y,height)
    REP(x,width){
      if(init_map[y][x] == 'R'){
        robot = PII(x,y);
      }
    }

  robotHistory.push_back(robot);
  history.push_back(init_map);
  vector<char> orders;
  int cursor = 0;

  while(cin>>line){

    if(line == "") break;

    if (line == "<"){
      if (cursor > 0){
        ++cursor;
        cout<<cursor<<"/"<<(history.size() - 1)<<endl;
        printMap(history[cursor]);
        --cursor;
        printMap(history[cursor]);
      }
    }
    else if(line == ">"){
      if(cursor < history.size() - 1){
        ++cursor;
        cout<<cursor<<"/"<<(history.size() - 1)<<endl;
        printMap(history[cursor]);
      }
    }
    else if(line == "<<"){
      cursor = 0;
      cout<<cursor<<"/"<<(history.size() - 1)<<endl;
      printMap(history[cursor]);
    }
    else if(line == ">>"){
      cursor = history.size() - 1;
      cout<<cursor<<"/"<<(history.size() - 1)<<endl;
      printMap(history[cursor]);
    }
    else{
      REP(i,line.length()){
        orders.push_back(line[i]);
      }
      line = "";
      cursor = history.size() - 1;
      while(cursor < line.length()){
        vector<string> map = history[cursor];
        PII robot = robotHistory[cursor];
        map[robot.second][robot.first] = 'R';
        switch(line[cursor]){
          case 'L':
            if(0 < robot.first){
              --robot.first;
            }
            break;
          case 'R':
            if (robot.first < width - 1){
              ++robot.first;
            }
            break;
          case 'U':
            if (0 < robot.second){
              --robot.second;
            }
            break;
          case 'D':
            if (robot.second < height - 1){
              ++robot.second;
            }
            break;
          case 'W':
            break;
        }
        cursor++;
        map[robot.second][robot.first] = 'R';
        robotHistory.push_back(robot);
        history.push_back(update(map));

        cout<<cursor<<"/"<<(history.size() - 1)<<endl;
        printMap(history[cursor]);
      }

    }


  }

  return 0;
}

