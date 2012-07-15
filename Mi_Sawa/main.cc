#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <signal.h>
#include <boost/unordered_map.hpp>
#include "game.cc"
#include "ai2.cc"


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
    sigint_received = true;
}/*}}}*/

int main(int argc, char *argv[])/*{{{*/
{
    signal(SIGINT, sigint_handler);

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
    pair<int,string> final_answer(0, "A");
    while (!sigint_received) {
        cout << "Solving with max_depth=" << max_depth << endl;
        const pair<int,string> r = solve(g, max_depth);
        if (final_answer.first < r.first) {
            final_answer = r;
        }
        ++max_depth;
    }
    cout << "Final score: " << final_answer.first << endl;
    cout << final_answer.second << endl;
    return 0;
}/*}}}*/


/* vim: set fdm=marker:*/
