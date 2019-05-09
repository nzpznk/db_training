#include "TrieInvertedList.h"

// static int qgram_block[128000000];
// static int term_block[128000000];
TrieNode<vector<vector<int>>> TrieInvertedList::qgramlist[1000000];
TrieNode<vector<int>> TrieInvertedList::termlist[1000000];

static int p1 = 1;
static int p2 = 1;

TrieInvertedList::TrieInvertedList(int q) {
    this->term_id_counter = 0;
    this->_q = min(8, q);
    // if (_q == -1) {
    //     // qgramlist = nullptr;
    //     // termlist = new TrieNode<vector<int>>[1000000];
    //     termlist[0].ch = term_block;
    // } else {
    //     // termlist = nullptr;
    //     // qgramlist = new TrieNode<vector<vector<int>>>[1000000];
    //     qgramlist[0].ch = qgram_block;
    // }
}

TrieInvertedList::~TrieInvertedList() {
    // delete[] termlist;
    // delete[] qgramlist;
    // termlist = nullptr;
    // qgramlist = nullptr;
}

void TrieInvertedList::insert_qgram(char** str, int str_num) {
    // cout << "insert qgram: " << str << endl;
    for (int index = 0; index < str_num; ++index) {
        const char *lo = str[index], *hi = str[index]+_q-1;
        int next = 0;
        auto root = &qgramlist[0];
        map<unsigned long long, int> hash2cnt;
        unsigned long long hash = 0;
        int i = 0;
        while ((*hi) != '\0') {
            auto cur = root;
            hash = 0;
            i = 0;
            for (const char* pval = lo; pval <= hi; ++pval, ++i) {
                hash |= ((*pval) << (i << 3));
                next = cur->ch[*pval];
                if (next == 0) { // insert a new node
                    next = cur->ch[*pval] = p1++;
                    // cout << "p1 << 7: " << (p1<<7) << endl;
                    // qgramlist[next].ch = &qgram_block[p1 << 7];
                }
                // cout << *pval << ": p1: " << cur-qgramlist << " next: " << next << endl;
                cur = &qgramlist[next];
            }
            auto it = hash2cnt.find(hash);
            int ind = 0;
            if (it == hash2cnt.end()) {
                hash2cnt.emplace(hash, 1);
            } else {
                ind = (it->second)++;
            }
            if (ind == cur->id_list.size()) {
                cur->id_list.emplace_back();
            }
            cur->id_list[ind].push_back(index);
            ++lo;
            ++hi;
        }
    }
}

void TrieInvertedList::insert_term(const char* str, int index, vector<unsigned>& term_id) {
    // cout << "insert into term: " << str << endl;
    const char *lo = str, *hi = str;
    while ((*lo) == ' ') ++lo;
    hi = lo;
    auto root = &termlist[0];
    int next = 0;
    while ((*hi) != '\0') {
        auto cur = root;
        while ((*hi) != ' ' && (*hi) != '\0') {
            next = cur->ch[*hi];
            if (next == 0) {
                next = cur->ch[*hi] = p2++;
                // termlist[next].ch = &term_block[p2 << 7];
            }
            cur = &termlist[next];
            ++hi;
        }
        // cout << string(lo, hi-lo) << endl;
        if (cur->id_list.empty()) {
            cur->term_id = term_id_counter++;
            cur->id_list.push_back(index);
            term_id.push_back(cur->term_id);
        } else if (cur->id_list.back() != index) {
            cur->id_list.push_back(index);
            term_id.push_back(cur->term_id);
        }
        while((*hi) == ' ') ++hi;
        lo = hi;
    }
    std::sort(term_id.begin(), term_id.end());
}

void TrieInvertedList::get_qgram(const char* str, vector<reference_wrapper<vector<int>>>& inv_index) {
    const char *lo = str, *hi = str+_q-1;
    int next = 0;
    auto root = &qgramlist[0];
    map<unsigned long long, int> hash2cnt;
    unsigned long long hash = 0;
    int i = 0;
    inv_index.clear();
    while ((*hi) != '\0') {
        auto cur = root;
        hash = 0;
        i = 0;
        bool found = true;
        for (const char* pval = lo; pval <= hi; ++pval, ++i) {
            hash |= ((*pval) << (i << 3));
            next = cur->ch[*pval];
            if (next == 0) { // qgram not found
                inv_index.push_back(_empty_list);
                found = false;
                break;
            }
            cur = &qgramlist[next];
        }
        ++lo;
        ++hi;
        if (found) {
            auto it = hash2cnt.find(hash);
            if (it == hash2cnt.end()) {
                hash2cnt.emplace(hash, 1);
                inv_index.push_back(cur->id_list[0]);
            } else if(it->second < cur->id_list.size()) {
                inv_index.push_back(cur->id_list[(it->second)++]);
            } else {
                inv_index.push_back(_empty_list);
            }
        }
    }
}

void TrieInvertedList::get_term(const char* str, vector<unsigned>& query_term, vector<reference_wrapper<vector<int>>>& inv_index) {
    // cout << "geting term................................" << endl;
    const char *lo = str, *hi = str;
    while ((*lo) == ' ') ++lo;
    hi = lo;
    auto root = &termlist[0];
    int next = 0;
    set<unsigned> selected;
    set<string> new_term_in_query;
    inv_index.clear();
    while ((*hi) != '\0') {
        auto cur = root;
        bool found = true;
        while ((*hi) != ' ' && (*hi) != '\0') {
            next = cur->ch[*hi];
            if (next == 0) {
                found = false;
                while ((*hi) != ' ' && (*hi) != '\0') ++hi;
                auto new_term = string(lo, hi-lo);
                if (new_term_in_query.find(new_term) == new_term_in_query.end()) {
                    new_term_in_query.insert(new_term);
                    inv_index.push_back(_empty_list);
                    selected.insert(term_id_counter++);
                }
                break;
            }
            cur = &termlist[next];
            ++hi;
        }
        if (found) {
            if (cur->id_list.empty()) { // also is term not in dataset
                auto new_term = string(lo, hi-lo);
                if (new_term_in_query.find(new_term) == new_term_in_query.end()) {
                    new_term_in_query.insert(new_term);
                    inv_index.push_back(_empty_list);
                    selected.insert(term_id_counter++);
                }
            } else if (selected.find(cur->term_id) == selected.end()) {
                // cout << string(lo, hi-lo) << ", term_id: " << cur->term_id << endl;
                selected.insert(cur->term_id);
                inv_index.push_back(cur->id_list);
            }
        }
        while((*hi) == ' ') ++hi;
        lo = hi;
    }
    query_term.assign(selected.begin(), selected.end());
}

void TrieInvertedList::qgram_output() {}// just for debug
void TrieInvertedList::term_output() {} // just for debug
