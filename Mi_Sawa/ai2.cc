#include "game.cc"

volatile bool sigint_received = false;

typedef size_t memo_key_type;
typedef result memo_value_type;
typedef boost::unordered_map<memo_key_type, memo_value_type> memo_type;
memo_type memo;

memo_key_type key(const grid& gr, int d) /*{{{*/
{
    vector<string>& vv = const_cast<vector<string>&>(gr.v);
    const char orig = vv[gr.robot.y][gr.robot.x];
    vv[gr.robot.y][gr.robot.x] = 'R';

    memo_key_type seed = 0;
    boost::hash_combine(seed, gr.v);
    boost::hash_combine(seed, d);
    vv[gr.robot.y][gr.robot.x] = orig;
    return seed;
}/*}}}*/

class heuristic_algorithm{/*{{{*/
    private:
    template<typename H, typename Cond>
    static pair<grid, result> a_star(const grid& gr, H h, Cond c)/*{{{*/
    {
        priority_queue<pair<int, pair<grid, result> > > pq;
        pq.push(make_pair(-h(gr, result()), make_pair(gr, result())));
        while (!pq.empty()) {
            result res = pq.top().second.second;
            grid crr = pq.top().second.first;
            pq.pop();
            result res2;
            grid gr2;
            for (int i = 0; i < ABORT; i++) {
                const move_type m = static_cast<move_type>(i);
                res2 = res;
                gr2 = crr;
                const int t = gr2.move(m);
                res2.append(0, m);
                if (t == INVALID_MOVE || t == NO_DIFFERENCE) {
                    continue;
                }
                if (!gr2.losing) {
                    if(c(gr2, res2)) return make_pair(gr2, res2);
                    pq.push(make_pair(-h(gr2, res2), make_pair(gr2, res2)));
                }
            }
        }
    }/*}}}*/

    class drop_heuristic/*{{{*/
    {
        private:
        const grid gr;
        const pos object;
        public:
        drop_heuristic(grid gr, pos obj): gr(gr), object(obj){}
        int operator() (const grid& g, result r)
        {
            int res = 0;
            set<pos> memo;
            queue<pos> q;
            q.push(object);
            while (!q.empty()) {
                pos p = q.front();
                memo.insert(p);
                q.pop();
                if (gr.v[p.y][p.x] == '*' && g.v[p.y][p.x] != '*') {
                    res += 100 / object.dist(p);
                }else if (g.v[p.y][p.x] == '*' && g.v[p.y-1][p.x]  == ' '){
                    res += 50 / object.dist(p);
                }else if (g.v[p.y][p.x] != gr.v[p.y][p.x]){
                    res += 10 / object.dist(p);
                }
                if (gr.v[p.y][p.x] != '#' && object.dist(p) < 5) {
                    for (int i = 0; i < ABORT; i++) {
                        const move_type m = static_cast<move_type>(i);
                        if(!memo.count(p+m)) q.push(p + m);
                    }
                }
            }
            return res;
        }
    };/*}}}*/

    class drop_judge/*{{{*/
    {
        private:
        const grid gr;
        const pos object;
        public:
        drop_judge(grid gr, pos obj): gr(gr), object(obj){}
        bool operator() (const grid& gr, result r)
        {
            return gr.v[object.y][object.x] != '*' || r.move.size() > 10;
        }
    };/*}}}*/

    public:
    static result try_drop_rock(const grid& gr, pos obj)/*{{{*/
    {
        pair<grid, result> res = a_star(gr, drop_heuristic(gr, obj), drop_judge(gr, obj));
        if (res.first.v[obj.y][obj.x] != '*') return res.second;
        return result();
    }/*}}}*/

    static result go_to(const grid& gr, pos crr)/*{{{*/
    {
    }/*}}}*/
};/*}}}*/

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
        for(int i = 0; i < (r.move.size()+2)/3; ++i){
            oss << r.move[i];
            if (r.move[i] == ABORT) {
                total += 25 * gr.collected_lambda;
                goto END;
            }
            const int t = gr.move(r.move[i]);
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
    }
END:
    cout << "Total score: " << total << endl;
    return make_pair(total, oss.str() + "A");
}/*}}}*/


/* vim: set fdm=marker:*/
