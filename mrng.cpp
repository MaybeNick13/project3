#include "mrng.h"

using namespace std;

void search_graph_MRNG :: initialize_distances(){
    // Ininitializing the distances data structure
    distances = vector<vector<float>>(node_popullation - 1);
    cout << "Initializing distances" << endl;

    for(int i = 0; i < node_popullation - 1; i++){
        cout << i << endl ;
        distances[i] = vector<float> (node_popullation- 1 - i);
        for(int j = i + 1; j < node_popullation; j++){
            distances[i][j - i - 1] = euclidean_distance((*nodes)[i] ,(*nodes)[j]);
        }
    }
}

float search_graph_MRNG :: get_dist(int point_a, int point_b){
    // Given 2 ids of nodes, return their distance
    int big, small;
    if(point_a > point_b){
        big = point_a;
        small = point_b;
    }else if(point_a < point_b){
        big = point_b;
        small = point_a;
    }else if(point_a  == point_b){
        return -1;
    }

    return distances[small][big - small - 1];
}

void search_graph_MRNG :: resize_multimap(multimap<float, mapNode> starting_map, int size){
    if(size >= (int)starting_map.size()){
        return;
    }

    multimap<float, mapNode> dummy_map;
    auto it = starting_map.begin();
    for(int i = 0; i < size; i++){
        dummy_map.insert(*it);
        it++;
    }

    starting_map = dummy_map;
}

bool search_graph_MRNG :: check_if_on_map(multimap<float, mapNode> mm, int id){
    for(const auto &it : mm){
        if(it.second.id == id){
            return true;
        }
    }
    return false;
}

search_graph_MRNG :: search_graph_MRNG(int l_mrng, int n_mrng, int dimension, int num_of_nodes, MRNG_Node **ns) : l_MRNG(l_mrng), N_MRNG(n_mrng), 
node_popullation(num_of_nodes), dimen(dimension), nodes(ns){

    initialize_distances();     // Initializing distances first
    cout << endl << "initializing distances done" << endl;
    adjacency_list = vector<list<int>>(num_of_nodes);
    vector<float> centroid(dimension, 0);

    for(int p = 0; p < node_popullation; p++){
        cout<< p << endl;
        for(int d = 0; d < dimen; d++){
            centroid[d] += (float)(*nodes)[p].image[d];
        }

        multimap <float , int> candidates;
        // polu argo, prepei na ginei me ena vector kai aplws ena sorting sto telos isws na einai pio apodotiko
        for(int i = 0; i < node_popullation; i++){
            if(i == p)
                continue;
            
            candidates.insert({get_dist(p, i), i});

        }
        
        adjacency_list[p].push_back(candidates.begin()->second);
        candidates.erase(candidates.begin()->first);

        for(auto r = candidates.begin(); r != candidates.end(); r++){

            bool condition = true;

            for(auto t = adjacency_list[p].begin(); t != adjacency_list[p].end(); t++){

                if(get_dist(r->second, p) > get_dist(*t, r->second) && get_dist(r->second, p) > get_dist(*t, p)){
                    condition = false;
                    break;
                    // etsi opws exei graftei o algori9mos einai poly argo pali (n^2*d*neighbors) 
                    // alla den nomizw oti mporei na apofeux9ei auto dusthxws
                }
            }
            if(condition == true){
                adjacency_list[p].push_back(r->second);
            }

        }
        ofstream adj_list_file("adj_list.txt");
        for(int p = 0; p < num_of_nodes; p++){
            adj_list_file << "Node " << p << " Neighbours: ";
            int count = 0;
            for(auto itr : adjacency_list[p]){
                adj_list_file << itr << ", ";
                count++;
            }
            adj_list_file << endl;
            adj_list_file << "Count of " << p << " :" << count << endl;
        }
    }


    for(int d = 0; d < dimen; d++){
        centroid[d] = centroid[d] / num_of_nodes;    
    }

    //finding nearest node to centroid of entire dataset, that is our search node
    double min_dist = euclidean_distance((*nodes)[0], centroid);
    int position = 0;

    for(int p = 1; p < num_of_nodes; p++){
        float p_dist = euclidean_distance((*nodes)[p], centroid);
        if(p_dist < min_dist){
            position = p;
            min_dist = p_dist;
        }
    }

    starting_node = position;

}

