#pragma once
#include "TrieInvertedList.h"
// #include "InvertedList.h"
#include <list>
#include <queue>
#include <vector>
#include <cassert>
#include <utility>
#include <iostream>
#include <cstring>
#include <string>
#include <cmath>
#include <algorithm>
#include <functional>
using namespace std;

const int SUCCESS = 0;
const int FAILURE = 1;

class SimSearcher
{
public:
	SimSearcher();
	~SimSearcher();

    /**
     * @brief 读取指定的输入文件并建立索引
     * 
     * @param filename 输入文件，每行一个字符串代表记录，行号也是记录id，从0开始
     * @param q qgram方法的建议q值，可以忽略
     * @return int 创建成功返回SUCCESS, 失败返回FAILURE
     */
	int createIndex(const char *filename, unsigned q);
    /**
     * @brief 查询与query串近似度小于threshold的所有串
     * 
     * @param query 查询串
     * @param threshold Jaccard阈值
     * @param result 结果，<字符串id, 与query之间的Jaccard相似度>，按照id从小到大排序，不能有重复
     * @return int 查询成功返回SUCCESS, 失败返回FAILURE
     */
	int searchJaccard(const char *query, double threshold, std::vector<std::pair<unsigned, double> > &result);
    /**
     * @brief 
     * 
     * @param query 查询串
     * @param threshold Edit Distance阈值
     * @param result 结果，<字符串id, 与query之间的ED相似度>
     * @return int 查询成功返回SUCCESS, 失败返回FAILURE
     */
	int searchED(const char *query, unsigned threshold, std::vector<std::pair<unsigned, unsigned> > &result);
private:
    void read_dataset(const char* filename);
    vector<pair<unsigned, int>> merge_skip(int tot, const vector<reference_wrapper<vector<int>>>& inv_index, const function<bool(int)>& filter);
    vector<unsigned> divide_skip(int tot, vector<reference_wrapper<vector<int>>>& all_list, const function<bool(int)>& filter, double mv);
    void verify_filter_ed(const char* query, unsigned threshold, const vector<unsigned>& candidates, vector<pair<unsigned, unsigned>>& result);
    void verify_filter_jaccard(const vector<unsigned>& query_hash, double threshold, const vector<unsigned>& candidates, vector<pair<unsigned, double>>& result);
    vector<unsigned> length_filter(int q_length, int threshold);
    vector<unsigned> length_filter(int q_length, int threshold, const vector<unsigned>& candidates);
    bool length_filter(int id);
    vector<unsigned> term_num_filter(int q_term_num, double threshold);
    vector<unsigned> term_num_filter(int q_term_num, double threshold, const vector<unsigned>& candidates);
private:
    unsigned _q;
    int str_num;
    int _min_term_num;
    double _mv_ed;
    double _mv_jaccard;
    char** str_list;
    vector<unsigned>** term_ids;
    TrieInvertedList* qgram_list;
    TrieInvertedList* term_list;
    // InvertedList* qgram_list;
    // InvertedList* term_list;
    vector<reference_wrapper<vector<int>>> inv_idx;
};
