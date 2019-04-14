#include "SimSearcher.h"

static char input[80000000];
static vector<unsigned> block3[200000];
static int b3p = 0;
static int ed_mat[256][256];

static int* len_list_switch = nullptr;
static int len_list[200000];
static int term_len_list[200000];

static int _query_length = 0;
static int _query_term_num = 0;
static int ed_th = 0;
static double jaccard_th = 0;


SimSearcher::SimSearcher()
{
    term_list = nullptr;
    qgram_list = nullptr;
    str_list = new char*[200000];
    term_ids = new vector<unsigned>*[200000];
    _mv_ed = 0.0085;
    _mv_jaccard = 0.007;
    _min_term_num = 512;
}

SimSearcher::~SimSearcher()
{
}

inline bool filter_ed(int candidate_length) {
    return abs(candidate_length-_query_length) > ed_th;
}

inline bool filter_jaccard(int term_length) {
    return term_length < _query_term_num * jaccard_th || term_length > _query_term_num / jaccard_th;
}

void SimSearcher::read_dataset(const char* filename) {
    FILE* fp = fopen(filename, "r");
    int num = fread(input, sizeof(char), sizeof(input), fp);
    input[num] = '\0';
    this->str_num = 0;
    for (int i = 0; i < num; ++i) {
        str_list[str_num] = input + i;
        while(input[i] != '\n') ++i;
        input[i] = '\0';
        len_list[str_num] = input + i - str_list[str_num];
        ++str_num;
    }
}

int SimSearcher::createIndex(const char *filename, unsigned q)
{
    this->_q = q;
    read_dataset(filename);
    qgram_list = new InvertedList(q);
    term_list = new InvertedList();
    for (int i = 0; i < str_num; ++i) {
        term_ids[i] = &block3[b3p++];
        qgram_list->insert_qgram(str_list[i], i);
        term_list->insert_term(str_list[i], i, *(term_ids[i]));
        term_len_list[i] = term_ids[i]->size();
        _min_term_num = min(_min_term_num, term_len_list[i]);
    }
    // qgram_list->qgram_output();
    // term_list->term_output();
    return SUCCESS;
}

vector<pair<unsigned,int>> SimSearcher::merge_skip(int tot, const vector<reference_wrapper<vector<int>>>& inv_index, const function<bool(int)>& filter) {
    // assert(tot <= inv_index.size());
    priority_queue<pair<unsigned,int>, vector<pair<unsigned,int>>, greater<pair<unsigned,int>>>  pq;
    vector<vector<int>::iterator> plist;
    for (int i = 0; i < inv_index.size(); ++i) {
        plist.push_back(inv_index[i].get().begin());
        while (plist[i] != inv_index[i].get().end() && filter(len_list_switch[*(plist[i])])) {
            ++plist[i];
        }
        if (plist[i] != inv_index[i].get().end()) {
            pq.emplace(*(plist[i]++), i);
        }
    }
    vector<int> poped;
    vector<pair<unsigned, int>> ret;
    while (pq.size() >= tot) { // if pq.size() < tot, then number of one index can not exceed tot
        auto t = pq.top().first;
        while (!pq.empty() && pq.top().first == t) {
            poped.push_back(pq.top().second);
            pq.pop();
        }
        int cnt = poped.size(); // poped number
        if (cnt >= tot) {
            ret.emplace_back(t, cnt);
            for (int ind : poped) {
                while (plist[ind] != inv_index[ind].get().end() && filter(len_list_switch[*(plist[ind])])) {
                    ++plist[ind];
                }
                if (plist[ind] != inv_index[ind].get().end()) {
                    pq.emplace(*(plist[ind]++), ind);
                }
            }
        } else { // cnt < tot, outer "while" means tot <= pq.size()
            for (int i = 0; i < tot-cnt-1; ++i) {
                poped.push_back(pq.top().second);
                pq.pop();
            }
            auto t1 = pq.top().first;
            for (int ind : poped) {
                plist[ind] = lower_bound(inv_index[ind].get().begin(), inv_index[ind].get().end(), t1);
                // cout << "t1 = " << t1 << " *plist[ind] = " << *plist[ind] << endl;
                while (plist[ind] != inv_index[ind].get().end() && filter(len_list_switch[*(plist[ind])])) {
                    ++plist[ind];
                }
                if (plist[ind] != inv_index[ind].get().end()) {
                    pq.emplace(*(plist[ind]++), ind);
                }
            }
        }
        poped.clear();
    }
    return ret;
}

vector<unsigned> SimSearcher::divide_skip(int tot, vector<reference_wrapper<vector<int>>>& all_list, const function<bool(int)>& filter, double mv) {
    vector<unsigned> ret;
    // long->short after sorting
    sort(   all_list.begin(), 
            all_list.end(), 
            [](const reference_wrapper<vector<int>>& a, 
                const reference_wrapper<vector<int>>& b)->bool {
                    return a.get().size() > b.get().size();
                });
    // Lshort >= 2, Llong = tot/(1+vlogM), if long_num == 0, divide_skip->merge_skip
    int long_num = min((int)floor(tot * 1.0 / (1 + mv * log2(all_list[0].get().size()))), tot-1);
    // cout << "long_num: " << long_num << endl;
    auto id2times = merge_skip( tot - long_num, 
                                vector<reference_wrapper<vector<int>>>(all_list.begin()+long_num, all_list.end()), 
                                filter);
    for (auto& val : id2times) {
        if (val.second >= tot) {
            ret.push_back(val.first);
        } else {
            for (int i = 0; i < long_num; ++i) {
                if(binary_search(all_list[i].get().begin(), all_list[i].get().end(), val.first)) {
                    if(++val.second >= tot) {
                        ret.push_back(val.first);
                        break;
                    }
                }
            }
        }
    }
    return ret;
}

