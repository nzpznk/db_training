#include <set>
#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <functional>
using namespace std;

template<typename T>
struct TrieNode {
    int term_id;
    T id_list; 
    int ch[128];
};

class TrieInvertedList {
public:
    TrieInvertedList(int q = -1);
    ~TrieInvertedList();
    void insert_qgram(char** str, int str_num);
    void insert_term(const char* str, int index, vector<unsigned>& term_id);
    void get_qgram(const char* str, vector<reference_wrapper<vector<int>>>& inv_index);
    void get_term(const char* str, vector<unsigned>& query_term, vector<reference_wrapper<vector<int>>>& inv_index);
    void qgram_output(); // just for debug
    void term_output(); // just for debug
private:
    unsigned _q;
    int term_id_counter;
    vector<int> _empty_list;
    static TrieNode<vector<vector<int>>> qgramlist[];
    static TrieNode<vector<int>> termlist[];
};
