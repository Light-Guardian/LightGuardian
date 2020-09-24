#include"Sketch.h"
#include"distribution.h"
#include <unistd.h>

using namespace std;

//const int N = 1000000;
const int N = 100000;
int topkthres = 100;

bool Cmp_By_Value(const pair<flow_t, uint32_t> & lhs, const pair<flow_t, uint32_t> & rhs){
    return lhs.second > rhs.second;
}

int print_uint128(__uint128_t n) {
  if (n == 0)  return printf("0");

  char str[40] = {0}; // log10(1 << 128) + '\0'
  char *s = str + sizeof(str) - 1; // start at the end
  while (n != 0) {
    if (s == str) return -1; // never happens

    *--s = "0123456789"[n % 10]; // save last digit
    n /= 10;                     // drop it
  }
  return printf("%s", s);
}

void calcu(ofstream & stlog){
    
    gtmap GT;
    map<flow_t, uint32_t> flowmap;
    srand(time(0));
    halfCUSketch* sketch;
    CMSketch* CMsketch;
    CUSketch* CUsketch;
    sketch = new halfCUSketch();
    CMsketch = new CMSketch();
    CUsketch = new CUSketch();

    ifstream fin(datapath, std::ios::binary);
    timestamp_t k1;
    flow_t k2_128;
    BOBHash32 *delay_hash; delay_hash = new BOBHash32(3);
    

    rep2(i, 0, N){    
        fin.read((char*)&k2_128, 13);
        fin.read((char*)&k1, sizeof(timestamp_t));
        unsigned delay;
        int tmp = delay_hash->run((char*)&k2_128, 13)%2;
        /*
        if(tmp!=0) {delay = get_ChiSquare(1, 3);}
        else {delay = get_Union(0, 80);}
        */
        //delay = get_Union(0, 80);
        delay = get_ChiSquare(1, 3);

        int delay_lev = get_delay_lev(delay);
        sketch->insert(k2_128, delay);
        CMsketch->insert(k2_128, delay);
        CUsketch->insert(k2_128, delay);

        if(flowmap.find(k2_128) == flowmap.end()){
            flowmap.insert(make_pair(k2_128, 1));
        }
        else{
            flowmap[k2_128] += 1;
        }

        if(GT.find(k2_128) == GT.end()){
            GT.insert(make_pair(k2_128, vector<unsigned>(begin(Zeros), end(Zeros))));
            GT[k2_128][delay_lev] += 1;
        }
        else{
            GT[k2_128][delay_lev] += 1;
        }
        
    }
    
    fin.close();
    vector<pair<flow_t, uint32_t> > keyvalue(flowmap.begin(), flowmap.end());
    sort(keyvalue.begin(), keyvalue.end(), Cmp_By_Value);
    cout << "flow number is " << keyvalue.size() << endl;
    unsigned flow_num = keyvalue.size();
    
    cout << "********* statistical analysis *********" << endl;  
    double aae_sum = 0;
    double are_sum = 0;
    int aae_num = flow_num;
    int are_num = topkthres;
    
    are_sum = 0; aae_sum = 0;
    
    /*
    Cardinality ARE
    */
    double card_re = ABS(flow_num-sketch->get_cardinality())/(double)flow_num;
    
	vector<double> real_dist;
	int maxflowsize = keyvalue[0].second;
	real_dist.resize(maxflowsize+1);
	rep2(i, 0, flow_num){
		real_dist[keyvalue[i].second]++;
	}

    /*
    distribution
    */
    vector<double> dist;
    vector<double> mice_dist;
	double dist_wmre;
    sketch->get_distribution(dist, mice_dist);
    double tmp1 = 0; double tmp2 = 0;
	int maximum_flowsize = max(dist.size(), real_dist.size());
	rep2(i, 1, maximum_flowsize){
		double n = 0; double n_hat = 0;
		if(i < dist.size()) n_hat = (double)dist[i];
		if(i < real_dist.size()) n = (double)real_dist[i];
		tmp1 += ABS(n_hat - n);
		tmp2 += (n_hat + n)/2;
	}
	dist_wmre = tmp1 / tmp2;
    /*
    entropy
    */
    double entropy = 0; double real_entropy = 0;
    int tot; double entr;
	sketch->get_entropy(tot, entr, dist);
	int real_tot; double real_entr;
	sketch->get_entropy(real_tot, real_entr, real_dist);
	entropy = -entr/tot + log2(tot);
    real_entropy = -real_entr/real_tot + log2(real_tot);
    
    double entr_re = ABS(real_entropy-entropy)/real_entropy;
    stlog << (Buck_Num_PerRow*3*4/1024) << ',' << card_re << ',' << dist_wmre << ',' << entr_re << endl;

    if(sketch) delete sketch;
    if(CMsketch) delete CMsketch;
    if(CUsketch) delete CUsketch;
    if(delay_hash) delete delay_hash;

}

void testGetOpt(int argc, char *argv[], int &a, int &b, int &c) {
    int opt; 
    const char *optstring = "a:b:c:"; 

    while ((opt = getopt(argc, argv, optstring)) != -1) {
        printf("opt = %c\n", opt); 
        printf("optarg = %s\n", optarg);
        printf("optind = %d\n", optind);
        printf("argv[optind - 1] = %s\n\n",  argv[optind - 1]);
        if(opt=='a') a = atoi(optarg);
        if(opt=='b') b = atoi(optarg);
        if(opt=='c') c = atoi(optarg);
    }
}

int main(int argc, char *argv[]){
    
    topkthres = 1000;
    ofstream stlog("./card_dist_ent_vs_memory.csv", std::ios::out | std::ios::trunc);
    Row_Num = 3;
    //Buck_Num_PerRow = (int)((double)65536*3/(double)Row_Num);
    stlog << "memory(KB), cardinality RE, distribution WMRE, entropy RE" << endl;
    Buck_Num_PerRow = 1024;
    while(Buck_Num_PerRow <= 65536){
        calcu(stlog);
        Buck_Num_PerRow *= 2;
    }

    stlog.close();
    return 0;
}