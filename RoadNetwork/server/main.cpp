#include <vector>
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <utility>
#include <ctime>
#include "json.hpp"
#include "httplib.h"
#include "GPTree.h"
#include "Position.h"
#include "Car.h"

using namespace std;
using json = nlohmann::json;
using namespace httplib;

const char* gtree_dat_file = "../data/GP_Tree.data";
const char* node_file = "../data/road_new.cnode";
const char* edge_file = "../data/road.nedge";
const char* car_file = "../data/car.txt";

vector<Car> carlist;
vector<Position> nodelist;

/**
 * construct or load G-Tree, load all cars
 */
void prepare() {
    if (ifstream(gtree_dat_file).good()) {
        load_gtree(gtree_dat_file);
        cout << "load_end" << endl;
    } else {
        cout << "build from node and edge file" << endl;
        if (ifstream(node_file).good() && ifstream(edge_file).good()) {
            construct_gtree(edge_file, node_file);
            cout << "build finished" << endl;
            save_gtree(gtree_dat_file);
            cout << "gtree saved to: " << gtree_dat_file << endl;
        } else {
            cout << "cannot find edge file or node file" << endl;
        }
    }

    cout << "reading node file" << endl;
    ifstream node_fp(node_file);
    copy(istream_iterator<Position>(node_fp), istream_iterator<Position>(), back_inserter(nodelist));
    node_fp.close();
    cout << "last node: " << nodelist.back() << endl;
    cout << "node number: " << nodelist.size() << endl;

    cout << "reading car file" << endl;
    ifstream car_fp(car_file);
    copy(istream_iterator<Car>(car_fp), istream_iterator<Car>(), back_inserter(carlist));
    car_fp.close();
    cout << "last car: " << carlist.back() << endl;
    cout << "car number: " << carlist.size() << endl;
}

void get_customer_order(Car& car) {
    int n = car.cust_num;
    for (int i = 0; i < n; ++i) {
        car.mat[4][i] = car.mat[i][4] = network_dist(car.p.id, car.dst_list[i].id);
    }
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < i; ++j) {
            car.mat[i][j] = car.mat[j][i] = network_dist(car.dst_list[i].id, car.dst_list[j].id);
        }
    }

    if (n == 0) {
        car.d1 = 0x7fffffff;
    } else {
        int min_len = 0x7fffffff, tot_len = 0;
        vector<int> ord;
        for (int i = 0; i < n; ++i) {
            ord.push_back(i);
        }
        do {
            tot_len = car.mat[4][ord[0]];
            for (int i = 0; i < n-1; ++i) {
                tot_len += car.mat[ord[i]][ord[i+1]];
            }
            if (tot_len < min_len) {
                min_len = tot_len;
                car.order = ord;
            }
        } while(next_permutation(ord.begin(), ord.end()));
        car.d1 = min_len;
    }
}

int mat[5][5];

void find_car(int ori_id, int dst_id, vector<pair<int, int>>& cars) {
    cars.clear();
    int d4 = network_dist(ori_id, dst_id);
    vector<pair<int, int>> candidates;
    for (auto& car : carlist) {
        if (car.cust_num == 4) {
            continue;
        }
        int d2 = network_dist(ori_id, car.p.id);
        if(d2 > 10000) {
            continue;
        } else {
            candidates.emplace_back(d2, car.cid);
        }
    }

    sort(candidates.begin(), candidates.end());

    for (auto& d2_cid : candidates) {
        if (cars.size() >= 5) break;
        auto& car = carlist[d2_cid.second];
        int d2 = d2_cid.first;
        vector<int> order;
        int n = car.cust_num + 1;
        copy(&car.mat[0][0], &car.mat[0][0]+5*5, &mat[0][0]);
        // mat[4][n-1] = mat[n-1][4] = network_dist(dst_id, car.p.id);
        // for (int i = 0; i < n-1; ++i) {
        //     mat[i][n-1] = mat[n-1][i] = network_dist(dst_id, car.dst_list[i].id);
        // }

        mat[4][n-1] = mat[n-1][4] = d4;
        for (int i = 0; i < n-1; ++i) {
            mat[i][4] = mat[4][i] = network_dist(ori_id, car.dst_list[i].id);
            mat[i][n-1] = mat[n-1][i] = network_dist(dst_id, car.dst_list[i].id);
        }

        int d3 = 0x7fffffff;
        for (int i = 0; i < n; ++i) {
            order.push_back(i);
        }
        do {
            int tot_len = mat[4][order[0]];
            for (int i = 0; i < n-1; ++i) {
                tot_len += mat[order[i]][order[i+1]];
            }
            if (tot_len < d3) {
                d3 = tot_len;
            }
        } while(next_permutation(order.begin(), order.end()));

        if (d3 - d4 <= 10000 && d2 + d3 - car.d1 <= 10000) {
            cars.emplace_back(d2, car.cid);
        }
    }
}

