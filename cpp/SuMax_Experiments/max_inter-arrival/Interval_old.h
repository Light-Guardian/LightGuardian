#include"param.h"

int get_delay_lev(unsigned delay){
    rep2(i, 0, 9){
        if(delay < delayLevThres[i]) return i;
    }
    return 9;
}

class buck{
public:
    double delta_t;
    double last_ts;
    buck(double lt = 0.0){
        delta_t = 0.0, last_ts = 0;
    }
    void insert(double ts){
        if(last_ts < 1) last_ts = ts;
        else{
            double tmp = (ts - last_ts);
            delta_t = max(delta_t, tmp);
            last_ts = ts;
        }
    }
    void init(double ts){
        last_ts = ts;
    }
    double query(){
        //cout << delta_t << endl;
        return delta_t;
    }
    ~buck(){ }
};

class row{
public:
    buck **bucket;
    BOBHash32 *bobhash;
    
    row(double ts, unsigned seed){
        cout << "row" << endl;
        cout << Buck_Num_PerRow << endl;
        bucket = new buck*[(unsigned)Buck_Num_PerRow];
        cout << Buck_Num_PerRow << endl;
        rep2(i, 0, Buck_Num_PerRow){
            bucket[i] = new buck(ts);
        }
        bobhash = new BOBHash32((unsigned)seed);
        cout << "row end" << endl;
    }
    ~row(){
        rep2(i, 0, Buck_Num_PerRow) if(bucket[i]) delete bucket[i];
        if(bobhash) delete bobhash;
        if(bucket) delete bucket;
    }
    
    int get_hash(const flow_t flow){
        return bobhash->run((char*)&flow, 13) % Buck_Num_PerRow;
    }
    void insert(const flow_t flow, const double timestamp){
        int pos = get_hash(flow);
        bucket[pos]->insert(timestamp);
    }
    double query(const flow_t flow){
        int pos = get_hash(flow);
        //cout << "pos " << pos << endl;
        return bucket[pos]->query();
    }
};

class interval_Sketch{
public:
    row **Row;
    interval_Sketch(double ts = 0.0){
        cout << "inter" << endl;
        Row = new row*[Row_Num];
        rep2(i, 0, Row_Num){
            Row[i] = new row(ts,(unsigned)(i+rand())%25);
        }
        cout << "inter end" << endl;
    }
    ~interval_Sketch(){
        rep2(i, 0, Row_Num) if(Row[i]) delete Row[i];
        if(Row) delete Row;
    }

    void insert(const flow_t flow, const double timestamp){
        rep2(i, 0, Row_Num){
            Row[i]->insert(flow, timestamp);
        }
    }
    double query(const flow_t flow){
        double Min = 1e12;
        double Max = 0;
        rep2(i, 0, Row_Num){
            double curr = Row[i]->query(flow);
            if(curr >= Max) Max = curr;
            if(curr <= Min) Min = curr;
        }
        //return Max;
        return Min;
    }
};