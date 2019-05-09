#include "SimJoiner.h"

using namespace std;

const int DATAPOOL_SIZE = 80000000;
static char input1[DATAPOOL_SIZE];
static char input2[DATAPOOL_SIZE];
static int segLenTable[300][4];
static int segStartTable[300][4];
static int ed_mat[256][256];

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

int SimJoiner::joinJaccard(const char *filename1, const char *filename2, double threshold, vector<JaccardJoinResult> &result) {
    result.clear();
    return SUCCESS;
}

int SimJoiner::joinED(const char *filename1, const char *filename2, unsigned threshold, vector<EDJoinResult> &result) {
    result.clear();
    tau = threshold;
    initSegLenTable(tau);
    readDataset(filename1, input1, strSet1, strLen1);
    readDataset(filename2, input2, strSet2, strLen2);
    createIndex(strSet2, strLen2);
    passJoin(strSet1, strLen1, strSet2, strLen2, result);
    return SUCCESS;
}

inline vector<string> SimJoiner::substringSelection(const char* str, int len, int l, int t) {
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
        set<int> candidateSet;
        for (int l = len - tau; l <= len + tau; ++l) {
            for (int t = 0; t <= tau; ++t) {
                // sub string selection
                auto substringSet = substringSelection(str, len, l, t);
                // add candidates
                InvIndexMap& indexMap = inv[l][t];
                InvIndexMap::iterator iter;
                for (string substring : substringSet) {
                    if ((iter = indexMap.find(substring)) != indexMap.end()) {
                        candidateSet.insert(iter->second.begin(), iter->second.end());
                    }
                }
            }
        }
        // verification
        verify_ed_dist(i, str, strs2, lens2, candidateSet, result);
    }
}

inline void SimJoiner::verify_ed_dist( int first_id, 
                                const char* query, 
                                vector<const char*>& strs2, 
                                vector<int>& lens2, 
                                const set<int>& candidates, 
                                vector<EDJoinResult>& result)
{
    int q_length = strlen(query);
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

void SimJoiner::createIndex(vector<const char*>& strs, vector<int>& lens) {
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
    // for (int i = 0; i < 300; ++i) {
    //     for (int j = 0; j < 4; ++j) {
    //         if (inv[i][j].size() <= 0) continue;
    //         cout << "len = " << i << ", t = " << j << endl;
    //         for (auto p : inv[i][j]) {
    //             cout << p.first << ": ";
    //             for (auto id : p.second) {
    //                 cout << id << " ";
    //             }
    //             cout << endl;
    //         }
    //     }
    // }
}
