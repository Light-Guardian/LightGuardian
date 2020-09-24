#ifndef SKETCH
#define SKETCH
#include"param.h"
#include "EMFSD.h"
int get_delay_lev(unsigned delay){
    rep2(i, 0, Lev_Num-1){
        if(delay < delayLevThres[i]) return i;
    }
    return Lev_Num-1;
}

class buck{
public:
    unsigned *Bin;
    buck(){
        Bin = new unsigned[Lev_Num];
        rep2(i, 0, Lev_Num) Bin[i] = 0;
    }
    void insert(unsigned delay){
        int delayLev = get_delay_lev(delay);
        Bin[delayLev] += 1;
    }
    unsigned query(unsigned delay){
        int lev = get_delay_lev(delay);
        return Bin[lev];
    }
    unsigned query_all(){
        int cnt = 0;
        rep2(i, 0, Lev_Num) cnt += Bin[i];
        return cnt;
    }
    ~buck(){
        if(Bin)delete[] Bin;
    }
};

class row{
public:
    buck **bucket;
    BOBHash32 *bobhash;
    bool *mask;
    
    row(unsigned seed){
        bucket = new buck*[Buck_Num_PerRow];
        mask = new bool[Buck_Num_PerRow];
        rep2(i, 0, Buck_Num_PerRow){
            bucket[i] = new buck();
            int randnum = rand()%100;
            if(randnum < 100*collect_rate) mask[i] = 1;
            else mask[i] = 0;
        }
        bobhash = new BOBHash32(seed);

    }
    ~row(){
        rep2(i, 0, Buck_Num_PerRow) if(bucket[i]) delete bucket[i];
        if(bobhash) delete bobhash;
        delete bucket;
    }
    
    int get_hash(const flow_t flow){
        return bobhash->run((char*)&flow, 13) % Buck_Num_PerRow;
    }
    void insert(const flow_t flow, const unsigned delay){
        int pos = get_hash(flow);
        bucket[pos]->insert(delay);
    }
    int query(const flow_t flow, const unsigned delay){
        int pos = get_hash(flow);
        //cout << "pos " << pos << endl;
        if(mask[pos])return (int)bucket[pos]->query(delay);
        else return -1; 
    }
    int query_all(const unsigned pos){
        return (int)bucket[pos]->query_all();
    }
};

class halfCUSketch{
public:
    row **Row;
    halfCUSketch(){
        Row = new row*[Row_Num];
        rep2(i, 0, Row_Num){
            Row[i] = new row((unsigned)i);
        }
    }
    ~halfCUSketch(){
        rep2(i, 0, Row_Num) if(Row[i]) delete Row[i];
        if(Row) delete Row;
    }

