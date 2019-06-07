#ifndef __GPTREE_H__
#define __GPTREE_H__

#include <vector>

void construct_gtree(const char* edge_file, const char* node_file);

void save_gtree(const char* output_file);

void load_gtree(const char* tree_file);

int network_dist(int ori_id, int dst_id);

int find_path(int ori_id, int dst_id, std::vector<int>& path);

int get_node_num();

int get_edge_num();

#endif
