#include "utils.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <cstring>
#include <chrono>



int main(int argc, char * argv[]){
    srand((unsigned)time(NULL));
    string input = "encoded_train_images.dat";
    string queryF = "encoded_test_images.dat";
    string output = "results_brute.txt";
    string inputB="input.dat";
    string queryB="query.dat";
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
        if (strcmp(argv[i], "-N") == 0) {
            GraphN = std::stoi(argv[i + 1]);
        }
        if (strcmp(argv[i], "-qB") == 0) {
            queryB = argv[i + 1];
        }
        if (strcmp(argv[i], "-iB") == 0) {
            inputB = argv[i + 1];
        }
    }

    ifstream images(input);
    ifstream imagesB(inputB);
    ofstream outfile;
    outfile.open(output);
    if (!images.is_open()) {
        cerr << "Failed to open input file" << endl;
        return 1;
    }
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

    ImageSize = rows * columns;
    TableSize = NumImages / 6;

    Node * array = new Node[NumImages+1];
    ImageSize = 784;
    Node * arrayB = new Node[NumImages+1];
    ImageSize = rows * columns;

    imagesB.seekg(16);

     for (int i = 0; i < NumImages; i++) {
        images.read(array[i].image.data(), ImageSize);
        imagesB.read(arrayB[i].image.data(), 784);
    }
    pair_dist_pos * methodResult = new pair_dist_pos[GraphN];
    pair_dist_pos * exhaustiveResult  = new pair_dist_pos[GraphN];
    bool repeat=false;
        do {
        repeat = false;
        double totalMethodDuration = 0.0;
        double totalExhaustiveDuration = 0.0;
        double maf=-1;
        ifstream query(queryF);
        ifstream queryBigStream(queryB);
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
        if (queryImages > 10) {
            queryImages = 10;
        }
        query.seekg(16);
        queryBigStream.seekg(16);
        Node * queries = new Node[queryImages];             // DELETE LATER
        ImageSize = 784;
        Node * queriesB = new Node[queryImages];            // DELETE LATER
        ImageSize = rows * columns;
        for (int i = 0; i < queryImages; i++) {
            query.read(queries[i].image.data(), ImageSize);
            queryBigStream.read(queriesB[i].image.data(), 784);
        }
        for (int i=0; i < queryImages; i++){
            ImageSize = 784;
            auto startExhaustive = chrono::high_resolution_clock::now();
            priority_queue<pair_dist_pos, vector<pair_dist_pos>, compare> distances = calculateDistances(arrayB, queriesB[i], GraphN);
            auto endExhaustive = chrono::high_resolution_clock::now();
            ImageSize=rows*columns;
            chrono::duration < double > durationExhaustive = endExhaustive - startExhaustive;
            totalExhaustiveDuration += durationExhaustive.count();

            
            auto startMethod =  chrono::high_resolution_clock::now();
            priority_queue<pair_dist_pos, vector<pair_dist_pos>, compare> distancesSmall = calculateDistances(array, queries[i], GraphN);
            auto endMethod = chrono::high_resolution_clock::now();
            chrono::duration < double > durationMethod = endMethod - startMethod;
            totalMethodDuration += durationMethod.count();
            ImageSize = 784;
            for (int j = GraphN - 1; j >= 0; j--) {
                exhaustiveResult[j] = distances.top();
                distances.pop();
                methodResult[j] = distancesSmall.top();
                distancesSmall.pop();
                if (j==0)
                    maf += euclidean_distance(arrayB[methodResult[j].pos],queriesB[i])/exhaustiveResult[j].distance;
                } 
                outfile <<"Query" << i <<endl;
            for (int j = 0; j < GraphN; j++) {
                outfile << "Nearest neighbor-" << j + 1 << " " << methodResult[j].pos << endl;
                outfile << "distanceApproximate:" << euclidean_distance(arrayB[methodResult[j].pos], queriesB[i]) << endl;
                outfile << "distanceTrue:" << exhaustiveResult[j].distance << endl;
            }
            ImageSize = rows * columns;
        }
        outfile << "tAverageApproximate:" << (totalMethodDuration/queryImages) << endl;
        outfile << "tAverageTrue:" << (totalExhaustiveDuration/queryImages) << endl;
        outfile <<"MAF"<< maf/queryImages << endl;
        delete[] queries;
        delete[] queriesB;
        cout << "Repeat with different query?[y/n]" << endl;
        string answer;
        cin >> answer;
        if (answer.compare("y") == 0) {
        cout << "Enter query file name" << endl;
        cin >> answer;
        repeat = true;
        queryF = answer;
        }
    }while (repeat == 1);
    delete[] array;
    delete[] arrayB;
    images.close();
    imagesB.close();
    delete[] methodResult;
    delete[] exhaustiveResult;
    return 0;
}