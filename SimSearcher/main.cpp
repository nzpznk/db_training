#include "SimSearcher.h"
#include <fstream>

using namespace std;

int main(int argc, char **argv)
{
	SimSearcher searcher;
	vector<pair<unsigned, unsigned> > resultED;
	vector<pair<unsigned, double> > resultJaccard;
	unsigned q = 4, edThreshold = 2;
	double jaccardThreshold = 0.85;

	searcher.createIndex("./dblp-200K", q);
    cout << "index created" << endl;
    ifstream fp = ifstream("./dblp-200K");
    string query;
    while(getline(fp, query)) {
        searcher.searchED(query.c_str(), edThreshold, resultED);
    }
    while(getline(fp, query)) {
        searcher.searchJaccard(query.c_str(), jaccardThreshold, resultJaccard);
    }
	// searcher.searchJaccard("query", jaccardThreshold, resultJaccard);
    // for (int i = 0; i < 1000; ++i) {
    // }
    // cout << "ed result:" << endl;
    // for (auto val : resultED) {
    //     cout << val.first << " " << val.second << endl;
    // }
    // cout << "jac result:" << endl;
    // for (auto val : resultJaccard) {
    //     cout << val.first << " " << val.second << endl;
    // }
	return 0;
}
