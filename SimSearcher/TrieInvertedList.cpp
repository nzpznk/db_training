#include "InvertedList.h"

static int block[128000000];

template<typename T>
struct TrieNode {
    char val;
    int term_id;
    int* ch;
    T id_list; 
};

static TrieNode<vector<int>> termlist[1000000]; // nodelist[0] is the root
static TrieNode<vector<vector<int>>> qgramlist[1000000];
// static int p = 1;

template<typename T>
inline TrieNode<T>* alloc_node() {
    nodelist[p].ch = block + (p << 7);
    return &nodelist[p++];
}

InvertedList::InvertedList(int q = -1) {
    this->_q = q;
    nodelist[0].ch = block;
}

InvertedList::~InvertedList() {

}

void InvertedList::insert_qgram(const char* str, int index) {

}

void InvertedList::insert_term(const char* str, int index, vector<unsigned>& term_id) {

}

void InvertedList::get_qgram(const char* str, vector<reference_wrapper<vector<int>>>& inv_index) {

}

void InvertedList::get_term(const char* str, vector<unsigned>& query_term, vector<reference_wrapper<vector<int>>>& inv_index) {

}

void InvertedList::qgram_output() {}// just for debug
void InvertedList::term_output() {} // just for debug

void InvertedList::bin_search_insert_index(int bucket_ind, int index) {

}
