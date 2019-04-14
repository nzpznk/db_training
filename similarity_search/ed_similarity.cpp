#include <iostream>
#include <algorithm>
#include <vector>

using namespace std;

// given query, term, and k. query and term is similar iff. ed(query, term) <= k
bool is_similar(const char* query, const char* term, int len1, int len2, int k) {
    if (std::abs(len1 - len2) > k) return false;
    ++len1;
    ++len2;
    int** mat = new int*[len1];
    for (int i = 0; i < len1; ++i) {
        mat[i] = new int[len2];
    }
    for (int i = 0; i <= k; ++i) {
        mat[i][0] = i;
        mat[0][i] = i;
    }
    for (int i = 1; i < len1; ++i) {
        for (int j = std::max(i-k, 0); j < std::min(i+k, len2); ++j) {
            int m1 = 
        }
    }
    return true;
}

int main() {
    return 0;
}
