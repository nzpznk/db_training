#include "InvertedList.h"

static const int _capacity = 999983;
static vector<vector<int>> block1[_capacity];
static vector<int> block2[_capacity];
static int b1p = 0;
static int b2p = 0;
static int prime = 131;
static unsigned _p_power;
static vector<vector<int> >** _qgram_table;
static vector<int>** _term_table;
static const char* _vals[_capacity];
static const char* _ends[_capacity];

InvertedList::InvertedList(int q) {
    _q = q;
    if (_q != -1) {
        _p_power = 1;
        for (int i = 1; i < _q; ++i) {
            _p_power *= prime;
            _p_power %= _capacity;
        }
        _qgram_table = new vector<vector<int>>*[_capacity];
    } else {
        _term_table = new vector<int>*[_capacity];
    }
}

InvertedList::~InvertedList() {
    for (int i = 0; i < _capacity; ++i) {
        if (_vals[i] != nullptr) {
            delete _qgram_table[i];
            _qgram_table[i] = nullptr;
        }
    }
    delete[] _qgram_table;
    _qgram_table = nullptr;
}

void InvertedList::bin_search_insert_index(int bucket_num, int index) {
    // *******************************or just a for loop**********************************************
    vector<vector<int>>& hash_list = *_qgram_table[bucket_num];
    // bool found = false;
    // for (auto it = hash_list.begin(); it != hash_list.end(); ++it) {
    //     if ((*it).back() != index) {
    //         (*it).push_back(index);
    //         found = true;
    //         break;
    //     }
    // }
    // if (!found) {
    //     hash_list.push_back({index});
    // }
    int lo = 0, hi = hash_list.size()-1;
    if (hash_list[lo].back() != index) {
        hash_list[lo].push_back(index);
    } else if (hash_list[hi].back() == index) {
        hash_list.push_back({index});
    } else {
        while(hi - lo > 1) {
            int mi = lo + (hi - lo) / 2;
            if (hash_list[mi].back() == index) {
                lo = mi;
            } else {
                hi = mi;
            }
        }
        hash_list[hi].push_back(index);
    }
}

void InvertedList::insert_qgram(const char* str, int index) {
    const char *lo = str, *hi = str;
    unsigned int hash = 0, bak_hash = 0;
    while (hi-lo < _q) {
        hash = (hash * prime + (*hi)) % _capacity;
        ++hi;
    }
    --hi;
    while ((*hi) != '\0') {
        int step = 1;
        // cout << "gram: " << string(lo, hi+1) << " " << hash;
        bak_hash = hash;
        while (_vals[hash] && memcmp(lo, _vals[hash], _q) != 0) { // bucket is not null and not the target bucket
            hash = (bak_hash + step * step) % _capacity;
            // cout << "->" << hash;
            step += 1;
        }
        // cout << endl;
        if (!_vals[hash]) {
            _vals[hash] = lo;
            _qgram_table[hash] = &block1[b1p++];
            _qgram_table[hash]->push_back({index});
        } else { // memcmp(str + lo, _vals[posi], _q) == 0, bucket is found
            bin_search_insert_index(hash, index);
        }
        hash = (bak_hash + _capacity - (_p_power * (*lo) % _capacity)) % _capacity;
        ++lo;
        ++hi;
        hash = (hash * prime + (*hi)) % _capacity;
    }
}

