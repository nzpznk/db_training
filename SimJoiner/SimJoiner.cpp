#include "SimJoiner.h"

using namespace std;

const int DATAPOOL_SIZE = 80000000;
static char input1[DATAPOOL_SIZE];
static char input2[DATAPOOL_SIZE];
static int segLenTable[300][4];
static int segStartTable[300][4];
static int ed_mat[256][256];
static unordered_map<int, int> term_cnt;

bool operator< (const TermId& l, const TermId& r) {
    int lcnt = term_cnt[l.id], rcnt = term_cnt[r.id];
    return (lcnt < rcnt) || ((lcnt == rcnt) && (l.id < r.id));
}

bool operator== (const TermId& l, const TermId& r) {
    return l.id == r.id;
}

inline void initSegLenTable(int tau) {
    ++tau;
    for (int i = 0; i < 300; ++i) {
        int avg = i / tau;
        for (int j = 0; j < tau; ++j) {
            segLenTable[i][j] = avg;
        }
        for (int j = tau - (i % tau); j < tau; ++j) {
            ++segLenTable[i][j];
        }
        segStartTable[i][0] = 0;
        for (int j = 1; j < tau; ++j) {
            segStartTable[i][j] = segStartTable[i][j-1] + segLenTable[i][j-1];
        }
    }
}

inline void readDataset(const char* filename, char* dataPool, vector<const char*>& strs, vector<int>& strLen) {
    strs.clear();
    strLen.clear();
    FILE* fp = fopen(filename, "r");
    int num = fread(dataPool, sizeof(char), DATAPOOL_SIZE, fp);
    fclose(fp);
    dataPool[num] = '\0';
    for (int i = 0; i < num; ++i) {
        strs.push_back(dataPool + i);
        while(dataPool[i] != '\n') ++i;
        dataPool[i] = '\0';
        strLen.push_back(dataPool + i - strs.back());
    }
}

SimJoiner::SimJoiner() {
}

SimJoiner::~SimJoiner() {
}

int SimJoiner::joinED(const char *filename1, const char *filename2, unsigned threshold, vector<EDJoinResult> &result) {
    result.clear();
    tau = threshold;
    initSegLenTable(tau);
    readDataset(filename1, input1, strSet1, strLen1);
    readDataset(filename2, input2, strSet2, strLen2);
    createPassJoinIndex(strSet2, strLen2);
    passJoin(strSet1, strLen1, strSet2, strLen2, result);
    return SUCCESS;
}

vector<string> SimJoiner::substringSelection(const char* str, int len, int l, int t) {
    vector<string> ret;
    int delta = len-l;
    int st = max({0, segStartTable[l][t]-t, segStartTable[l][t]+delta-(tau-t)});
    int ed = min({len-segLenTable[l][t], segStartTable[l][t]+t, segStartTable[l][t]+delta+(tau-t)});
    for (int p = st; p <= ed; ++p) {
        ret.push_back(string(str+p, str+p+segLenTable[l][t]));
    }
    return ret;
}

void SimJoiner::passJoin(vector<const char*>& strs1, vector<int>& lens1, 
                         vector<const char*>& strs2, vector<int>& lens2, 
                         vector<EDJoinResult>& result) {
    for (int i = 0; i < strs1.size(); ++i) {
        const char* str = strs1[i];
        int len = lens1[i];
        vector<int> candidateSet;
        for (int l = len - tau; l <= len + tau; ++l) {
            for (int t = 0; t <= tau; ++t) {
                // sub string selection
                auto substringSet = substringSelection(str, len, l, t);
                // add candidates
                InvIndexMap& indexMap = inv[l][t];
                InvIndexMap::iterator iter;
                for (string& substring : substringSet) {
                    if ((iter = indexMap.find(substring)) != indexMap.end()) {
                        candidateSet.insert(candidateSet.end(), iter->second.begin(), iter->second.end());
                        // candidateSet.insert(iter->second.begin(), iter->second.end());
                    }
                }
            }
        }
        sort(candidateSet.begin(), candidateSet.end());
        auto ip = unique(candidateSet.begin(), candidateSet.end());
        candidateSet.resize(ip - candidateSet.begin());
        // verification
        verify_ed_dist(i, str, lens1[i], strs2, lens2, candidateSet, result);
    }
}

