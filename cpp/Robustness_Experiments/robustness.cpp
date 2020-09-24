#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <ctime>

#include <unordered_map>

using namespace std;

vector<pair<string, vector<string>>> up_link;
vector<pair<string, vector<string>>> down_link;

unordered_map<string, vector<string>> up_link_map;
unordered_map<string, vector<string>> down_link_map;

vector<string> server = {"h1", "h2", "h3", "h4", "h5", "h6", "h7", "h8", "h9", "h10", "h11", "h12", "h13", "h14", "h15", "h16"};
vector<string> switch_ = {"s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18", "s19", "s20"};

vector<string> done_server;

unordered_map<string, bool*> switch_repo;
unordered_map<string, int> switch_repo_size;

unordered_map<string, bool> is_server;
unordered_map<string, bool> is_done_server;
unordered_map<string, bool> is_aggregated_switch;

int server_num = 16;
int switch_num = 20;
int random_bound = 10;
int sketch_fragment_num;

void init_up_link(vector<pair<string, vector<string>>> &link){
    link.push_back(make_pair("h1", vector<string> {"s13"}));
    link.push_back(make_pair("h2", vector<string> {"s13"}));
    link.push_back(make_pair("h3", vector<string> {"s14"}));
    link.push_back(make_pair("h4", vector<string> {"s14"}));
    link.push_back(make_pair("h5", vector<string> {"s15"}));
    link.push_back(make_pair("h6", vector<string> {"s15"}));
    link.push_back(make_pair("h7", vector<string> {"s16"}));
    link.push_back(make_pair("h8", vector<string> {"s16"}));
    link.push_back(make_pair("h9", vector<string> {"s17"}));
    link.push_back(make_pair("h10", vector<string> {"s17"}));
    link.push_back(make_pair("h11", vector<string> {"s18"}));
    link.push_back(make_pair("h12", vector<string> {"s18"}));
    link.push_back(make_pair("h13", vector<string> {"s19"}));
    link.push_back(make_pair("h14", vector<string> {"s19"}));
    link.push_back(make_pair("h15", vector<string> {"s20"}));
    link.push_back(make_pair("h16", vector<string> {"s20"}));
    link.push_back(make_pair("s13", vector<string> {"s5", "s6"}));
    link.push_back(make_pair("s14", vector<string> {"s5", "s6"}));
    link.push_back(make_pair("s15", vector<string> {"s7", "s8"}));
    link.push_back(make_pair("s16", vector<string> {"s7", "s8"}));
    link.push_back(make_pair("s17", vector<string> {"s9", "s10"}));
    link.push_back(make_pair("s18", vector<string> {"s9", "s10"}));
    link.push_back(make_pair("s19", vector<string> {"s11", "s12"}));
    link.push_back(make_pair("s20", vector<string> {"s11", "s12"}));
    link.push_back(make_pair("s5", vector<string> {"s1", "s2"}));
    link.push_back(make_pair("s6", vector<string> {"s3", "s4"}));
    link.push_back(make_pair("s7", vector<string> {"s1", "s2"}));
    link.push_back(make_pair("s8", vector<string> {"s3", "s4"}));
    link.push_back(make_pair("s9", vector<string> {"s1", "s2"}));
    link.push_back(make_pair("s10", vector<string> {"s3", "s4"}));
    link.push_back(make_pair("s11", vector<string> {"s1", "s2"}));
    link.push_back(make_pair("s12", vector<string> {"s3", "s4"}));
}

void init_down_link(vector<pair<string, vector<string>>> &link){
    link.push_back(make_pair("s1", vector<string> {"s5", "s7", "s9", "s11"}));
    link.push_back(make_pair("s2", vector<string> {"s5", "s7", "s9", "s11"}));
    link.push_back(make_pair("s3", vector<string> {"s6", "s8", "s10", "s12"}));
    link.push_back(make_pair("s4", vector<string> {"s6", "s8", "s10", "s12"}));
    link.push_back(make_pair("s5", vector<string> {"s13", "s14"}));
    link.push_back(make_pair("s6", vector<string> {"s13", "s14"}));
    link.push_back(make_pair("s7", vector<string> {"s15", "s16"}));
    link.push_back(make_pair("s8", vector<string> {"s15", "s16"}));
    link.push_back(make_pair("s9", vector<string> {"s17", "s18"}));
    link.push_back(make_pair("s10", vector<string> {"s17", "s18"}));
    link.push_back(make_pair("s11", vector<string> {"s19", "s20"}));
    link.push_back(make_pair("s12", vector<string> {"s19", "s20"}));
    link.push_back(make_pair("s13", vector<string> {"h1", "h2"}));
    link.push_back(make_pair("s14", vector<string> {"h3", "h4"}));
    link.push_back(make_pair("s15", vector<string> {"h5", "h6"}));
    link.push_back(make_pair("s16", vector<string> {"h7", "h8"}));
    link.push_back(make_pair("s17", vector<string> {"h9", "h10"}));
    link.push_back(make_pair("s18", vector<string> {"h11", "h12"}));
    link.push_back(make_pair("s19", vector<string> {"h13", "h14"}));
    link.push_back(make_pair("s20", vector<string> {"h15", "h16"}));

}

