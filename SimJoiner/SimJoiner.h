#ifndef __EXP2_SIMJOINER_H__
#define __EXP2_SIMJOINER_H__

#include <cstdlib>
#include <cmath>
#include <vector>
#include <cstring>
#include <string>
#include <functional>
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <unordered_map>

using namespace std;

template <typename IDType, typename SimType>
struct JoinResult {
    IDType id1;
    IDType id2;
    SimType s;
    JoinResult(IDType _id1, IDType _id2): id1(_id1), id2(_id2) {}
};

typedef struct TermId { 
    int id;
    TermId(int _id = -1) : id(_id) {}
} TermId;
typedef int StrId;
typedef JoinResult<unsigned, double> JaccardJoinResult;
typedef JoinResult<unsigned, unsigned> EDJoinResult;
typedef vector<int> InvIndexVec;
typedef unordered_map<string, InvIndexVec> InvIndexMap;
typedef unordered_map<int, vector<StrId>> DeltaIndex;

const int SUCCESS = 0;
const int FAILURE = 1;

class SimJoiner {
public:
    SimJoiner();
    ~SimJoiner();
    int joinED( const char *filename1, 
                const char *filename2, 
                unsigned threshold, 
                std::vector<EDJoinResult> &result);
    int joinJaccard(const char *filename1, 
                    const char *filename2, 
                    double threshold, 
                    std::vector<JaccardJoinResult> &result);

private:
    void createPassJoinIndex(vector<const char*>& strs, vector<int>& lens);
    void passJoin(vector<const char*>& unindexed_strs, vector<int>& unindexed_strlens, 
                  vector<const char*>& indexed_strs, vector<int>& indexed_strlens, 
                  vector<EDJoinResult>& result);
    vector<string> substringSelection(const char* str, int len, int l, int t);
    void verify_ed_dist(int first_id, 
                        const char* query, 
                        int q_length, 
                        vector<const char*>& strs2, 
                        vector<int>& lens2, 
                        const vector<int>& candidates, 
                        vector<EDJoinResult>& result);
    void tokenize(  const vector<const char*>& strs, 
                    const vector<int>& lens, 
                    vector<vector<TermId>>& terms);
    void sortTermByIDF(vector<vector<TermId>>& terms);
    void createDeltaIndex(const vector<vector<TermId>>& terms, int t);
    void calMaxT(const vector<vector<TermId>>& terms, double jac);
    void getPrefix(int t, int l, const vector<TermId>& term, vector<int>& prefix);
    void getAdaptJoinCandidates(int t, 
                                int l, 
                                int first_id, 
                                const vector<int>& prefix, 
                                const unordered_map<int, int>& count, 
                                vector<int>& candidates);
    void verify_jaccard(int first_id, 
                        const vector<TermId>& query, 
                        const vector<int>& candidates, 
                        const vector<vector<TermId>>& second_set, 
                        vector<JaccardJoinResult> &result);
    int estimateCost(   int t, 
                        const vector<TermId>& query, 
                        vector<int>& prefix, 
                        unordered_map<int, int>& count);

    void test_index();
    // void 
    int tau;
    double jac;
    vector<const char*> strSet1;
    vector<int> strLen1;
    vector<const char*> strSet2;
    vector<int> strLen2;
    unordered_map<string, TermId> term2id;
    vector<vector<TermId>> termSet1;
    vector<vector<TermId>> termSet2;
    int global_term_counter;
    int t_max;
    int s_upper;
    int s_lower;
    double ratio;
    InvIndexMap inv[300][4]; // given l, i, seg, get inv_vec
    vector<DeltaIndex> dinv; // given l, term_id, get inv_vec
};

#endif
