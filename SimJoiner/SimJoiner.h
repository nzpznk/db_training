#ifndef __EXP2_SIMJOINER_H__
#define __EXP2_SIMJOINER_H__

#include <set>
#include <bitset>
#include <vector>
#include <cstring>
#include <string>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <unordered_map>

using namespace std;

template <typename IDType, typename SimType>
struct JoinResult {
    IDType id1;
    IDType id2;
    SimType s;
    JoinResult(IDType _id1, IDType _id2): id1(_id1), id2(_id2) {}
};

typedef JoinResult<unsigned, double> JaccardJoinResult;
typedef JoinResult<unsigned, unsigned> EDJoinResult;
typedef vector<int> InvIndexVec;
typedef unordered_map<string, InvIndexVec> InvIndexMap;

const int SUCCESS = 0;
const int FAILURE = 1;

class SimJoiner {
public:
    SimJoiner();
    ~SimJoiner();
    int joinJaccard(const char *filename1, const char *filename2, double threshold, std::vector<JaccardJoinResult> &result);
    int joinED(const char *filename1, const char *filename2, unsigned threshold, std::vector<EDJoinResult> &result);

private:
    void createIndex(vector<const char*>& strs, vector<int>& lens);
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
    int tau;
    vector<const char*> strSet1;
    vector<int> strLen1;
    vector<const char*> strSet2;
    vector<int> strLen2;
    InvIndexMap inv[300][4]; // given l, i, seg, get inv_vec
};

#endif