inline void SimJoiner::verify_ed_dist(  int first_id, 
                                        const char* query, 
                                        int q_length, 
                                        vector<const char*>& strs2, 
                                        vector<int>& lens2, 
                                        const vector<int>& candidates, 
                                        vector<EDJoinResult>& result)
{
    for (int i = 0; i <= tau; ++i) {
        ed_mat[i][0] = i;
    }
    for (int j = 0; j <= tau+1; ++j) {
        ed_mat[0][j] = j;
    }
    for (auto id : candidates) {
        int c_length = lens2[id];
        const char* cstr = strs2[id];
        bool early_stop = false;
        for (int i = 1 ; i <= q_length; ++i) {
            int j_st = 1, j_ed = i+tau;
            if (i-tau > 0) {
                j_st = i-tau;
                ed_mat[i][j_st-1] = tau+1;
            }
            if (j_ed < c_length) {
                ed_mat[i][j_ed+1] = tau+1;
            } else {
                j_ed = c_length;
            }
            int row_min = tau+1;
            for (int j = j_st; j <= j_ed; ++j) {
                ed_mat[i][j] = min({
                    ed_mat[i-1][j] + 1, 
                    ed_mat[i][j-1] + 1, 
                    ed_mat[i-1][j-1] + (query[i-1] == cstr[j-1] ? 0 : 1)
                });
                row_min = min(ed_mat[i][j], row_min);
            }
            if (row_min > tau) {
                early_stop = true;
                break;
            }
        }
        if (!early_stop && ed_mat[q_length][c_length] <= tau) {
            result.emplace_back(first_id, id);
        }
    }
}

void SimJoiner::createPassJoinIndex(vector<const char*>& strs, vector<int>& lens) {
    for (int i = 0; i < 300; ++i) {
        for (int j = 0; j <= tau; ++j) {
            inv[i][j].clear();
        }
    }
    for (int i = 0; i < strs.size(); ++i) {
        for (int j = 0; j <= tau; ++j) {
            string seg = string(strs[i] + segStartTable[lens[i]][j], segLenTable[lens[i]][j]);
            InvIndexMap& index = inv[lens[i]][j];
            InvIndexMap::iterator iter;
            if ((iter = index.find(seg)) == index.end()) {
                index.emplace(seg, InvIndexVec({i}));
            } else {
                iter->second.push_back(i);
            }
        }
    }
}

void SimJoiner::tokenize(   const vector<const char*>& strs, 
                            const vector<int>& lens, 
                            vector<vector<TermId>>& terms)
{
    terms.clear();
    for (int i = 0; i < strs.size(); ++i) {
        terms.emplace_back();
        const char *str = strs[i];
        const char *lo = str, *hi = str;
        while ((*lo) == ' ') ++lo;
        hi = lo;
        while ((*hi) != '\0') {
            while ((*hi) != ' ' && (*hi) != '\0') { ++hi; }
            // [lo, hi) is the term
            string term(lo, hi);
            unordered_map<string, TermId>::iterator iter;
            if ((iter = term2id.find(term)) == term2id.end()) {
                term2id.emplace(term, global_term_counter);
                term_cnt.emplace(global_term_counter, 1);
                terms[i].emplace_back(global_term_counter);
                ++global_term_counter;
            } else if (find(terms[i].begin(), terms[i].end(), iter->second) == terms[i].end()) {
                terms[i].push_back(iter->second);
                ++term_cnt[iter->second.id];
            }
            while((*hi) == ' ') ++hi;
            lo = hi;
        }
    }
}

void SimJoiner::sortTermByIDF(vector<vector<TermId>>& terms) {
    for (auto& term : terms) {
        sort(term.begin(), term.end());
    }
}

void SimJoiner::createDeltaIndex(const vector<vector<TermId>>& terms, int t) {
    dinv.clear();
    dinv.assign(t, DeltaIndex());
    s_upper = 0;
    s_lower = 1e8;
    for (int i = 0; i < terms.size(); ++i) {
        auto& term = terms[i];
        s_upper = max(s_upper, (int)term.size());
        s_lower = min(s_lower, (int)term.size());
        int termsize = term.size();
        for (int j = 0; j < termsize; ++j) {
            int suffix_len = termsize - j;
            if (t >= suffix_len) {
                dinv[t - suffix_len][term[j].id].push_back(i);
            } else {
                dinv[0][term[j].id].push_back(i);
            }
        }
    }
}

