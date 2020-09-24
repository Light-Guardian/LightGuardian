#include"Sketch.h"
#include"distribution.h"
#include <unistd.h>

using namespace std;

const int N = 1000000;
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
        if(tmp!=0) {delay = get_ChiSquare(1, 3);}
        else {delay = get_Union(0, 80);}
        
        
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
    double wmre = 0;
    double wmre_tmp1 = 0;
    double wmre_tmp2 = 0;
    int aae_num = flow_num;
    int are_num = topkthres;
    
    rep2(i, 0, aae_num){
        rep2(j, 0, Lev_Num){
            aae_sum += (double)ABS((sketch->query_lev(keyvalue[i].first, j) - GT[keyvalue[i].first][j]));
        }
    }
    rep2(i, 0, are_num){
        rep2(j, 0, Lev_Num){
            are_sum += ((double)ABS((sketch->query_lev(keyvalue[i].first, j) - GT[keyvalue[i].first][j]))+1.0) / (GT[keyvalue[i].first][j]+1.0);
            wmre_tmp1 += (double)ABS((sketch->query_lev(keyvalue[i].first, j) - GT[keyvalue[i].first][j]));
            wmre_tmp2 += (double)(sketch->query_lev(keyvalue[i].first, j) + GT[keyvalue[i].first][j])/2.0;
        }
        wmre += wmre_tmp1/wmre_tmp2;
    }
    wmre /= (double) are_num;
    aae_sum = aae_sum / (double)(aae_num*Lev_Num);
    are_sum = are_sum / (double)(are_num*Lev_Num);
    cout << "aae:                       " << aae_sum << " of top " << aae_num << " flows" << endl;
    cout << "are:                       " << are_sum << " of top " << are_num << " flows" << endl;
    stlog << Buck_Num_PerRow*4*8*3/1024 << ',' << wmre << ',';

    //compute CM
    aae_sum = 0;
    are_sum = 0;
    wmre = 0;
    wmre_tmp1 = 0;
    wmre_tmp2 = 0;
    rep2(i, 0, aae_num){
        rep2(j, 0, Lev_Num){
            aae_sum += (double)ABS((CMsketch->query_lev(keyvalue[i].first, j) - GT[keyvalue[i].first][j]));
        }
    }
    rep2(i, 0, are_num){
        rep2(j, 0, Lev_Num){
            are_sum += ((double)ABS((CMsketch->query_lev(keyvalue[i].first, j) - GT[keyvalue[i].first][j]))+1.0) / (GT[keyvalue[i].first][j]+1.0);
            wmre_tmp1 += (double)ABS((CMsketch->query_lev(keyvalue[i].first, j) - GT[keyvalue[i].first][j]));
            wmre_tmp2 += (double)(CMsketch->query_lev(keyvalue[i].first, j) + GT[keyvalue[i].first][j])/2.0;
        }
        wmre += wmre_tmp1/wmre_tmp2;
    }
    wmre /= (double) are_num;

    aae_sum = aae_sum / (double)(aae_num*Lev_Num);
    are_sum = are_sum / (double)(are_num*Lev_Num);
    
    stlog << wmre << ',';

    //compute CU
    aae_sum = 0;
    are_sum = 0;
    wmre = 0;
    wmre_tmp1 = 0;
    wmre_tmp2 = 0;
    rep2(i, 0, aae_num){
        rep2(j, 0, Lev_Num){
            aae_sum += (double)ABS((CUsketch->query_lev(keyvalue[i].first, j) - GT[keyvalue[i].first][j]));
        }
    }
    rep2(i, 0, are_num){
        rep2(j, 0, Lev_Num){
            are_sum += ((double)ABS((CUsketch->query_lev(keyvalue[i].first, j) - GT[keyvalue[i].first][j]))+1.0) / (GT[keyvalue[i].first][j]+1.0);
            wmre_tmp1 += (double)ABS((CUsketch->query_lev(keyvalue[i].first, j) - GT[keyvalue[i].first][j]));
            wmre_tmp2 += (double)(CUsketch->query_lev(keyvalue[i].first, j) + GT[keyvalue[i].first][j])/2.0;
        }
        wmre += wmre_tmp1/wmre_tmp2;
    }
    wmre /= (double) are_num;
    aae_sum = aae_sum / (double)(aae_num*Lev_Num);
    are_sum = are_sum / (double)(are_num*Lev_Num);
    stlog << wmre << endl;
    
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
    
    char outpath[100] = "./union_delay_wmre_vs_memory";
    //strcat(outpath,to_ssssstring(Row_Num).c_str());
    strcat(outpath, ".csv");
    ofstream stlog(outpath, std::ios::out | std::ios::trunc);
    
    stlog << "memory(KB), acu wmre, cm wmre, cu wmre" << endl;
    Buck_Num_PerRow = 1024;
    while(Buck_Num_PerRow <= 65536*2){
        //Buck_Num_PerRow = (int)((double)basic_Buck_Num_PerRow*3/(double)Row_Num);
        calcu(stlog);
        Buck_Num_PerRow *= 2;
    }
    stlog.close();

    return 0;
}