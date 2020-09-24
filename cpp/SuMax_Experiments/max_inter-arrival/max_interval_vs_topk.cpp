#include"Interval_old.h"
#include"Interval.h"
#include"packet.h"

using namespace std;

const int N = 1000000;
int topkthres = 1000;
map<flow_t, vector<double>> GT;
map<flow_t, uint32_t> flowmap;

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

double calcu(ofstream &stlog){
    srand(time(0));
    MinMaxSketch* sketch;
    sketch = new MinMaxSketch(0);
    
    ifstream fin(datapath, std::ios::binary);
    timestamp_t k1 = 0;
    flow_t k2_128 = 0;
    BOBHash32 *delay_hash; delay_hash = new BOBHash32(ABS(rand()%10));
    cout << 1 << endl;
    double GTMAX = 0;
    fin.read((char*)&k2_128, 13);
    fin.read((char*)&k1, sizeof(timestamp_t));
    if(flowmap.find(k2_128) == flowmap.end()){
        flowmap.insert(make_pair(k2_128, 1));
    }
    else{
        flowmap[k2_128] += 1;
    }
    
    map<flow_t, timestamp_t> sketch_flow_last_timestamp;
    map<flow_t, timestamp_t> sketch_flow_max_interval;
    
    rep2(i, 1, N){
        if(i %(N/10) == 0) std::cout << "Loading... "<< i/(N/100) << '%' << endl;
        k2_128 = 0, k1 = 0;
        fin.read((char*)&k2_128, 13);
        fin.read((char*)&k1, 8);

        if(flowmap.find(k2_128) == flowmap.end()){
            flowmap.insert(make_pair(k2_128, 1));
        }
        else{
            flowmap[k2_128] += 1;
        }
        sketch->insert(k2_128, k1);

        if(sketch_flow_last_timestamp.find(k2_128)==sketch_flow_last_timestamp.end()){
            sketch_flow_last_timestamp.insert(make_pair(k2_128, k1));
        }
        if(sketch_flow_max_interval.find(k2_128)==sketch_flow_max_interval.end()){
            sketch_flow_max_interval.insert(make_pair(k2_128, 0));
        }
        double gt_interval = ABS(k1 - sketch_flow_last_timestamp[k2_128]);
        sketch_flow_max_interval[k2_128] = max(sketch_flow_max_interval[k2_128], gt_interval);
        sketch_flow_last_timestamp[k2_128] = k1;
    }
    double are = 0;
    double are_old = 0;
    double cnt = 0.0;
    double correct_cnt = 0.0;
    double wrong_cnt = 0.0;
    
    int topk = topkthres;
    int falsecnt = 0;
    vector<pair<flow_t,unsigned>> sorttmp;
    map<flow_t, unsigned>::iterator sortit = flowmap.begin();
    while(sortit != flowmap.end()){
        sorttmp.push_back(*sortit);
        sortit++;
    }
    sort(sorttmp.begin(),sorttmp.end(),Comp);
    
    rep2(j, 0, topk){
        flow_t flowname = sorttmp[j].first;
        timestamp_t max_interval = sketch->query(flowname);
        timestamp_t gt_max_interval = sketch_flow_max_interval[flowname];
        are += (ABS(gt_max_interval-max_interval))/(gt_max_interval);
        if(gt_max_interval>GTMAX) GTMAX = gt_max_interval;
    }
    
    are /= (topk);
    if(sketch) delete sketch;
    if(delay_hash) delete delay_hash;
    return are;
}

void ave(ofstream & stlog){
    double arecnt = 0;
    //how many times to average
    int avetimes = 5;
    rep2(i, 0, (unsigned)avetimes){
        arecnt += calcu(stlog);
    }
    arecnt /= (double) avetimes;
    stlog << topkthres << ',' << arecnt << endl;
    return;
}

int main(){
    ofstream stlog("./row3_basicbucknum262144_maxinterval_topk.csv", std::ios::out | std::ios::trunc);
    double basic_buck_num = 65536*2*2;
    Row_Num = 3;
    stlog << "top-k, ARE" << endl;
    topkthres = 10;
    while(topkthres <= 50000){
        Buck_Num_PerRow = (int)((double)basic_buck_num*3/(double)Row_Num);
        ave(stlog);
        if(topkthres <= 100) topkthres += 10;
        else if(topkthres <= 1000) topkthres += 100;
        else if(topkthres <= 10000) topkthres += 1000;
        else topkthres += 10000;
    }
    return 0;
}