void SimJoiner::test_index() {
    vector<vector<TermId>> termSet = {
              {2, 3, 5},
              {3, 6, 7}, 
              {2, 4, 7}, 
        {1, 4, 5, 8, 9}, 
              {1, 5, 6}
    };
    createDeltaIndex(termSet, 4);
    for (int i = 0; i < dinv.size(); ++i) {
        cout << "Delta(I" << i+1 << "):" << endl;
        for (auto x : dinv[i]) {
            cout << "e" << x.first << ": ";
            for (auto str_id : x.second) {
                cout << str_id+1 << " ";
            }
            cout << endl;
        }
    }
}

void SimJoiner::calMaxT(const vector<vector<TermId>>& terms, double jac) {
    t_max = 0;
    for (auto& term : terms) {
        int s = ceil(term.size() * jac);
        if (s > t_max) t_max = s;
    }
}

inline void SimJoiner::getPrefix(   int t, 
                                    int l, 
                                    const vector<TermId>& term, 
                                    vector<int>& prefix)
{
    // cout << "termsize: " << term.size() << endl;
    for (int i = 0; i < term.size() - t + l; ++i) {
        prefix.push_back(term[i].id);
    }
}

void SimJoiner::getAdaptJoinCandidates( int t, 
                                        int l, 
                                        int first_id, 
                                        const vector<int>& prefix, 
                                        const unordered_map<int, int>& count, 
                                        vector<int>& candidates)
{
    int rlen = termSet1[first_id].size();
    for (auto kv : count) {
        int slen = termSet2[kv.first].size();
        if (slen > rlen / jac || slen < rlen * jac) {
            continue;
        }
        if (kv.second >= l) {
            candidates.push_back(kv.first);
        }
    }
    // unordered_map<int, int> count;
    // int rlen = termSet1[first_id].size();
    // int last_dinv_id = t_max - t + l;
    // for (int i = 0; i < last_dinv_id; ++i) {
    //     for (auto term_id : prefix) {
    //         if (dinv[i].find(term_id)==dinv[i].end()) {
    //             continue;
    //         }
    //         // cout << "term_id: " << term_id << ": ";
    //         for (auto str_id : dinv[i][term_id]) {
    //             int slen = termSet2[str_id].size();
    //             if (slen > rlen / jac || slen < rlen * jac) {
    //                 continue;
    //             }
    //             if (count.find(str_id) == count.end()) {
    //                 count[str_id] = 1;
    //             } else {
    //                 ++count[str_id];
    //             }
    //             if (count[str_id] == l) {
    //                 candidates.push_back(str_id);
    //             }
    //         }
    //         // cout << endl;
    //     }
    // }
    // cout << "t: " << t << ", l: " << l << endl;
    // cout << "count: " << count[26] << " " << count[27] << " " << count[28] << endl;
    sort(candidates.begin(), candidates.end());
}

void SimJoiner::verify_jaccard( int first_id, 
                                const vector<TermId>& query, 
                                const vector<int>& candidates, 
                                const vector<vector<TermId>>& second_set, 
                                vector<JaccardJoinResult>& result)
{
    vector<TermId> intersect;
    for (auto id : candidates) {
        set_intersection(   query.begin(), query.end(), 
                            second_set[id].begin(), second_set[id].end(), 
                            back_inserter(intersect));
        double intersect_size = intersect.size();
        double union_size = query.size() + second_set[id].size() - intersect_size;
        double jaccard = intersect_size / union_size;
        if (jaccard >= jac) {
            result.emplace_back(first_id, id);
        }
        intersect.clear();
    }
}

