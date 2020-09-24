
class ts_buck{
public:
    double ts;
    ts_buck(double timestamp){
        ts = timestamp;
    }
    ~ts_buck(){}
    void insert(double timestamp){
        ts = timestamp;
    }
    double query(){
        return ts;
    }
};
class int_buck{
public:
    double ts;
    int_buck(double timestamp){
        ts = timestamp;
    }
    ~int_buck(){}
    void insert(double timestamp){
        ts = max(ts, timestamp);
        
    }
    double query(){
        return ts;
    }
};
class ts_row{
public:
    ts_buck ** bucket;
    BOBHash32 *bobhash;
    ts_row(unsigned seed, double ts = 0.0){
        bucket = new ts_buck*[Buck_Num_PerRow];
        rep2(i, 0, Buck_Num_PerRow){
            bucket[i] = new ts_buck(ts);
        }
        bobhash = new BOBHash32((unsigned)seed);
    }
    ~ts_row(){
        rep2(i, 0, Buck_Num_PerRow) if(bucket[i]) delete bucket[i];
        if(bobhash) delete bobhash;
        delete bucket;
    }
    unsigned get_hash(const flow_t flow){
        return bobhash->run((char*)&flow, 13) % Buck_Num_PerRow;
    }
    void insert(const flow_t flow, const timestamp_t timestamp){
        unsigned pos = get_hash(flow);
        bucket[pos]->insert(timestamp);
    }
    double query(const flow_t flow){
        unsigned pos = get_hash(flow);
        return bucket[pos]->query();
    }
};
class int_row{
public:
    int_buck ** bucket;
    BOBHash32 *bobhash;
    int_row(unsigned seed, double ts = 0.0){
        bucket = new int_buck*[Buck_Num_PerRow];
        rep2(i, 0, Buck_Num_PerRow){
            bucket[i] = new int_buck(ts);
        }
        bobhash = new BOBHash32((unsigned)seed);
    }
    ~int_row(){
        rep2(i, 0, Buck_Num_PerRow) if(bucket[i]) delete bucket[i];
        if(bobhash) delete bobhash;
        delete bucket;
    }
    unsigned get_hash(const flow_t flow){
        return bobhash->run((char*)&flow, 13) % Buck_Num_PerRow;
    }
    void insert(const flow_t flow, const timestamp_t timestamp){
        unsigned pos = get_hash(flow);
        bucket[pos]->insert(timestamp);
    }
    double query(const flow_t flow){
        unsigned pos = get_hash(flow);
        return bucket[pos]->query();
    }
};

class MinMaxSketch{
public:
    ts_row ** tsRow;
    int_row ** intRow;
    MinMaxSketch(double ts = 0){
        tsRow = new ts_row*[Row_Num];
        intRow = new int_row*[Row_Num];
        rep2(i, 0, Row_Num){
            tsRow[i] = new ts_row((unsigned)(i+rand())%25, ts);
            intRow[i] = new int_row((unsigned)((i+5)*2)%25, ts);
        }
    }
    ~MinMaxSketch(){
        rep2(i, 0, Row_Num){
            if(tsRow[i]) delete tsRow[i];
            if(intRow[i]) delete intRow[i];
        }
        if(tsRow) delete tsRow;
        if(intRow) delete intRow;
    }
    void insert(const flow_t &flow, const timestamp_t &ts){
        rep2(i, 0, Row_Num){
            double last_ts = tsRow[i]->query(flow);
            double tmp = ts - last_ts;
            
            if(ABS(last_ts)<1e-5){
                tsRow[i]->insert(flow, ts);
            }
            else{
                double delta_ts = ts - last_ts;
                intRow[i]->insert(flow, delta_ts);
                tsRow[i]->insert(flow, ts);
            }
        }
    }
    double query(const flow_t &flow){
        double Min = 1e10;
        rep2(i, 0, Row_Num){
            double delta_ts = intRow[i]->query(flow);
            if(delta_ts < Min) Min = delta_ts;
        }
        return Min;
    }
    double last_arrival(const flow_t &flow){
        double Min = 1e100;
        rep2(i, 0, Row_Num){
            double last_ts = tsRow[i]->query(flow);
            if(last_ts < Min) Min = last_ts;
        }
        return Min;
    }
};