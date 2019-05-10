#include "SimJoiner.h"

using namespace std;

int main(int argc, char **argv) {
    SimJoiner joiner;

    vector<EDJoinResult> resultED;
    vector<JaccardJoinResult> resultJaccard;

    unsigned edThreshold = 2;
    double jaccardThreshold = 0.75;

    // joiner.joinJaccard(argv[1], argv[2], jaccardThreshold, resultJaccard);
    // joiner.joinED(argv[1], argv[2], edThreshold, resultED);
    // cout << resultJaccard.size() << endl;
    // joiner.joinED("./dblp-200K.txt", "./dblp-200K.txt", 3, resultED);
    joiner.joinJaccard("./dblp-200K.txt", "./dblp-200K.txt", jaccardThreshold, resultJaccard);
    // joiner.joinJaccard("./sample.txt", "./sample.txt", 0.75, resultJaccard);
    // cout << resultED.size() << endl;
    // for (int i = 0; i < resultED.size(); ++i) {
    //     cout << resultED[i].id1 << ", " << resultED[i].id2 << endl;
    // }
    cout << resultJaccard.size() << endl;
    // for (int i = 0; i < resultJaccard.size(); ++i) {
    //     cout << "J " << resultJaccard[i].id1 << " " << resultJaccard[i].id2 << endl;
    // }
    return 0;
}
