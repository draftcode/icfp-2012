#include "game.cc"

volatile bool sigint_received = false;

typedef pair<vector<string>, int> memo_key_type;
typedef result memo_value_type;
typedef boost::unordered_map<memo_key_type, memo_value_type> memo_type;
memo_type memo;

memo_key_type key(const grid& gr, int d) /*{{{*/
{
    vector<string>& vv = const_cast<vector<string>&>(gr.v);
    const char orig = vv[gr.robot.y][gr.robot.x];
    vv[gr.robot.y][gr.robot.x] = 'R';
    memo_key_type k = make_pair(gr.v, d);
    vv[gr.robot.y][gr.robot.x] = orig;
    return k;
}/*}}}*/


result dfs(const grid& gr, int depth)/*{{{*/
{
    result r = result::end();
    if (depth == 0) {
        return r;
    }
    memo_type::const_iterator it = memo.find(key(gr, depth));
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
            const result u = dfs(g, depth-1).prepend(t-1, m);
            r = max(r, u);
        }
    }
    if (r.score == 0) {
        int a = INF;
        for (int i = 0; i < ABORT; i++) {
            const move_type m = static_cast<move_type>(i);
            g = gr;
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
    memo.insert(make_pair(key(gr, depth), r));
    return r;
}/*}}}*/

pair<int, string> solve(grid gr, int max_depth)/*{{{*/
{
    static const int DAMEPO = -1000;
    int total = 0;
    ostringstream oss;
    string last_lambda;
    int last_total = 0;
    while (!sigint_received) {
        if (gr.winning) {
            cout << "winning" << endl;
            break;
        } else if (gr.losing) {
            cout << "losing" << endl;
            break;
        }
        const result r = dfs(gr, max_depth);
        cout << r << endl;
        oss << r.move[0];
        if (r.move[0] == ABORT) {
            total += 25 * gr.collected_lambda;
            break;
        }
        const int t = gr.move(r.move[0]);
        --total;
        total += t;
        if (t > 0) {
            last_lambda = oss.str();
            last_total = total + 25 * gr.collected_lambda;
        }
        if (total < DAMEPO) {
            cout << "Rollback & Abort" << endl;
            cout << "Total score: " << last_total << endl;
            return make_pair(last_total, last_lambda + "A");
        }
        cout << "Current total: " << total << endl;
        gr.show(cout);
    }
    cout << "Total score: " << total << endl;
    return make_pair(total, oss.str() + "A");
}/*}}}*/



