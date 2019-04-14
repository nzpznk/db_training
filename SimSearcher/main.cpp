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

	searcher.createIndex("./dblp-200K.txt", q);
    cout << "index created" << endl;
    ifstream fp = ifstream("./dblp-200K.txt");
    string query;
    // searcher.searchJaccard("carsten westendorff j rgen hoffmann german gomez roman tina herberts siegmar reinert interindividueller vergleich der genauigkeit navigations assistierter implantatbettbohrungen mit konventionell gef hrten freihandbohrungen am unterkiefermodell", jaccardThreshold, resultJaccard);
    // int cnt = 0;
    while(getline(fp, query)) {
        searcher.searchED(query.c_str(), edThreshold, resultED);
        searcher.searchJaccard(query.c_str(), jaccardThreshold, resultJaccard);
        // if(resultED[0].first != cnt) {
        //     cout << query << endl;
        // }
        // cnt++;
    }
    // while(getline(fp, query)) {
    //     searcher.searchJaccard(query.c_str(), jaccardThreshold, resultJaccard);
    //     // if(resultJaccard[0].first != cnt) {
    //     //     cout << query << endl;
    //     // }
    //     // cnt++;
    // }
	// searcher.searchJaccard("query", jaccardThreshold, resultJaccard);
    // for (int i = 0; i < 1000; ++i) {
    // }
    // cout << "ed result:" << endl;
    // for (auto val : resultED) {
    //     cout << val.first << " " << val.second << endl;
    // }
    cout << "jac result:" << endl;
    for (auto val : resultJaccard) {
        cout << val.first << " " << val.second << endl;
    }
	return 0;
}
