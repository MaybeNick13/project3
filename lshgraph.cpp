#include <iostream>

#include <fstream>

#include <vector>

#include <random>

#include <numeric>

#include <ctime>

#include <unordered_map>

#include <list>

#include <set>

#include <queue>

#include <chrono>

#include "utils.h"

#include "hashing.h"

#include <cstring>

using namespace std;

vector<vector<pair_dist_pos>> create_graph(Node* array, int N) {
    string output = "output.txt";
    ofstream outfile;
    outfile.open(output);
    bool repeat;
    repeat = false;
    hashtable* tables[L];
    for (int j = 0; j < L; j++) {
        tables[j] = new hashtable(j, & array); //arxikopiisi ton hashtables
    }
    vector<vector<pair_dist_pos>> graph (NumImages,vector<pair_dist_pos>(N));
    for (int i = 0; i < NumImages; i++) {
        auto startMethod = chrono::high_resolution_clock::now();
        pair_dist_pos furthest;
        set < int > same_ids; ///gia pithanoun gitones
        priority_queue < pair_dist_pos, vector < pair_dist_pos > , compare > nn_pqueue;
        list < int > candidates;
        for (int j = 0; j < L; j++) {
            unsigned int query_id = array[i].IDS[j];
            unsigned int bucket_num = query_id % TableSize;

            candidates = tables[j]->get_bucket(bucket_num); //ikones sto idio bucket
            for (auto itr: candidates) {
                same_ids.insert(itr);
            }
        }
        int f = 0;
        if(same_ids.size() >= GraphN){
            for (auto itr: same_ids) {

                float dist = euclidean_distance(array[itr], array[i]); //diasxizoume ton pinaka me ta same_ids
                pair_dist_pos current {
                    dist,
                    itr
                };
                if (dist!=0){
                    if (f < N) { //an den exoume vrei N stoixeia akoma, aplos vale to mesa
                        nn_pqueue.push(current);
                    } else {
                        furthest = nn_pqueue.top();
                        if (current.distance < furthest.distance) {
                        nn_pqueue.pop();
                        nn_pqueue.emplace(current);
                        }
                    }
                    f++;
                }
            }
            outfile << "\t";
        }
        else{
            outfile << "exh:";
            nn_pqueue = calculateDistances(array, array[i], N);
        }
        auto endMethod = chrono::high_resolution_clock::now();
        chrono::duration < double > durationMethod = endMethod - startMethod;

        for (int j = N - 1; j >= 0; j--) {
            graph[i][j] = nn_pqueue.top();
            nn_pqueue.pop();
        }
        for (int j = 0; j < N; j++) {
            outfile << graph[i][j].pos << " ";
            }
        outfile << endl;
    }
    return graph;
    }