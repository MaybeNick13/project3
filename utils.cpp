#include "utils.h"
#include <vector>
#include <list>
#include <iostream>
#include <cmath>
#include <fstream>
#include <queue>


using namespace std;

 int W = 6;
 int k = 8;
 int L = 10;
 int N = 50;
 int E=40;
 int R = 4;
 int GraphN = 5;
 int NumImages;
 int ImageSize =784;
 int TableSize;
 unsigned long M= 4294967291;
 int ck=14;
 int cM=10;
 int cProbes=2; 
// Implementation of myMap

myMap::myMap() {
    table.resize(TableSize);
}

void myMap::insert(unsigned int key, int value) {
    struct myPair currPair = {key, value};
    table[key].push_back(currPair);
}

list<int> myMap::get(unsigned int key) {
    list<int> values;
    for (const auto& currPair : table[key]) {
        if (currPair.key == key) {
            values.push_back(currPair.pos);
        }
    }
    return values;
}

void myMap::create_logfile() {
    ofstream lgfile("logfile.txt");
    int total = 0;
    for (int i = 0; i < TableSize; i++) {
        if(table[i].size() > 0)
            lgfile << "Bucket " << i << " size:\t" << table[i].size() << endl;
        total += table[i].size();
    }
    lgfile << "Total number of points in table is " << total << endl;
}

myfMap::myfMap() {
    table.resize(TableSize);
}

void myfMap::insert(unsigned long key, int value) {
    struct myfPair currPair = {key, value};
    table[key].push_back(currPair);
}

list<int> myfMap::get(unsigned long key) {
    list<int> values;
    cout << " key is " << key << " table size is " << table.size() <<endl;
    for (const auto& currPair : table[key]) {
        if (currPair.key == key) {
            values.push_back(currPair.pos);
        }
    }
    return values;
}

void myfMap::display() {
    for (int i = 0; i < 100; i++) {
        std::cout << "Bucket " << i << ": ";
        for (const myfPair& myPair : table[i]) {
            std::cout << "(" << myPair.key << ", " << myPair.pos << ") ";
        }
        std::cout << std::endl;
    }
}
// Implementation of Node

Node::Node() {
    image = vector<char>(ImageSize);
    IDS = vector<unsigned int>(L, -1);
}


int findSmallestGreater(const vector<kmean_node>& vec, int n) {
    int left = 0;
    int right = vec.size() - 1;
    int result = -1; // Initialize with a default value, in case no such element is found.

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (vec[mid].key > n) {
            result = mid; // Update the result with the current candidate.
            right = mid - 1;  // Continue searching in the left half.
        } else {
            left = mid + 1;   // Discard the left half.
        }
    }

    return result;
}

// Implementation of euclidean_distance

float euclidean_distance(const Node& a, const Node& b) {
    float result = 0.0;
    float sum = 0.0;
    float x;

    for (int i = 0; i < ImageSize; i++) {
        x = (unsigned char)a.image[i] - (unsigned char)b.image[i];
        sum += x * x;
    }

    result = sqrt(sum);
    return result;
}

float euclidean_distance(const vector<float> &a, const vector<float> &b) {
    float result = 0.0;
    float sum = 0.0, x;

    for (int i = 0; i < ImageSize; i++) {
        x = a[i] - b[i];
        sum += x * x;
    }

    result = sqrt(sum);
    return result;
}

float euclidean_distance(const Node& a, const vector<float> &b) {
    float result = 0.0;
    float sum = 0.0, x;

    for (int i = 0; i < ImageSize; i++) {
        x = (unsigned char)a.image[i] - b[i];
        sum += x * x;
    }

    result = sqrt(sum);
    return result;
}

float sqr_euclidean_distance_points(const vector<char> &a, const vector<float> &b){
    float sum = 0.0, x;

    for (int i = 0; i < ImageSize; i++) {
        x = (unsigned char)a[i] - b[i];
        sum += x * x;
    }

    return sum;
}