void InvertedList::insert_term(const char* str, int index, vector<unsigned>& term_id) {
    const char *lo = str, *hi = str;
    unsigned hash = 0, bak_hash = 0;
    while ((*lo) == ' ') ++lo;
    hi = lo;
    while ((*hi) != '\0') {
        hash = 0;
        while ((*hi) != ' ' && (*hi) != '\0') {
            hash = (hash * prime + (*hi)) % _capacity;
            ++hi;
        }
        // cout << string(lo, hi) << " hash: " << hash << endl;
        bak_hash = hash;
        int step = 1;
        while(_vals[hash] && (hi - lo != _ends[hash] - _vals[hash] || memcmp(lo, _vals[hash], hi - lo) != 0)) {
            hash = (bak_hash + step * step) % _capacity;
            step += 1;
        }
        if (!_vals[hash]) {
            _vals[hash] = lo;
            _ends[hash] = hi;
            block2[b2p].push_back(index);
            _term_table[hash] = &block2[b2p++];
            term_id.push_back(hash);
        } else if (_term_table[hash]->back() != index){
            _term_table[hash]->push_back(index);
            term_id.push_back(hash);
        }
        while((*hi) == ' ') ++hi;
        lo = hi;
    }
    sort(term_id.begin(), term_id.end());
}

void InvertedList::get_qgram(const char* str, vector<reference_wrapper<vector<int>>>& inv_index) {
    map<unsigned, int> hash2cnt;
    const char *lo = str, *hi = str;
    unsigned int hash = 0, bak_hash = 0;
    while (hi-lo < _q) {
        hash = (hash * prime + (*hi)) % _capacity;
        ++hi;
    }
    --hi;
    inv_index.clear();
    while ((*hi) != '\0') {
        int step = 1;
        bak_hash = hash;
        while (_vals[hash] && memcmp(lo, _vals[hash], _q) != 0) { // bucket is not null and not the target bucket
            hash = (bak_hash + step * step) % _capacity;
            step += 1;
        }
        if (!_vals[hash]) {
            inv_index.push_back(_empty_list);
        } else { // memcmp(str + lo, _vals[posi], _q) == 0, bucket is found
            if (hash2cnt.find(hash) == hash2cnt.end()) {
                hash2cnt.emplace(hash, 0);
            }
            inv_index.push_back((*_qgram_table[hash])[hash2cnt[hash]++]);
        }
        hash = (bak_hash + _capacity - (_p_power * (*lo) % _capacity)) % _capacity;
        ++lo;
        ++hi;
        hash = (hash * prime + (*hi)) % _capacity;
    }
}

void InvertedList::get_term(const char* str, vector<unsigned>& query_term, vector<reference_wrapper<vector<int>>>& inv_index) {
    inv_index.clear();
    set<unsigned> selected;
    const char *lo = str, *hi = str;
    unsigned hash = 0, bak_hash = 0;
    while ((*lo) == ' ') ++lo;
    hi = lo;
    while ((*hi) != '\0') {
        hash = 0;
        while ((*hi) != ' ' && (*hi) != '\0') {
            hash = (hash * prime + (*hi)) % _capacity;
            ++hi;
        }
        bak_hash = hash;
        int step = 1;
        while(_vals[hash] && (hi - lo != _ends[hash] - _vals[hash] || memcmp(lo, _vals[hash], hi - lo) != 0)) {
            hash = (bak_hash + step * step) % _capacity;
            step += 1;
        }
        if (!_vals[hash]) {
            inv_index.push_back(_empty_list);
            selected.insert(hash);
        } else if (selected.find(hash) == selected.end()){
            inv_index.push_back(*(_term_table[hash]));
            selected.insert(hash);
        }
        while((*hi) == ' ') ++hi;
        lo = hi;
    }
    query_term.assign(selected.begin(), selected.end());
}

void InvertedList::qgram_output() {
    for (int i = 0; i < _capacity; ++i) {
        if (_vals[i] != nullptr) {
            cout << string(_vals[i], _vals[i] + _q) << endl;
            for (auto v : *_qgram_table[i]) {
                cout << "\t";
                for (int ind : v) {
                    cout << ind << " ";
                }
                cout << endl;
            }
        }
    }
}

void InvertedList::term_output() {
    for (int i = 0; i < _capacity; ++i) {
        if (_vals[i] != nullptr) {
            cout << string(_vals[i], _ends[i]) << endl;
            cout << "\t";
            for (auto ind : *_term_table[i]) {
                cout << ind << " ";
            }
            cout << endl;
        }
    }
}
