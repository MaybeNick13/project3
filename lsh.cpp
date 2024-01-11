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

int main(int argc, char * argv[]) {
    string input = "input.dat";
    string queryF = "query.dat";
    string output = "output.txt";
    for (int i = 1; i < argc; i++) { //parsing gia ta input variables
        if (strcmp(argv[i], "-o") == 0) {
        output = argv[i + 1];
        }
        if (strcmp(argv[i], "-d") == 0) {
        input = argv[i + 1];
        }
        if (strcmp(argv[i], "-q") == 0) {
        queryF = argv[i + 1];
        }
        if (strcmp(argv[i], "-k") == 0) {
        k = std::stoi(argv[i + 1]);
        }
        if (strcmp(argv[i], "-L") == 0) {
        L = std::stoi(argv[i + 1]);
        }
        if (strcmp(argv[i], "-N") == 0) {
        N = std::stoi(argv[i + 1]);
        }
        if (strcmp(argv[i], "-R") == 0) {
        R = std::stoi(argv[i + 1]);
        }
    }

    // creating outpuf file
    ofstream outfile;
    outfile.open(output);


    //opening input file
    ifstream images(input);
    if (!images.is_open()) {
        cerr << "Failed to open input.dat" << endl;
        return 1;
    }

    //reading from input file usefull data
    images.seekg(4);
    char buffer[4];
    images.read(buffer, 4);
    NumImages = (static_cast < uint32_t > (static_cast < unsigned char > (buffer[0])) << 24) |
        (static_cast < uint32_t > (static_cast < unsigned char > (buffer[1])) << 16) |
        (static_cast < uint32_t > (static_cast < unsigned char > (buffer[2])) << 8) |
        (static_cast < uint32_t > (static_cast < unsigned char > (buffer[3])));
    int rows, columns;
    images.read(buffer, 4);
    rows = (static_cast < uint32_t > (static_cast < unsigned char > (buffer[0])) << 24) |
        (static_cast < uint32_t > (static_cast < unsigned char > (buffer[1])) << 16) |
        (static_cast < uint32_t > (static_cast < unsigned char > (buffer[2])) << 8) |
        (static_cast < uint32_t > (static_cast < unsigned char > (buffer[3])));
    images.read(buffer, 4);
    columns = (static_cast < uint32_t > (static_cast < unsigned char > (buffer[0])) << 24) |
        (static_cast < uint32_t > (static_cast < unsigned char > (buffer[1])) << 16) |
        (static_cast < uint32_t > (static_cast < unsigned char > (buffer[2])) << 8) |
        (static_cast < uint32_t > (static_cast < unsigned char > (buffer[3])));

    // getting dimensions from data of file
    TableSize = NumImages / 4;
    ImageSize = rows * columns;
    bool repeat;
    // creating array for storing the data
    Node * array = new Node[NumImages + 1];
    // data structures for keeping the results and comparing
    pair_dist_pos * methodResult = new pair_dist_pos[N];
    pair_dist_pos * exhaustiveResult = new pair_dist_pos[N];

    // writing the data to the array
    for (int i = 0; i < NumImages - 1; i++) {
        images.read(array[i].image.data(), ImageSize);
    }
    do {

        repeat = false;
        // opening query file
        ifstream query(queryF);
        if (!query.is_open()) {
        cerr << "Failed to open query.dat" << endl;
        return 1;
        }
        query.seekg(4);
        int queryImages;
        query.read(buffer, 4);
        queryImages = (static_cast < uint32_t > (static_cast < unsigned char > (buffer[0])) << 24) |
        (static_cast < uint32_t > (static_cast < unsigned char > (buffer[1])) << 16) |
        (static_cast < uint32_t > (static_cast < unsigned char > (buffer[2])) << 8) |
        (static_cast < uint32_t > (static_cast < unsigned char > (buffer[3])));
        // this can be removed / changed if we want more queries, done like this for debugging puproses only
        if (queryImages > 10){
            queryImages = 10;
        }
        query.seekg(16);
        // node for storing queries
        Node queries[queryImages];
        // writing query data from file to array
        for (int i = 0; i < queryImages; i++) {
            query.read(queries[i].image.data(), ImageSize);
        }
        // creation of hashtables
        hashtable* tables[L];
        for (int j = 0; j < L; j++) {
            tables[j] = new hashtable(j, & array); //initialization of hashtables
        }

        for (int i = 0; i < queryImages; i++) {

            auto startMethod = chrono::high_resolution_clock::now();
            pair_dist_pos furthest;
            int query_pos = NumImages ;
            array[query_pos].image = queries[i].image;
            set < int > same_ids; ///gia pithanoun gitones
            priority_queue < pair_dist_pos, vector < pair_dist_pos > , compare > nn_pqueue;
            list < int > candidates;
            set < int > rNeighbors;
            for (int j = 0; j < L; j++) {
                tables[j]->hash( & queries[i]);
                unsigned int query_id = queries[i].IDS[j];

                unsigned int bucket_num = query_id % (TableSize);

                candidates = tables[j]->get_bucket(bucket_num); //ikones sto idio bucket

                for (auto itr: candidates) {
                    same_ids.insert(itr);
                }

            }
            cout << "Found " << same_ids.size() << " candidates for query" << endl;
            int f = 0;
            for (auto itr: same_ids) {
                float dist = euclidean_distance(array[itr], queries[i]); //diasxizoume ton pinaka me ta same_ids
                pair_dist_pos current {
                    dist,
                    itr
                };

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
            auto endMethod = chrono::high_resolution_clock::now();
            chrono::duration < double > durationMethod = endMethod - startMethod;
            // R RANGE SEARCH
            for (int j = 0; j < L; j++) {
                tables[j]->hash( & queries[i]);
                unsigned int query_id = queries[i].IDS[j];

                unsigned int bucket_num = query_id % (TableSize);
                candidates = tables[j]->get_bucket(bucket_num);

                for (auto itr: candidates) {
                    if (euclidean_distance(array[itr], queries[i]) < R) {
                        rNeighbors.insert(itr);
                    }
                }

            }
            // R RANGE SEARCH
            priority_queue < pair_dist_pos, vector < pair_dist_pos > , compare > distances;
            auto startExhaustive = chrono::high_resolution_clock::now();
            distances = calculateDistances(array, queries[i]);
            auto endExhaustive = chrono::high_resolution_clock::now();
            chrono::duration < double > durationExhaustive = endExhaustive - startExhaustive;
            for (int j = N - 1; j >= 0; j--) {
                exhaustiveResult[j] = distances.top();
                distances.pop();
                methodResult[j] = nn_pqueue.top();
                nn_pqueue.pop();
            }

            for (int j = 0; j < N; j++) {
                outfile << "Nearest neighbor-" << j + 1 << " " << methodResult[j].pos << endl;
                outfile << "distanceLSH:" << methodResult[j].distance << endl;
                outfile << "distanceTrue:" << exhaustiveResult[j].distance << endl;
            }
            outfile << "tLSH:" << durationMethod.count() << endl;
            outfile << "tTrue:" << durationExhaustive.count() << endl;
            outfile << "R-near neighbors:" << endl;
            for (const auto & element: rNeighbors) {
                outfile << element << endl;
            }
        }

        // Close the input file
        cout << "Repeat with different query?[y/n]" << endl;
        string answer;
        cin >> answer;
        if (answer.compare("y") == 0) {
        cout << "Enter query file name" << endl;
        cin >> answer;
        repeat = true;
        queryF = answer;
        }
        for(int t = 0; t < L; t++){
            delete tables[t];
        }
    } while (repeat == 1);
    delete[] array;
    images.close();
    delete[] methodResult;
    delete[] exhaustiveResult;

    return 0;
}