int SimSearcher::searchJaccard(const char* query, double threshold, vector<pair<unsigned, double>>& result)
{
	result.clear();
    len_list_switch = term_len_list;
    vector<unsigned> query_term;
    term_list->get_term(query, query_term, inv_idx);
    jaccard_th = threshold;
    _query_term_num = query_term.size();
    int tot = max(ceil(jaccard_th * _query_term_num), ceil((_query_term_num + _min_term_num) / (1 + 1 / jaccard_th)));
    vector<unsigned> candidates;
    if (tot <= 0) {
        candidates = term_num_filter(inv_idx.size(), jaccard_th);
    } else {
        vector<unsigned>** ids = term_ids;
        int q_num = inv_idx.size();
        candidates = divide_skip(tot, inv_idx, filter_jaccard, _mv_jaccard);
    }
    verify_filter_jaccard(query_term, jaccard_th, candidates, result);
	return SUCCESS;
}

int SimSearcher::searchED(const char* query, unsigned threshold, vector<pair<unsigned, unsigned>>& result)
{
	result.clear();
    len_list_switch = len_list;
    ed_th = threshold;
    _query_length = strlen(query);
    int tot = _query_length + 1 - _q - ed_th * _q;
    vector<unsigned> candidates;
    if (tot <= 0) {
        // cout << "tot <= 0, only using length filter" << endl;
        // all strings are candidates
        candidates = length_filter(_query_length, ed_th);
    } else {
        qgram_list->get_qgram(query, inv_idx);
        candidates = divide_skip(tot, inv_idx, filter_ed, _mv_ed);
    }
    verify_filter_ed(query, ed_th, candidates, result);
	return SUCCESS;
}

void SimSearcher::verify_filter_ed( const char* query, 
                                    unsigned threshold, 
                                    const vector<unsigned>& candidates, 
                                    vector<pair<unsigned, unsigned>>& result)
{
    int th = threshold;
    int q_length = strlen(query);
    for (int i = 0; i <= th; ++i) {
        ed_mat[i][0] = i;
    }
    for (int j = 0; j <= th+1; ++j) {
        ed_mat[0][j] = j;
    }
    for (auto id : candidates) {
        int c_length = len_list[id];
        const char* cstr = str_list[id];
        bool early_stop = false;
        for (int i = 1 ; i <= q_length; ++i) {
            int j_st = 1, j_ed = i+th;
            if (i-th > 0) {
                j_st = i-th;
                ed_mat[i][j_st-1] = th+1;
            }
            if (j_ed < c_length) {
                ed_mat[i][j_ed+1] = th+1;
            } else {
                j_ed = c_length;
            }
            int row_min = th+1;
            for (int j = j_st; j <= j_ed; ++j) {
                ed_mat[i][j] = min({
                    ed_mat[i-1][j] + 1, 
                    ed_mat[i][j-1] + 1, 
                    ed_mat[i-1][j-1] + (query[i-1] == cstr[j-1] ? 0 : 1)
                });
                row_min = min(ed_mat[i][j], row_min);
            }
            if (row_min > th) {
                early_stop = true;
                break;
            }
        }
        if (!early_stop && ed_mat[q_length][c_length] <= th) {
            result.emplace_back(id, ed_mat[q_length][c_length]);
        }
    }
}

void SimSearcher::verify_filter_jaccard(const vector<unsigned>& query_hash, 
                                        double threshold, 
                                        const vector<unsigned>& candidates, 
                                        vector<pair<unsigned, double>>& result)
{
    vector<unsigned> intersect;
    for (auto id : candidates) {
        set_intersection(query_hash.begin(), query_hash.end(), 
                        term_ids[id]->begin(), term_ids[id]->end(), 
                        back_inserter(intersect));
        double intersect_size = intersect.size();
        double union_size = query_hash.size() + term_len_list[id] - intersect_size;
        double jac = intersect_size / union_size;
        if (jac >= threshold) {
            result.emplace_back(id, jac);
        }
        intersect.clear();
    }
}

vector<unsigned> SimSearcher::length_filter(int q_length, int threshold) {
    vector<unsigned> ret;
    for (int i = 0; i < str_num; ++i) {
        if (abs(len_list[i]-q_length) > threshold) continue;
        ret.push_back(i);
    }
    return ret;
}

vector<unsigned> SimSearcher::length_filter(int q_length, int threshold, const vector<unsigned>& candidates) {
    vector<unsigned> ret;
    for (auto e : candidates) {
        if (abs(len_list[e]-q_length) > threshold) continue;
        ret.push_back(e);
    }
    return ret;
}

vector<unsigned> SimSearcher::term_num_filter(int q_term_num, double threshold) {
    vector<unsigned> ret;
    for (int i = 0; i < str_num; ++i) {
        int c_num = term_len_list[i];
        if (c_num <= q_term_num / threshold && c_num >= q_term_num * threshold) {
            ret.push_back(i);
        }
    }
    return ret;
}

vector<unsigned> SimSearcher::term_num_filter(int q_term_num, double threshold, const vector<unsigned>& candidates) {
    vector<unsigned> ret;
    for (auto id : candidates) {
        int c_num = term_len_list[id];
        if (c_num <= q_term_num / threshold && c_num >= q_term_num * threshold) {
            ret.push_back(id);
        }
    }
    return ret;
}