int SimJoiner::estimateCost(int t, 
                            const vector<TermId>& query, 
                            vector<int>& prefix, 
                            unordered_map<int, int>& count)
{
    int prev_fr, fr;
    double prev_cr, cr;
    fr = 0;
    cr = 0;
    getPrefix(t, 1, query, prefix);
    int prefix_len = prefix.size();
    int first_dinv_id = t_max - t + 1;
    count.clear();
    // init
    for (auto termid : prefix) {
        for (int i = 0; i < first_dinv_id; ++i) {
            if (dinv[i].find(termid) == dinv[i].end()) {
                continue;
            }
            auto& termid_inv = dinv[i][termid];
            unordered_map<StrId, int>::iterator iter;
            for (auto str_id : termid_inv) {
                if ((iter = count.find(str_id)) == count.end()) {
                    count.emplace(str_id, 1);
                } else {
                    ++iter->second;
                }
            }
            fr += termid_inv.size();
        }
    }
    if (t == 1) return 1;
    cr = count.size();
    // incremental computation
    vector<reference_wrapper<vector<int>>> delta_index;
    vector<int> virtual_table_st = {0};
    int chosen_l = t-1;
    for (int i = first_dinv_id; i < t_max; ++i) {
        prev_fr = fr;
        prev_cr = cr;
        cr = 0;
        int new_term = query[prefix_len++].id;
        prefix.push_back(new_term);
        delta_index.clear();
        // new fr
        for (int j = 0; j < i; ++j) {
            if (dinv[j].find(new_term) == dinv[j].end()) {
                continue;
            }
            auto& termid_inv = dinv[j][new_term];
            delta_index.emplace_back(termid_inv);
            virtual_table_st.push_back(virtual_table_st.back() + termid_inv.size());
            fr += termid_inv.size();
        }
        for (auto termid : prefix) {
            if (dinv[i].find(termid) == dinv[i].end()) {
                continue;
            }
            auto& termid_inv = dinv[i][termid];
            delta_index.emplace_back(termid_inv);
            virtual_table_st.push_back(virtual_table_st.back() + termid_inv.size());
            fr += termid_inv.size();
        }
        // new cr, sample and estimate
        int K = ratio * virtual_table_st.back();
        int cnt = 0;
        for (int samp_idx = 0; samp_idx < K; ++samp_idx) {
            int id = rand() % virtual_table_st.back();
            auto iter = upper_bound(virtual_table_st.begin(), virtual_table_st.end(), id);
            int vec_id = (iter - virtual_table_st.begin()) - 1;
            StrId sample = delta_index[vec_id].get()[id - virtual_table_st[vec_id]];
            if (count.find(sample) == count.end()) continue;
            else if (count[sample] > i-first_dinv_id+1) {
                cr += 1;
            } else if (count[sample] == i-first_dinv_id+1) {
                cnt += 1;
            }
        }
        cr += cnt * 1.0 / K * (virtual_table_st.back());
        
        virtual_table_st.clear();
        virtual_table_st.push_back(0);
        if (prev_fr + prev_cr * (query.size() + (s_upper + s_lower) / 2.0)
            <= fr + cr * (query.size() + (s_upper + s_lower) / 2.0)) 
        {
            chosen_l = i - first_dinv_id + 1;
            break;
        }
        // update count set
        unordered_map<int, int>::iterator iter;
        for (auto update_index : delta_index) {
            for (auto str_id : update_index.get()) {
                if ((iter = count.find(str_id)) == count.end()) {
                    count.emplace(str_id, 1);
                } else {
                    ++iter->second;
                }
            }
        }
    }
    return chosen_l;
}

int SimJoiner::joinJaccard( const char *filename1, 
                            const char *filename2, 
                            double threshold, 
                            vector<JaccardJoinResult> &result) 
{
    result.clear();
    term2id.clear();
    term_cnt.clear();
    global_term_counter = 0;
    jac = threshold;
    ratio = 0.015;
    readDataset(filename1, input1, strSet1, strLen1);
    readDataset(filename2, input2, strSet2, strLen2);
    tokenize(strSet1, strLen1, termSet1);
    tokenize(strSet2, strLen2, termSet2);
    sortTermByIDF(termSet1);
    sortTermByIDF(termSet2);
    calMaxT(termSet1, threshold);
    // 使用t_max划分出的是最细粒度的delta index
    createDeltaIndex(termSet2, t_max);
    vector<int> prefix;
    vector<int> candidates;
    unordered_map<int, int> count;
    for (int i = 0; i < termSet1.size(); ++i) {
        // get overlap t
        int t = ceil(jac * termSet1[i].size());
        // estimation, use l-prefix
        // int l = max(1, t / 5);
        int l = estimateCost(t, termSet1[i], prefix, count);
        // cout << l << endl;
        // for (auto kv : count) {
        //     cout << kv.first << ": " << kv.second << endl;
        // }
        // getPrefix(t, l, termSet1[i], prefix);
        getAdaptJoinCandidates(t, l, i, prefix, count, candidates);
        verify_jaccard(i, termSet1[i], candidates, termSet2, result);
        prefix.clear();
        candidates.clear();
        // if ((i + 1) % 1000 == 0) {
        //     cout << i + 1 << endl;
        // }
    }
    // cout << "haha_counter: " << haha_counter << endl;
    return SUCCESS;
}
