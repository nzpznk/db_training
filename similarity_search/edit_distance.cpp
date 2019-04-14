#include <cstdio>
#include <cstring>
#include <utility>
#include <algorithm>
using namespace std;

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

int ed_mat[256][256];

int faster_cal_edit_dist(const char* query, const char* cstr, int threshold) {
    int th = threshold;
    int q_length = strlen(query);
    for (int i = 0; i <= th; ++i) {
        ed_mat[i][0] = i;
    }
    for (int j = 0; j <= th+1; ++j) {
        ed_mat[0][j] = j;
    }
    int c_length = strlen(cstr);
    bool early_stop = false;
    for (int i = 1 ; i <= q_length; ++i) {
        int j_st = 1, j_ed = i+th;
        if (i-th > 0) {
            j_st = i-th;
            ed_mat[i][j_st-1] = th+1;
        }
        if (j_ed < c_length) {
            ed_mat[i][j_ed+1] = th+1;
        } else {
            j_ed = c_length;
        }
        int row_min = th + 1;
        for (int j = j_st; j <= j_ed; ++j) {
            ed_mat[i][j] = min({
                ed_mat[i-1][j] + 1, 
                ed_mat[i][j-1] + 1, 
                ed_mat[i-1][j-1] + (query[i-1] == cstr[j-1] ? 0 : 1)
            });
            row_min = min(ed_mat[i][j], row_min);
        }
        if (row_min > th) {
            early_stop = true;
            break;
        }
    }
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            printf("%d ", ed_mat[i][j]);
        }
        printf("\n");
    }
    return ed_mat[q_length][c_length];
}

int main() {
    printf("%d\n", cal_edit_distance("sitting", "kitten", 7, 6));
    printf("%d\n", faster_cal_edit_dist("kitten", "sitting", 2));
    printf("%d\n", faster_cal_edit_dist("sitting", "kitten", 2));
    return 0;
}
