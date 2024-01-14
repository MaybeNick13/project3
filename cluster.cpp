#include "cluster.h"

void cluster::insert_to_grp(int grp_num, int point_num){            //bazoume to point sto group number
    for(int d = 0; d < dimen; d++)
        sum_of_pts[grp_num][d] += (float)((unsigned char)(*points)[point_num].image[d]);    
    groups[grp_num].insert(point_num);                                //bazoume to point sto set pou krataei ta id enos group 
    group_num[point_num] = grp_num;                                
}

bool cluster::remove_from_group(int grp_num , int point_num){       //bgazw to stoixeio sthn 9esh point_num apo to cluster group num
    if(groups[grp_num].erase(groups[grp_num].find(point_num)) == groups[grp_num].end())
        return false;                                                //check an uparxei
    for(int d = 0; d < dimen; d++)
        sum_of_pts[grp_num][d] -= (float)((unsigned char)(*points)[point_num].image[d]);    //meiwnw to sum katallhla
    return true;
}

bool cluster::check_conv(vector<vector <float>> old){            //sunarthsh poy mas leei an kanoun converge ta centroids, pairnontas os orisma ta palia centroids
    for(int c = 0; c < num_of_centroids; c++){
        for(int d = 0; d < dimen; d++){
            if(centroids[c][d] - old[c][d] > 1 || centroids[c][d] - old[c][d] < -1)    //exw balei na mhn allazoun perissotero apo mia monada oi times twn diastasewn tou cube
                return false;
        }
    }
    return true;
}

cluster :: cluster(Node** pts, int nc, int np, int dim):points(pts), num_of_centroids(nc), num_of_points(np), dimen(dim){
    // Arxikopoihsh ton metablhtwn
    centroids = vector<vector<float>>(num_of_centroids);        
    group_num = vector<int>(num_of_points, -1);               
    groups.resize(num_of_centroids);
    vector <float>closest_centr_dist (num_of_points);
    vector <float>furthest_centr_dist(num_of_points);
    sum_of_pts = vector<vector<float>>(num_of_points, vector<float>(dimen));

    for(int i = 0; i < num_of_centroids; i++){
        centroids[i] = vector<float>(dimen);    
    }

    int r = rand() % num_of_points;			//random stoixeio ws prwto kentro
    vector<int> excl(num_of_centroids, -1);		//xrhsimopoiw to database gia na mhn xreiazetai na upologisw apostaseis apo shmeia poy einai kentra

    excl[0] = r;
    vector<float> dummy(dimen);
    for(int d = 0; d < dimen; d++){
        dummy[d] = (float)((unsigned char)(*points)[r].image[d]);	//centroid position = random point position, to dummy to exw xrhsimopoihsei gia logous debugging
        sum_of_pts[0][d] = 0.0;
    }

    centroids[0] = dummy;

    for(int p = 0; p < num_of_points; p++){
        closest_centr_dist[p] = euclidean_distance((*points)[p], centroids[0]);
        furthest_centr_dist[p] = closest_centr_dist[p];                //arxikopoihsh twn groups, ta bazw ola sto 1o kentro
        insert_to_grp(0, p);
    }

    for(int c = 1; c < num_of_centroids + 1; c++){
        vector<kmean_node> arr(num_of_points - c);
        float sum = 0;
        int j = 0;                                        
        for(int p = 0; p < num_of_points; p++){
            bool flag = false;
            for(int i = 0; i < c; i++){
                if(excl[i] == p){                //check an to shmeio exei epilex9ei ws kentro
                    flag = true;
                    break;
                }
            }
            if(flag == true)
                continue;

            if(c > 0){	
                float dist = euclidean_distance((*points)[p], centroids[c-1]);
                if(dist < closest_centr_dist[p]){
                    closest_centr_dist[p] = dist;            //bazoume to shmeio sto kontinotero kentro
                    remove_from_group(group_num[p], p);
                    insert_to_grp(c-1, p);
                }
                if(dist > furthest_centr_dist[p])
                    furthest_centr_dist[p];
            }
            sum += furthest_centr_dist[p];                    //a9roisma, to kratame gia na epile3oume tuxaia to epomeno kentro, alla ta pio makrina exoun megaluterh pi9anothta na epilex9oun
            arr[j].key = sum;
            arr[j].pos = p;
            j++;
        }


        if(c < num_of_centroids){	
            int random_pos = rand() % (int)(sum + 1);

            int next_cluster_pos = findSmallestGreater(arr, random_pos);        //dikh mou sunarthsh pou kanei binary search se ena map(to array einai map [key,pos]) kai epistrefei to akrivws megalytero stoixeio tou array

            for(int d = 0; d < dimen; d++){
                dummy[d] = (float)((unsigned char)(*points)[arr[next_cluster_pos].pos].image[d]);	//centroid position = random point position
            }
            centroids[c] = dummy;
            excl[c] = next_cluster_pos;
            
            arr.clear();
        }
    }

}

