#include "hashing.h"


hashfunc::hashfunc() {}

hashfunc::hashfunc(Node* nodes) {   
    // random uniformly distributed number generator
    random_device rd{};
    mt19937 gen{rd()};
    normal_distribution<float> myRand(0.0, 10.0);


    proj.resize(ImageSize);
    float length = 0;
    for (int i = 0; i < ImageSize; i++) {
        // getting the projection vector
        proj[i] = myRand(gen); 
        if(proj[i] < 0)
            proj[i] = -proj[i];
        length += proj[i] * proj[i];
    }
    length = sqrt(length);

    for (int i = 0; i < ImageSize; i++) {

        proj[i] = proj[i] / length; // regularization
    }

    uniform_real_distribution<float> myRealRand(0.0, W - 0.00000001);
    t = myRealRand(gen); 

    hashvalues = vector<int>(NumImages);

    for (int i = 0; i < NumImages; i++) {
        float innerprod = 0;
        for (int j = 0; j < ImageSize; j++) {
            innerprod += proj[j] * (float)((unsigned char)(nodes[i].image[j]));
        }


        hashvalues[i] = (innerprod + t) / W;
        if(innerprod > 2000){
        }

        // POSSIBLE FAULT HERE, SHOULD DEBUG TO BE SURE
    }
}

float hashfunc::hash(Node node) {        //function gia to hashing to query meta tin arxikopiisi olou tou table
    float innerprod = 0;
    for (int j = 0; j < ImageSize; j++) {
        innerprod += proj[j] * (float)((unsigned char)(node.image[j]));
    }

    return ((innerprod + t) / W);
}

float hashfunc::hash(vector<float> center) {
    float innerprod = 0;
    for (int j = 0; j < ImageSize; j++) {
        innerprod += proj[j] * center[j];
    }
    return ((innerprod + t) / (float)W);
}

vector<int> hashfunc::get_values() {
    return hashvalues;
}

hashtable::hashtable() {}

hashtable::hashtable(int num, Node** im) : images(im), id_num(num) {
    // vector for storing hvalues of hashtables
    vector<vector<int>> hval(k);
    factors = new int*;
    *factors = new int[k];
    srand((unsigned)time(NULL));
    unsigned int ID [NumImages];
    unsigned int gHash [NumImages];
    hashfuncs = new hashfunc*[k];

    for (int i = 0; i < k; i++) {                   //ftiaxnei k hashfuncs kai apothikevi tis times tous
        hashfuncs[i] = new hashfunc(*images);
        // returns projections of all vectors in a random line
        hval[i] = hashfuncs[i]->get_values();
        // random number to multiply the hvals
        (*factors)[i] = (rand() % 9) + 1;
    }

    for (int i = 0; i < NumImages; i++) {
        ID[i] = 0;
    }

    for (int i = 0; i < NumImages; i++) {
        for (int j = 0; j < k; j++) {
            ID[i] +=(*factors)[j] * hval[j][i];
        }
        ID[i] = ID[i] % M;
        (*images)[i].IDS[id_num] = ID[i];
    }

    for (int i = 0; i < NumImages; i++) {
        gHash[i] = ID[i] % (TableSize);
        map.insert(gHash[i], i);
    }
}

void hashtable::hash(Node* im) {
    vector<int> hval(k);
    int ID = 0;
    for (int i = 0; i < k; i++) {
        float hfval = hashfuncs[i]->hash(*im);
        int f = (*factors)[i];
        ID += hfval * f;
    }
    ID = ID % M;
    im->IDS[id_num] = ID;
}

void hashtable::hash(vector<float> centroid, vector<int> *IDS){
    vector<int> hval(k);
    int ID = 0;
    for (int i = 0; i < k; i++) {
        ID += hashfuncs[i]->hash(centroid);
    }
    ID = ID % M;
    (*IDS)[id_num] = ID;
}

hashtable :: ~hashtable(){
    delete[] (*factors);
    delete[] factors;
    for(int i = 0; i < k; i++){
        delete hashfuncs[i];
    }
    delete[] hashfuncs;
    
}

void hashtable::print_table(){
    map.create_logfile();
}

list<int> hashtable::get_bucket(unsigned int bucket_num) {
    return map.get(bucket_num);
}
