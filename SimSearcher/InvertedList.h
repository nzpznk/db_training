#include <set>
#include <map>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <functional>
using namespace std;

class InvertedList {
public:
    InvertedList(int q = -1);
    ~InvertedList();
    void insert_qgram(const char* str, int index);
    void insert_term(const char* str, int index, vector<unsigned>& term_id);
    void get_qgram(const char* str, vector<reference_wrapper<vector<int>>>& inv_index);
    void get_term(const char* str, vector<unsigned>& query_term, vector<reference_wrapper<vector<int>>>& inv_index);
    void qgram_output(); // just for debug
    void term_output(); // just for debug
private:
    void bin_search_insert_index(int bucket_ind, int index);
private:
    unsigned _q;
    vector<int> _empty_list;
};