void cluster::assign_lloyds(){		//Mqueens

    vector<vector<float>> old;
    int i = 0;
    do{
        old = centroids;
        for(int p = 0; p < num_of_points; p++){
            float min = 10000000.0;
            float dist;
            int nc , cc = group_num[p]; 	//new cluster, current cluster
            for(int c = 0; c < num_of_centroids; c++){
                dist = euclidean_distance((*points)[p], centroids[c]);
                if(dist < min){
                    min = dist;
                    nc = c;
                }
            }
            if(nc != cc){
                remove_from_group(cc, p);        //to bgazw apo to prohgoumeno kai to bazw sto kainourio, einai sets opote ta insert kai delete einai grhgora 
                insert_to_grp(nc, p);
                update_2_centers(nc, cc);
            }else
                update_2_centers(cc, -1);			
            
        }
        i++;
    }while(!check_conv(old));        //mexri na kanei converge
}

vector<float> cluster::compute_silhouette(Node* pts){        //upologismos silhouette gia ka9e shmeio 
    vector<float> sil(num_of_points, 2.0);

    for(int p = 0; p < num_of_points; p++){
        float ap = avg_dist_clust_pts(group_num[p], p, pts);
        int closest_centroid;

        if(group_num[p] != 0)
            closest_centroid = 0;
        else
            closest_centroid = 1;

        float min_dist = euclidean_distance(pts[p], centroids[closest_centroid]);

        for(int c = 0; c < num_of_centroids; c++){
            if( c == group_num[p])
                continue;
            float dist;
            dist = euclidean_distance(pts[p], centroids[c]);
            if(dist < min_dist){
                closest_centroid = c;
                min_dist = dist;
            }
        }

        float bp = avg_dist_clust_pts(closest_centroid, p, pts);

        if(bp > ap){
            sil[p] = 1 - ap/bp;
        }
        else if(ap > bp){
            sil[p] = bp/ap - 1;
        }else{
            sil[p] = 0;
        }
    }
    return sil;
}

float cluster::avg_dist_clust_pts(int clust_num, int point_num, Node *pts){    //mesos oros apostashs shmeiou apo ta upoloipa sto cluster tou
    float avg_dist = 0;
    int i = 0;
    for(auto p : groups[clust_num]){
        if(p != point_num){
            avg_dist += euclidean_distance(pts[p], pts[point_num]);
            i++;
        }
    }
    avg_dist = avg_dist / i;
    return avg_dist;
}

void cluster::update_2_centers(int in, int out){                //kanei update ta kentra out kai in, xrhsimopoiontas sto sumofpts gia taxuthta, an to out einai -1 den kanei update to out

    for(int i = 0; i < dimen; i++)
        centroids[in][i] = sum_of_pts[in][i] / groups[in].size();


    // 2nd
    if(out != -1){
        for(int i = 0; i < dimen; i++)
            centroids[out][i] = sum_of_pts[out][i] / groups[out].size();
    }

}

void cluster::update_centers(){                               //kanei update ola ta kentra
    for(int c = 0; c < num_of_centroids; c++){
        for(int i = 0; i < dimen; i++)
            centroids[c][i] = sum_of_pts[c][i] / groups[c].size();
    }
}

vector<float> cluster::get_centroid(int c){
    return centroids[c];                        //epistrefei tis suntetagmenes enos kentroeidous me id c
}

set <int> cluster::get_group(int c){            //epistrefei ola ta points pou anoikoun sto cluster me id c
    return groups[c];
}

int cluster::sizeof_group(int c){                //epistrefei to plh9os twn stoixeiwn enos cluster
    return groups[c].size();
}