vector<int> search_graph_MRNG :: search_on_graph(MRNG_Node query){
    
    multimap<float , mapNode> S;
    vector<int> knn(N_MRNG);
    mapNode st_node;
    st_node.checked = false;
    st_node.id = starting_node;
    S.insert({euclidean_distance(query, (*nodes)[starting_node]), st_node});
    int i = 0;
    while(i < l_MRNG){
        int next_node = -1;
        for(auto &nd : S){
            if(nd.second.checked == false){
                nd.second.checked = true;
                next_node = nd.second.id;
            }
        }

        if(next_node == -1){
            break;    
        }

        for(auto &neighbor : adjacency_list[next_node]){
            if(check_if_on_map(S, neighbor) == true)
                continue;
            mapNode dummynd;
            dummynd.checked = false;
            dummynd.id = neighbor;
            S.insert({euclidean_distance((*nodes)[neighbor], query), dummynd});
        }

        resize_multimap(S, l_MRNG);
        i++;
        cout << i << "\t";
    }
    cout << endl;

    auto it = S.begin();
    for(int i = 0; i < N_MRNG; i++){
        knn[i] = (*it).second.id;
        it++;
    }

    return knn;
}


int mrng(int argc, char * argv[]){
    int N_mrng = 15;
    int l_mrng = 15; 
    char* input_name, *query_name, *output_name;
    int dimensions, num_of_images;
    ifstream in_str, q_str;
    ofstream out_str;

    input_name = get_str_of_option(argv, argv + argc, "-d");

    if(input_name)
		in_str = ifstream(input_name);
	else{
		cout << "Please provide input file next time, the program will now terminate" << endl;
		exit(-1);
	}

    query_name = get_str_of_option(argv, argv + argc, "-q");

    if(query_name)
		q_str = ifstream(query_name);
	else{
		cout << "Please provide input file next time, the program will now terminate" << endl;
		exit(-1);
	}

    output_name = get_str_of_option(argv, argv + argc, "-o");

	if(output_name)
		out_str = ofstream(output_name);
	else{
		cout << "Please provide output file name next time, the program will now terminate" << endl;
		exit(-2);
	}

    if (!in_str.is_open()) {
		cerr << "Failed to open input file" << endl;
        cout << "File name:" << input_name << endl;
		exit(errno);
	}

    if (!q_str.is_open()) {
		cerr << "Failed to open query file" << endl;
		exit(errno);
	}
    char* N_str, *l_str;

    N_str = get_str_of_option(argv, argv + argc, "-N");
    if(N_str)
        N_mrng = atoi(N_str);

    l_str = get_str_of_option(argv, argv + argc, "-l");
    if(l_str) 
        l_mrng = atoi(l_str);
    in_str.seekg(4);
    char buffer[4];
    in_str.read(buffer,4);
    num_of_images = (static_cast<uint32_t>(static_cast<unsigned char>(buffer[0]))<< 24) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[1])) << 16) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[2])) << 8) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[3])) );
    num_of_images = 1000;
    int rows, columns;
    in_str.read(buffer,4);
    rows = (static_cast<uint32_t>(static_cast<unsigned char>(buffer[0]))<< 24) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[1])) << 16) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[2])) << 8) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[3])) );
    in_str.read(buffer,4);
    columns = (static_cast<uint32_t>(static_cast<unsigned char>(buffer[0]))<< 24) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[1])) << 16) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[2])) << 8) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[3])) );

    dimensions = rows * columns;

    ImageSize = dimensions;
    
    MRNG_Node * array = new MRNG_Node[num_of_images]; //DONT FORGET TO DELETE

	for (int i = 0; i < num_of_images - 1; i++) {
		in_str.read((char*)(array[i].image.data()), dimensions);
	}
    auto constructGraph = chrono::high_resolution_clock::now();
    search_graph_MRNG graph(l_mrng, N_mrng, dimensions, num_of_images, &array);
    auto constructGraphEnd = chrono::high_resolution_clock::now();
    chrono::duration < double > graphTime = constructGraphEnd - constructGraph;
    double seconds= graphTime.count();


    cout << "SEARCH GRAPH DONE in " << seconds <<" seconds" << endl;

    bool repeat = false;

    do{
        double maf=-1;
        repeat = false;
        q_str.seekg(4);
        char q_buffer[4];
        q_str.read(q_buffer,4);
        int num_of_queries, q_dimen;
        num_of_queries = (static_cast<uint32_t>(static_cast<unsigned char>(q_buffer[0]))<< 24) |
        (static_cast<uint32_t>(static_cast<unsigned char>(q_buffer[1])) << 16) |
        (static_cast<uint32_t>(static_cast<unsigned char>(q_buffer[2])) << 8) |
        (static_cast<uint32_t>(static_cast<unsigned char>(q_buffer[3])) );
        int q_rows, q_columns;
        q_str.read(q_buffer,4);
        q_rows = (static_cast<uint32_t>(static_cast<unsigned char>(buffer[0]))<< 24) |
        (static_cast<uint32_t>(static_cast<unsigned char>(buffer[1])) << 16) |
        (static_cast<uint32_t>(static_cast<unsigned char>(buffer[2])) << 8) |
        (static_cast<uint32_t>(static_cast<unsigned char>(buffer[3])) );
        q_str.read(buffer,4);
        q_columns = (static_cast<uint32_t>(static_cast<unsigned char>(buffer[0]))<< 24) |
        (static_cast<uint32_t>(static_cast<unsigned char>(buffer[1])) << 16) |
        (static_cast<uint32_t>(static_cast<unsigned char>(buffer[2])) << 8) |
        (static_cast<uint32_t>(static_cast<unsigned char>(buffer[3])) );

        q_dimen = q_rows * q_columns;
        if(q_dimen != dimensions){
            cout << "Query dimensions not same as input data dimensions, program will now terminate" << endl;
            exit(-3);
        }

        MRNG_Node * queries = new MRNG_Node[num_of_queries];

        for (int i = 0; i < num_of_queries - 1; i++) {
            in_str.read((char*)(queries[i].image.data()), dimensions);
        }

        out_str << "MRNG results" << endl;
        double avg_exhaustive_time = 0, avg_SearchOnGraph_time = 0;
        num_of_queries = 10;
        for(int q = 0; q < num_of_queries; q++){
            auto exhaustiveSearchStart = chrono::high_resolution_clock::now();
            priority_queue<pair_dist_pos, vector<pair_dist_pos>, compare> distances = calculateDistances(array, queries[q], N_mrng, num_of_images);
            auto exhaustiveSearchEnd = chrono::high_resolution_clock::now();
            chrono::duration <double> exhaustiveTime = exhaustiveSearchEnd - exhaustiveSearchStart;
            avg_exhaustive_time += exhaustiveTime.count();
            

            vector<int> NNN;
            auto searchOnGraph_Start = chrono::high_resolution_clock::now();
            NNN = graph.search_on_graph(queries[q]);
            auto searchOnGraph_End = chrono::high_resolution_clock::now();
            chrono::duration <double> SearchOnGraphTime = searchOnGraph_End - searchOnGraph_Start;
            avg_SearchOnGraph_time += SearchOnGraphTime.count();
            out_str << "Query :" << q << endl;
            int i = 1;

            vector<pair_dist_pos> trueknn(N_mrng);

            for(int j = 1; j <= N_mrng; j++){
                trueknn[N_mrng-j] = distances.top();
                distances.pop();
                float dist= euclidean_distance(queries[q],array[NNN[N_mrng-j]]);
                if ((dist/trueknn[N_mrng-j].distance) > maf)
                maf=dist/trueknn[N_mrng-j].distance;

            }

            for(auto itr : NNN){
                out_str << "Nearest Neigbor-" << i << ": " << itr << endl;
                out_str << "distanceApproximate: " << euclidean_distance(array[itr], queries[q]) << endl;
                out_str << "distanceTrue: " << trueknn[i - 1].distance << endl;
                i++;
            }
        }
        avg_SearchOnGraph_time = avg_SearchOnGraph_time / num_of_queries;
        avg_exhaustive_time = avg_exhaustive_time / num_of_queries;
        out_str << "tAverageSearchOnGraph: " << avg_SearchOnGraph_time << endl;
        out_str << "tAverageTrue: " << avg_exhaustive_time << endl;
        out_str << "MAF: " << maf <<endl;
        delete[] queries;
        cout << "Repeat with different query?[y/n]" << endl;

        string answer;
        cin >> answer;
        if (answer.compare("y") == 0) {
            cout << "Enter query file name" << endl;
            cin >> answer;
            repeat = true;
            query_name=new char[answer.length()+1];
            strcpy(query_name,answer.c_str());
            q_str=ifstream(query_name);
    }
    }while (repeat == 1);
    delete[] array;
    return 0;
}