bool compare::operator()(const pair_dist_pos &a, const pair_dist_pos &b) {
    return a.distance <= b.distance;
}

priority_queue<pair_dist_pos, vector<pair_dist_pos>, compare> calculateDistances(Node* array, Node query){
    priority_queue < pair_dist_pos, vector < pair_dist_pos > , compare > distances;
    pair_dist_pos furthest;
    for (int j = 0; j < NumImages; j++) {
        float dist = euclidean_distance(array[j], query);
        if(dist == 0)
            continue;
        pair_dist_pos pair = {
            dist,
            j
        }; //xrisimopoioume to struct myPair, afti ti fora dinontas sti thesi tou key tin apostasi, gia kaliteri xrisi xorou
        if (j < GraphN) {
            distances.push(pair);

        } else {
            furthest = distances.top();
            if (pair.distance < furthest.distance) {
            distances.pop();
            distances.emplace(pair);
            }
        }
    }
    return distances;
}

priority_queue<pair_dist_pos, vector<pair_dist_pos>, compare> calculateDistances(Node* array, Node query, int Num){
    priority_queue < pair_dist_pos, vector < pair_dist_pos > , compare > distances;
    pair_dist_pos furthest;
    for (int j = 0; j < NumImages; j++) {
        float dist = euclidean_distance(array[j], query);
        if(dist == 0)
            continue;
        pair_dist_pos pair = {
            dist,
            j
        }; //xrisimopoioume to struct myPair, afti ti fora dinontas sti thesi tou key tin apostasi, gia kaliteri xrisi xorou
        if (j < Num) {
            distances.push(pair);

        } else {
            furthest = distances.top();
            if (pair.distance < furthest.distance) {
            distances.pop();
            distances.emplace(pair);
            }
        }
    }
    return distances;
}


char* get_str_of_option(char** begin, char** end, const string& option){
	char** name = find(begin, end, option); 
	if(name != end && ++name != end)
		return *name;
	return 0;
}

bool check_if_option_exists(char** begin, char** end, const string& option){
	char** name = find(begin, end, option);
	if(name != end)
		return true;
	return false;
}

int get_num_from_line(string line, int del_pos){

	int i = 0;
	char anum[20];
	while(line[++del_pos] != '\n' && line[del_pos] != '\0'){
		if(line[del_pos != ' ']){
			anum[i] = line[del_pos];
			i++;
		}
	}
	anum[i] = '\0';

	return atoi(anum);

}

float euclidean_distance(const MRNG_Node& a, const vector<float> &b) {
    float result = 0.0;
    float sum = 0.0, x;

    for (int i = 0; i < ImageSize; i++) {
        x = (unsigned char)a.image[i] - b[i];
        sum += x * x;
    }

    result = sqrt(sum);
    return result;
}
float euclidean_distance(const MRNG_Node& a, const MRNG_Node& b) {
    float result = 0.0;
    float sum = 0.0;
    float x;

    for (int i = 0; i < ImageSize; i++) {
        x = (unsigned char)a.image[i] - (unsigned char)b.image[i];
        sum += x * x;
    }

    result = sqrt(sum);
    return result;
}
MRNG_Node::MRNG_Node(){
    image = vector<unsigned char>(ImageSize);
}

priority_queue<pair_dist_pos, vector<pair_dist_pos>, compare> calculateDistances(MRNG_Node* array, MRNG_Node query, int kNN, int num_of_img){
    priority_queue < pair_dist_pos, vector < pair_dist_pos > , compare > distances;
    pair_dist_pos furthest;
    for (int j = 0; j < num_of_img; j++) {
        pair_dist_pos pair = {
            euclidean_distance(array[j], query),
            j
        }; //xrisimopoioume to struct myPair, afti ti fora dinontas sti thesi tou key tin apostasi, gia kaliteri xrisi xorou
        if (j < kNN) {
            distances.push(pair);

        } else {
            furthest = distances.top();
            if (pair.distance < furthest.distance) {
            distances.pop();
            distances.emplace(pair);
            }
        }
    }
    return distances;
}