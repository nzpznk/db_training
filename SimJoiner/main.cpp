#include "SimJoiner.h"

using namespace std;

int main(int argc, char **argv) {
    SimJoiner joiner;

    vector<EDJoinResult> resultED;
    vector<JaccardJoinResult> resultJaccard;

    unsigned edThreshold = 2;
    double jaccardThreshold = 0.85;

    // joiner.joinJaccard(argv[1], argv[2], jaccardThreshold, resultJaccard);
    // joiner.joinED(argv[1], argv[2], edThreshold, resultED);
    // joiner.joinJaccard("argv[1]", "argv[2]", jaccardThreshold, resultJaccard);
    joiner.joinED("./dblp-200K.txt", "./dblp-200K.txt", 3, resultED);
    // joiner.joinED("./sample.txt", "./sample.txt", 3, resultED);
    cout << resultED.size() << endl;
    // for (int i = 0; i < resultED.size(); ++i) {
    //     cout << resultED[i].id1 << ", " << resultED[i].id2 << endl;
    // }
    return 0;
}