void done_k_server(int done_num, vector<string> &done_server){
    int done_count = 0;
	while (done_count != done_num){
		int temp = rand() % server_num + 1;
		string str = "h" + to_string(temp);
		if (find(done_server.begin(), done_server.end(), str) == done_server.end()){
			done_server.push_back(str);
			done_count++;
			cout << "done server: " << str << endl;
		}
	}
}

string next_node(unordered_map<string, vector<string>> & link_map, string current_node,
	   	unordered_map<string, bool> & is_pass_node){
    string node;
    vector<string> & next_node_vector = link_map[current_node];

    if(next_node_vector.size() == 1){
        return next_node_vector[0];
    }else{
        int index = rand() % next_node_vector.size();
        while(is_pass_node[next_node_vector[index]]){
            index = rand() % next_node_vector.size();
        }
        return next_node_vector[index];
    }
}

void aggregator(string switch_id, int fragment_id, int &aggregated_sketch_num){
	auto aggregated_fragments = switch_repo[switch_id];

	if (!aggregated_fragments[fragment_id]) {
		aggregated_fragments[fragment_id] = true;
		switch_repo_size[switch_id] += 1;

		if (switch_repo_size[switch_id] == sketch_fragment_num) {
			is_aggregated_switch[switch_id] = true;
			aggregated_sketch_num++;

			cout<<"switch "<<switch_id<<" has been aggregated!"<<endl;
		}
	}
}

void init() {

	for (auto server_name : server) {
		is_server[server_name] = true;
	}

	for (auto done_server_name : done_server) {
		is_done_server[done_server_name] = true;
	}

	for (auto link_pair : up_link) {
		up_link_map[link_pair.first] = link_pair.second;
	}

	for (auto link_pair : down_link) {
		down_link_map[link_pair.first] = link_pair.second;
	}

	for (auto switch_name : switch_) {
		switch_repo_size[switch_name] = 0;
		switch_repo[switch_name] = new bool[sketch_fragment_num];
		for (int i = 0; i < sketch_fragment_num; i++) {
			switch_repo[switch_name][i] = false;
		}
	}
}

int main()
{
	cout<<"Please input the sketch_fragment_num * 3: ";
	cin>>sketch_fragment_num;
	sketch_fragment_num = sketch_fragment_num * 3;

	int done_num;
	cout<<"Please input the done_server_num: ";
	cin>>done_num;

	int seed;
	cout<<"Please input the random seed: ";
	cin>>seed;

	srand(seed);

    int packet_num = 0;
    int aggregated_sketch_num = 0;

    init_up_link(up_link);
    init_down_link(down_link);
    done_k_server(done_num, done_server);

	init();

    while(aggregated_sketch_num != switch_num){
        if(done_server.size() < server_num - 1 ){
            int direction_flag = 0; 
            int fragment_flag = 0; 
            int fragment_id = 0;
            string fragment_switch_id;

            int send_server_index = rand() % server_num + 1;
            string send_server = "h" + to_string(send_server_index);

            while(is_done_server[send_server]){
                send_server_index = rand() % server_num + 1;
                send_server = "h" + to_string(send_server_index);
            }

            int count = 0; 
            string current_node = send_server;

            unordered_map<string, bool> is_pass_node;
			is_pass_node[send_server] = true;

            while(count <= 6){
                if(direction_flag == 0){
                    string NextNode = next_node(up_link_map, current_node, is_pass_node);

                    if(fragment_flag == 0){
                        int temp = rand() % random_bound;
                       
                        if(temp == 0){
                            fragment_switch_id = NextNode;
                            fragment_id = rand() % sketch_fragment_num;
                            fragment_flag = 1;
                        }

                    }

                    count++;
                    
                    if(count >= 3){
                        direction_flag = 1;
                    }else{
                        if(rand() % 10 <= 4){
                            direction_flag = 1;
                        }
                    }

                    current_node = NextNode;

                }else{
                    string NextNode = next_node(down_link_map, current_node, is_pass_node);
                  
                    if(!is_server[NextNode]){
                        if(fragment_flag == 0){
                            int temp = rand() % random_bound;
                          
                            if(temp == 0){
                                fragment_switch_id = NextNode;
                                fragment_id = rand() % sketch_fragment_num;
                                fragment_flag = 1;
                            }
                        }
                    }else{
                        if(!is_done_server[NextNode] && fragment_flag == 1){
                            aggregator(fragment_switch_id, fragment_id, aggregated_sketch_num);
                        }
                        break;
                    }

                    count++;
                    current_node = NextNode;
                }
            }
        }else{
            cout<<"Input error! Done_server_num must be smaller than server_num - 1!"<<endl;
            break;
        }

        packet_num++;
        if (packet_num % 100000 == 0) {
            cout << "packet num: " << packet_num << endl;
        }
    }

    cout<<"It has sent "<<packet_num<<" packets!"<<endl;

    cout << "The run time is: " <<(double)clock() / CLOCKS_PER_SEC << "s" << endl;

    return 0;
}
