#include <map>
#include <list>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <functional>
using namespace std;

int main() {
    vector<int>* a = new vector<int>(10);
    for (auto it = a->begin(); it != a->end(); ++it) {
        cout << *it << endl;
    }
    printf("memcmp: %d\n", memcmp("aaa", "aaa", 3));
    int x = 2 - 10;
    cout << x << "testing mod: " << x % 5 << endl;
    cout << sizeof(vector<int>) << endl;
    cout << sizeof(vector<vector<int> >) << endl;
    cout << sizeof(vector<int>&) << endl;
    map<unsigned, int> hash2int;
    hash2int[3] = 1;
    cout << "testing map: " << hash2int[3]++ << " ";
    cout << hash2int[3] << endl;
    vector<vector<int> > va = {{1, 2, 3}};
    vector<reference_wrapper<vector<int>>> b;
    b.push_back(va[0]);
    b[0].get().push_back(5);
    for (int x : va[0]) {
        cout << x << " ";
    }
    cout << endl;
    cout << "reference size: " << sizeof(reference_wrapper<vector<int>>) << endl;
    list<int> la = {1, 2, 3, 4, 5, 6, 7, 7, 9, 10};
    for (auto it = la.begin(); it != la.end();) {
        if ((*it) % 2 == 0) {
            it = la.erase(it);
        } else {
            ++it;
        }
    }
    for (auto v : la) {
        cout << v << " ";
    }
    cout << endl;
    cout << "size of iterator: " << sizeof(vector<int>::iterator) << endl;
    vector<int> xx = {1, 2, 3, 4, 5};
    vector<int> yy = {2, 4, 6, 8, 10};
    vector<int> zz = {11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    // vector<int> zz;
    zz.clear();
    set_intersection(xx.begin(), xx.end(), yy.begin(), yy.end(), back_inserter(zz));
    for (int e : zz) {
        cout << e << " ";
    }
    cout << endl;
    for (auto& e : zz) {
        e += 3;
    }
    for (int e : zz) {
        cout << e << " ";
    }
    return 0;
}