void get_route_old(int st, int ed, int car_id, vector<int>& path, vector<int>& pass_posi) {
    path.clear();
    pass_posi.clear();
    const Car& car = carlist[car_id];
    int a = car.p.id;
    vector<int> tmp;
    path.push_back(a);
    for (int x : car.order) {
        int px = car.dst_list[x].id;
        pass_posi.push_back(px);
        find_path(a, px, tmp);
        for (int idx = 1; idx < tmp.size(); ++idx) {
            path.push_back(tmp[idx]);
        }
        a = px;
    }
}

void get_route_new(int st, int ed, int car_id, vector<int>& path, vector<int>& pass_posi) {
    path.clear();
    pass_posi.clear();
    auto& car = carlist[car_id];
    vector<int> order;
    int n = car.cust_num + 1;
    int mm[5][5];
    copy(&car.mat[0][0], &car.mat[0][0]+5*5, &mm[0][0]);

    mm[4][n-1] = mm[n-1][4] = network_dist(st, ed);
    for (int i = 0; i < n-1; ++i) {
        mm[i][4] = mm[4][i] = network_dist(st, car.dst_list[i].id);
        mm[i][n-1] = mm[n-1][i] = network_dist(ed, car.dst_list[i].id);
    }

    vector<int> ord;
    int d3 = 0x7fffffff;
    for (int i = 0; i < n; ++i) {
        order.push_back(i);
    }
    do {
        int tot_len = mm[4][order[0]];
        for (int i = 0; i < n-1; ++i) {
            tot_len += mm[order[i]][order[i+1]];
        }
        if (tot_len < d3) {
            d3 = tot_len;
            ord = order;
        }
    } while(next_permutation(order.begin(), order.end()));

    vector<int> tmp;
    int a = st;
    path.push_back(a);
    for (int x : ord) {
        int px = 0;
        if (x == n-1) {
            px = ed;
        } else {
            px = car.dst_list[x].id;
        }
        pass_posi.push_back(px);
        find_path(a, px, tmp);
        for (int idx = 1; idx < tmp.size(); ++idx) {
            path.push_back(tmp[idx]);
        }
        a = px;
    }
}

void get_nearest();

int main()
{
    clock_t st, ed;
    prepare();
    st = clock();
    for (auto& car : carlist) {
        get_customer_order(car);
    }
    ed = clock();
    cout << "car init finished: " << (ed-st)*1.0 / CLOCKS_PER_SEC << "s" << endl;

    Server svr;
    
    svr.set_base_dir("../frontend");

    svr.Get(R"(/node_file)", [&](const Request &req, Response &res) {
        std::cout << "getting node file" << std::endl;
        std::ifstream nfp(node_file);
        std::istreambuf_iterator<char> st(nfp), ed;
        std::string content(st, ed);
        res.set_content(json({{"node_file", content}}).dump(), "application/json");
    });

    svr.Get(R"(/car_file)", [&](const Request &req, Response &res) {
        std::cout << "getting car file" << std::endl;
        std::ifstream cfp(car_file);
        std::istreambuf_iterator<char> st(cfp), ed;
        std::string content(st, ed);
        res.set_content(json({{"car_file", content}}).dump(), "application/json");
    });

    svr.Post("/query_carlist", [](const auto &req, auto &res) {
        std::cout << req.body << std::endl;
        auto req_body = json::parse(req.body);
        std::cout << req_body.dump() << std::endl;
        int ori_id = req_body["stid"], dst_id = req_body["edid"];
        vector<pair<int, int>> ans;
        clock_t st = clock();
        find_car(ori_id, dst_id, ans);
        clock_t ed = clock();
        cout << (ed - st) * 1.0 / CLOCKS_PER_SEC << " seconds" << endl;
        auto ret_json = json::array();
        for (auto p : ans) {
            ret_json.push_back(json({{"dist", p.first}, {"car_id", p.second}, {"cust_num", carlist[p.second].cust_num}}));
        }
        auto ret_body = ret_json.dump();
        std::cout << "return str: " << ret_body << std::endl;
        res.set_content(ret_body, "application/json");
    });

    svr.Post("/query_car_path", [](const auto &req, auto &res) {
        std::cout << req.body << std::endl;
        auto req_body = json::parse(req.body);
        std::cout << req_body.dump() << std::endl;
        int ori_id = req_body["stid"], dst_id = req_body["edid"], car_id = req_body["carid"];
        int old = req_body["is_old"];

        vector<int> path, passenger;
        if (old == 1) {
            get_route_old(ori_id, dst_id, car_id, path, passenger);
        } else {
            get_route_new(ori_id, dst_id, car_id, path, passenger);
        }

        auto ret_json = json({{"path", json(path)}, {"passenger", json(passenger)}});
        auto ret_body = ret_json.dump();
        // std::cout << "return str: " << ret_body << std::endl;
        res.set_content(ret_body, "application/json");
    });

    std::cout << "server start Listening on port 8080" << std::endl;
    svr.listen("localhost", 8080);
    return 0;
}