    void insert(const flow_t flow, const unsigned delay){
        unsigned Min = 0x5fffffff;
        rep2(i, 0, Row_Num){
            unsigned curr = Row[i]->query(flow, delay);
            //cout << "curr " << curr << endl;
            if(curr <= Min){
                Row[i]->insert(flow, delay);
                Min = curr;
            }
        }
    }
    int query(const flow_t flow, const unsigned delay){
        int Min = 0x1fffffff;
        bool flag = 0;
        rep2(i, 0, Row_Num){
            int curr = Row[i]->query(flow, delay);
            if(curr != -1){
                flag = 1;
                if(curr <= Min) Min = curr;
            }
        }
        if(flag) return Min;
        else return -1;
    }
    unsigned query_lev(const flow_t flow, const int lev){
        unsigned Min = 0x5fffffff;
        bool flag = 0;
        rep2(i, 0, Row_Num){
            int pos = Row[i]->get_hash(flow);
            bool status = Row[i]->mask[pos];
            if(status){
                int curr = Row[i]->bucket[pos]->Bin[lev];
                if(curr <= Min) Min = curr;
                flag = 1;
            }
            
        }
        if(flag) return Min;
        else return -1;
    }
    int get_cardinality(){
        double zero_cnt = 0;
        rep2(i, 0, Buck_Num_PerRow){
            if(Row[0]->query_all((unsigned)i) == 0) zero_cnt += 1;
        }
        return (int)((double)Buck_Num_PerRow * log(Buck_Num_PerRow/(double)zero_cnt));
    }
    void get_distribution(vector<double> &dist, vector<double> &mice_dist){
        
        int Maxrec = 0;
        uint32_t temp[Buck_Num_PerRow];
        rep2(i, 0, Buck_Num_PerRow){
            if(Row[0]->query_all((unsigned)i) > Maxrec) Maxrec = Row[0]->query_all((unsigned)i);
            temp[i] = Row[0]->query_all((unsigned)i);
        }
        mice_dist.resize(Maxrec + 1);
        rep2(i, 0, Buck_Num_PerRow){
            int flowsizetmp = Row[0]->query_all((unsigned)i);
            mice_dist[flowsizetmp]++;
        }
    
        EMFSD *em_fsd_algo = new EMFSD();
        em_fsd_algo->set_counters(Buck_Num_PerRow, temp);
        rep2(i, 0, 10){
            cout << "epoch " << i << endl;
            em_fsd_algo->next_epoch();
        }
        dist = em_fsd_algo->ns;
    }
    void get_entropy(int &tot, double &entr, vector<double> &mice_dist)
    {
        for (int i = 1; i < mice_dist.size(); i++){
            tot += mice_dist[i] * i;
            entr += mice_dist[i] * i * log2(i);
		}
    }
};

class CMSketch{
public:
    row **Row;
    CMSketch(){
        Row = new row*[Row_Num];
        rep2(i, 0, Row_Num){
            Row[i] = new row((unsigned)i);
        }
    }
    ~CMSketch(){
        rep2(i, 0, Row_Num) if(Row[i]) delete Row[i];
        if(Row) delete Row;
    }

    void insert(const flow_t flow, const unsigned delay){
        rep2(i, 0, Row_Num){
            Row[i]->insert(flow, delay);
        }
    }
    unsigned query(const flow_t flow, const unsigned delay){
        unsigned Min = 0x5fffffff;
        rep2(i, 0, Row_Num){
            unsigned curr = Row[i]->query(flow, delay);
            if(curr <= Min) Min = curr;
        }
        return Min;
    }
    unsigned query_lev(const flow_t flow, const int lev){
        unsigned Min = 0x5fffffff;
        rep2(i, 0, Row_Num){
            int pos = Row[i]->get_hash(flow);
            unsigned curr = Row[i]->bucket[pos]->Bin[lev];
            if(curr <= Min) Min = curr;
        }
        return Min;
    }
};

class CUSketch{
public:
    row **Row;
    CUSketch(){
        Row = new row*[Row_Num];
        rep2(i, 0, Row_Num){
            Row[i] = new row((unsigned)i);
        }
    }
    ~CUSketch(){
        rep2(i, 0, Row_Num) if(Row[i]) delete Row[i];
        if(Row) delete Row;
    }

    void insert(const flow_t flow, const unsigned delay){
        unsigned Min = 0x5fffffff;
        rep2(i, 0, Row_Num){
            unsigned curr = Row[i]->query(flow, delay);
            //cout << "curr " << curr << endl;
            if(curr < Min){
                Min = curr;
            }
        }
        rep2(i, 0, Row_Num){
            unsigned curr = Row[i]->query(flow, delay);
            if(curr == Min) Row[i]->insert(flow, delay);
        }
    }
    unsigned query(const flow_t flow, const unsigned delay){
        unsigned Min = 0x5fffffff;
        rep2(i, 0, Row_Num){
            unsigned curr = Row[i]->query(flow, delay);
            if(curr <= Min) Min = curr;
        }
        return Min;
    }
    unsigned query_lev(const flow_t flow, const int lev){
        unsigned Min = 0x5fffffff;
        rep2(i, 0, Row_Num){
            int pos = Row[i]->get_hash(flow);
            unsigned curr = Row[i]->bucket[pos]->Bin[lev];
            if(curr <= Min) Min = curr;
        }
        return Min;
    }
};
#endif