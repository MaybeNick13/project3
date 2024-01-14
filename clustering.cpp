#include "cluster.h"

int main(int argc, char * argv[]) {

	// Open the input file
	char* input_name, *conf_name, *output_name, *inputB_name;
	bool is_complete;
	srand(time(NULL));

	input_name = get_str_of_option(argv, argv + argc, "-i");

	ifstream images, conf_stream, imagesB;
	ofstream out_stream;
	if(input_name)
		images = ifstream(input_name);
	else{
		cout << "Please provide input file next time, the program will now terminate" << endl;
		exit(-1);
	}

    inputB_name = get_str_of_option(argv, argv + argc, "-ib");

    if(inputB_name)
        imagesB = ifstream(inputB_name);
    else{
        cout << "Please provide the input file before the decoding process next time, the program will now terminate" << endl;
        exit(-1);
    }

	conf_name = get_str_of_option(argv, argv + argc, "-c");

    if(conf_name){
        conf_stream = ifstream(conf_name);
    }else{
        cout << "Please provide the config file next time, the program will now terminate" << endl;
        exit(-2);
    }

	output_name = get_str_of_option(argv, argv + argc, "-o");

	if(output_name)
		out_stream = ofstream(output_name);
	else{
		cout << "Please provide output file next time, the program will now terminate" << endl;
		exit(-2);
	}

	is_complete = check_if_option_exists(argv, argv + argc, "-complete");

	if (!images.is_open()) {
		cerr << "Failed to open input.dat" << endl;
		exit(errno);
	}

	if (!conf_stream.is_open()){
		cerr << "Failed to open configuration file" << endl;
		exit(errno);
	}

	// Read 16 bytes of starting data
    images.seekg(4);
    char buffer[4];
    images.read(buffer,4);
    NumImages = (static_cast<uint32_t>(static_cast<unsigned char>(buffer[0]))<< 24) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[1])) << 16) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[2])) << 8) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[3])) );
    int rows, columns;
    images.read(buffer,4);
    rows = (static_cast<uint32_t>(static_cast<unsigned char>(buffer[0]))<< 24) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[1])) << 16) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[2])) << 8) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[3])) );
    images.read(buffer,4);
    columns = (static_cast<uint32_t>(static_cast<unsigned char>(buffer[0]))<< 24) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[1])) << 16) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[2])) << 8) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[3])) );

    imagesB.seekg(8);

    int rowsB, columnsB;
    imagesB.read(buffer,4);
    rowsB = (static_cast<uint32_t>(static_cast<unsigned char>(buffer[0]))<< 24) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[1])) << 16) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[2])) << 8) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[3])) );
    imagesB.read(buffer,4);
    columnsB = (static_cast<uint32_t>(static_cast<unsigned char>(buffer[0]))<< 24) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[1])) << 16) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[2])) << 8) |
    (static_cast<uint32_t>(static_cast<unsigned char>(buffer[3])) );
    
    ImageSize= rows*columns;

	// Read image data into a vector of vectors
	// vector<vector<char>> array(NumImages, vector<char>(ImageSize));


	Node * array = new Node[NumImages];
    ImageSize = rowsB * columnsB;
    Node * arrayB = new Node[NumImages];
    ImageSize = rows * columns;
	for (int i = 0; i < NumImages - 1; i++) {
		images.read(array[i].image.data(), ImageSize);
        imagesB.read(arrayB[i].image.data(), rowsB * columnsB);
	}
	
	// Reading from configuration file

	int cluster_num,
	L_lsh = 3,
	k_lsh = 4,
	M_hcb = 10,
	k_hcb = 3,
	p_hcb = 2;


	string line;
	bool did_provide_clusternum = false;
	while(getline(conf_stream, line)){
		// Read after ": "
		int col_pos = line.find_first_of(':');
		if(!(line.compare(0, col_pos, "number_of_clusters"))){

			cluster_num = get_num_from_line(line, col_pos);
			did_provide_clusternum = true;
		}else if(!(line.compare(0, col_pos, "number_of_vector_hash_tables"))){

			L_lsh = get_num_from_line(line, col_pos);
			L = L_lsh;
		}else if(!(line.compare(0, col_pos, "number_of_vector_hash_functions"))){

			k_lsh = get_num_from_line(line, col_pos);
			k = k_lsh;
		}else if(!(line.compare(0, col_pos, "max_number_M_hypercube"))){

			M_hcb = get_num_from_line(line, col_pos);
			cM = M_hcb;
		}else if(!(line.compare(0, col_pos, "number_of_hypercube_dimensions"))){

			k_hcb = get_num_from_line(line, col_pos);
			ck = k_hcb;
		}else if(!(line.compare(0, col_pos, "number_of_probes"))){

			p_hcb = get_num_from_line(line, col_pos);
		}
	}

	if(did_provide_clusternum == false){
		cout << "Please provide number of clusters in your configuration file, the program will now terminate" << endl;
		exit(-2);
	}

	// creating cluster class

	auto time_start = chrono::high_resolution_clock::now();
	cluster cl(&array, cluster_num, NumImages, ImageSize);


	cl.assign_lloyds();

	auto time_end = chrono::high_resolution_clock::now();
	out_stream << fixed;
	out_stream << "Algorithm: Lloyds" << endl;
	
	for(int c = 0; c < cluster_num; c++){
		out_stream << "Cluster-" << c << "{\n";
		out_stream << "size: " << cl.sizeof_group(c)<< endl;
		out_stream << "Cluster coords:" << endl;
		vector<float> coordinates = cl.get_centroid(c);
		for(int i = 0; i < rows * columns; i++){
			out_stream << setw(6) << setprecision(1)<< coordinates[i] << "\t";
		}
        out_stream << endl;
		if(is_complete == true){
			out_stream << "Points in cluster: " ;
			for(auto p : cl.get_group(c)){
				out_stream << "," << setw(7) <<p;
			}
		}

		out_stream <<"}" << endl; 
	}
	chrono::duration < double > run_time = time_end - time_start;
	out_stream << "clustering_time: " << run_time.count() << endl;

    ImageSize = rowsB * columnsB;
	vector<float> sil = cl.compute_silhouette(arrayB);
    ImageSize = rows * columns;
	vector<float> avg_sil(cluster_num + 1);

	float total_sum = 0;
	int i = 0;
	for(int c = 0; c < cluster_num; c++){
		int j = 0;
		float cl_sum = 0;
		for(auto p : cl.get_group(c)){
			cl_sum += sil[p];
			j++;
		}
		avg_sil[c] = cl_sum / j;
		total_sum += cl_sum;
		i += j; 

	}
	total_sum=total_sum/i;
	out_stream << "Silhouette: [";
	for(int c = 0; c < cluster_num + 1; c++){
		if(c<cluster_num){
		    out_stream << setw(4) << setprecision(3) << avg_sil[c] << " ";
        }else{
		    out_stream << setw(4) << setprecision(3) << total_sum << " ";
		}
	}
	out_stream << "]" << endl;
}