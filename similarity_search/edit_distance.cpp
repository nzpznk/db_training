#include <cstdio>
#include <cstdlib>
#include <algorithm>

int cal_edit_distance(const char* a, const char* b, int len_a, int len_b) {
    ++len_a;
    ++len_b;
    int** mat = new int*[len_a];
    for (int i = 0; i < len_a; ++i) {
        mat[i] = new int[len_b];
    }
    for (int i = 0; i < len_a; ++i) mat[i][0] = i;
    for (int j = 0; j < len_b; ++j) mat[0][j] = j;
    for (int i = 1; i < len_a; ++i) {
        for (int j = 1; j < len_b; ++j) {
            int m1 = mat[i-1][j] + 1;
            int m2 = mat[i][j-1] + 1;
            int m3 = mat[i-1][j-1] + (a[i-1] == b[j-1] ? 0 : 1);
            mat[i][j] = std::min({m1, m2, m3});
        }
    }
    int ans = mat[len_a - 1][len_b - 1];
    for (int i = 0; i < len_a; ++i) {
        delete[] mat[i];
    }
    delete[] mat;
    mat = nullptr;
    return ans;
}

int main() {
    printf("%d", cal_edit_distance("sitting", "kitten", 7, 6));
    return 0;
}
