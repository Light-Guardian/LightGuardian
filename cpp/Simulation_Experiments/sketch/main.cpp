#include "utils.h"
#include "BOBHash32.h"
#include "cm.h"
#include "cu.h"
#include "hcu.h"

#include <string.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <stdint.h>
using namespace std;

#include <boost/program_options.hpp>
using namespace boost::program_options;

#define ft first
#define sc second

const int num_sw = 20;

vector< pair<int, LL> > flow[21];
map<int, bool> banned;
string file_name;

map<int, int> real_ans;
map<int, vector<int> > paths;
map<int, string> idt2id; 

int load_seed = 155;

void ParseArg(int argc, char *argv[])
{
    options_description opts("Benchmark Options");

    opts.add_options()
/*      ("verbose,v", "print more info")
        ("debug,d", "print debug info")
        ("filename,f", value<string>()->required(), "file dir")*/
        ("help,h", "print help info")
        ;
    variables_map vm;
    
    store(parse_command_line(argc, argv, opts), vm);

    if(vm.count("help"))
    {
        cout << opts << endl;
        return;
    }
}

void LoadData_Mininet(char file[], int no)
{
    BOBHash32 hash_id;
    hash_id.initialize(load_seed);
    // hash_id.initialize(rand()%MAX_PRIME32);
    
    ifstream is(file, ios::in);
    string buf;

    int cnt = 0;
    while (getline(is, buf))
    {
        int del = buf.find(' ');
        string idt = buf.substr(0, del);
        int id = hash_id.run(idt.c_str(), idt.length());
        LL sz = stoll(buf.substr(del+1));
        // printf("%s\n", buf.substr(del+1).c_str());

        // if (sz == 0)
        //     printf("sz = 0!!!!!!!!!!!!!!!! %d %s %s\n", no, buf.substr(del+1).c_str(), buf.c_str());
        // cout << idt << "!!!" << sz << endl;
        if (idt2id.count(id) > 0 && idt != idt2id[id])
        {
            printf("redefined! %s %s.\n", idt2id[id].c_str(), idt.c_str());
        }

        idt2id[id] = idt;
        flow[no].push_back(make_pair(id, sz));
        cnt += sz;
    }

    cout << file << " Loading complete. " << cnt << endl;
}

void Load_Blackhole(char file[])
{
    BOBHash32 hash_id;
    hash_id.initialize(load_seed);
    // hash_id.initialize(rand()%MAX_PRIME32);
    
    ifstream is(file, ios::in);
    string buf;

    int cnt = 0, flowid;
    int num_flow = 10007;
    while (getline(is, buf))
    {
        int step = cnt % 4;
        if (step == 0)
        {
            flowid = hash_id.run(buf.c_str(), buf.length());
        }
        else if (step == 1)
        {
            int key_point = stoi(buf);
            real_ans[flowid] =  key_point;
        }
        else if (step == 2)
        {
            paths[flowid] = {};
            bool flag = true;
            int las, len = buf.length();
            auto cbuf = buf.c_str();
            // printf("%d(%d): ", flowid, real_ans[flowid]);

            for (int i = 0; i < len; ++i)
                if (cbuf[i] == ' ')
                {
                    if (flag)
                    {
                        flag = false;
                        las = i;
                        continue;
                    }

                    int num = 0;
                    for (int j = las+2; j < i; ++j)
                        num = num*10 + (cbuf[j]-'0');
                    // printf("%d ", num);

                    paths[flowid].push_back(num);
                    las = i;
                }
            // printf("\n");
        }

        cnt++;
        if (cnt/4 >= num_flow)
            break;
    }

    cout << "Loading complete. " << cnt << endl;
}

void BlackHoleTest(vector<Sketch*> sk[], vector< pair<int, LL> > flow[], double threshold, bool init_first = false)
{
    static map<int, int> flow_cnt[21];
    
    for (int j = 1; j <= num_sw; ++j)
    {
        int n = flow[j].size(), n_sk = sk[j].size();
        
        flow_cnt[j].clear();
        for (int i = 0; i < n; ++i)
            flow_cnt[j][ flow[j][i].ft ] = 0;
        for (int i = 0; i < n; ++i)
            flow_cnt[j][ flow[j][i].ft ] += flow[j][i].sc;

        for (int id = 0; id < n_sk; ++id)
        {
            if (init_first)
                sk[j][id]->init();

            for (int i = 0; i < n; ++i)
            {
                // if(id == 0)
                //     printf("insert %d\n", flow[i]);
                int fid = flow[j][i].ft, fsz = flow[j][i].sc;
                sk[j][id]->insert(fid, fsz);
                // for (int k = 0; k < fsz; ++k)
                //     sk[j][id]->insert(fid);
            }
        }

        // printf("sw %d total_flow: %d\n", j, flow_cnt[j].size());
    }

    // summary
    printf("Black Hole Test\n");
    printf("-------------------------------------\n");

    fstream fout(file_name, ios::out | ios::app);
    int n_sk = sk[1].size();
    for (int id = 0; id < n_sk; ++id)
    {
        printf("%d) %s\n", id, sk[1][id]->name);
        
        int sz = real_ans.size(), bh_sz = 0, rp_sz = 0;
        double pr = 0, rr = 0;

        for (auto item: real_ans)
        {
            bool flag = false;
            int path_len = paths[item.ft].size();
            if (path_len == 1)
            {
                sz--;
                continue;
            }

            int las_est;
            for (int i = 0; i < path_len; ++i)
            {
                int sw = paths[item.ft][i];
                int est = sk[sw][id]->query_freq(item.ft);
                if (i != 0)
                {
                    if ((double)est/las_est < threshold)
                    {
                        flag = true;
                        break;
                    }
                }
                las_est = est;
            }

            if (item.sc)
                bh_sz++;
            if (flag)
                rp_sz++;
            pr += (flag == item.sc && flag);
            rr += (flag == item.sc && flag);

            // if (id == 0)
            // {
            //     int lo_est = ((VSketch*)sk[id])->query_freq_low(item.ft);
            //     if (lo_est > item.sc || est < item.sc)
            //     {
            //         printf("bad!\n");
            //     }
            //     fout << item.sc << ' ' << est << ' ' << lo_est << endl;
            // }
        }

        pr /= rp_sz;
        rr /= bh_sz;

        sk[1][id]->status();
        printf("Pr: %.3lf  RR: %1.3lf (Over %d/%d)\n", pr, rr, bh_sz, sz);
        fout << pr << ' ' << rr << endl;
    }
}

void LoopTest(vector<Sketch*> sk[], vector< pair<int, LL> > flow[], double threshold, bool init_first = false)
{
    static map<int, int> flow_cnt[21];
    banned.clear();
    
    for (int j = 1; j <= num_sw; ++j)
    {
        int n = flow[j].size(), n_sk = sk[j].size();
        
        flow_cnt[j].clear();
        for (int i = 0; i < n; ++i)
        {
            flow_cnt[j][ flow[j][i].ft ] = 0;
            banned[ flow[j][i].ft ] = true;
        }
        for (int i = 0; i < n; ++i)
        {
            flow_cnt[j][ flow[j][i].ft ] += flow[j][i].sc;
            if (flow_cnt[j][ flow[j][i].ft ] > 100)
                banned[ flow[j][i].ft ] = false;
        }

        for (int id = 0; id < n_sk; ++id)
        {
            if (init_first)
                sk[j][id]->init();

            for (int i = 0; i < n; ++i)
            {
                // if(id == 0)
                //     printf("insert %d\n", flow[i]);
                int fid = flow[j][i].ft, fsz = flow[j][i].sc;
                sk[j][id]->insert(fid, fsz);
                // for (int k = 0; k < fsz; ++k)
                //     sk[j][id]->insert(fid);
            }
        }

        // printf("sw %d total_flow: %d\n", j, flow_cnt[j].size());
    }

    // summary
    printf("Loop Test\n");
    printf("-------------------------------------\n");

    fstream fout(file_name, ios::out | ios::app);
    int n_sk = sk[1].size();
    for (int id = 0; id < n_sk; ++id)
    {
        printf("%d) %s\n", id, sk[1][id]->name);
        
        int sz = real_ans.size(), bh_sz = 0, rp_sz = 0;
        double pr = 0, rr = 0;

        for (auto item: real_ans)
        {
            bool flag = false;
            int path_len = paths[item.ft].size();
            if (path_len == 1)
            {
                sz--;
                continue;
            }

            int las_est;
            for (int i = 0; i < path_len; ++i)
            {
                int sw = paths[item.ft][i];
                int est = sk[sw][id]->query_freq(item.ft);
                // if (item.ft == 1989745473)
                // {
                //     printf("%d:%d ", sw, est);
                // }
                if (i != 0)
                {
                    if ((double)est/las_est > threshold)
                    {
                        flag = true;
                        break;
                    }
                }
                las_est = est;
            }

            if (item.sc)
                bh_sz++;
            if (flag)
                rp_sz++;
            pr += (flag == item.sc && flag);
            rr += (flag == item.sc && flag);
        }

        pr /= rp_sz;
        rr /= bh_sz;

        sk[1][id]->status();
        printf("\nPr: %.3lf  RR: %1.3lf (Over %d/%d)\n", pr, rr, bh_sz, sz);
        fout << pr << ' ' << rr << endl;
    }
}

map< int, vector<LL> > flow_itv[21];

void IntervalTest(vector<Sketch*> sk[], vector< pair<int, LL> > flow[], double threshold, bool init_first = false)
{
    static map<int, int> flow_cnt[21];
    banned.clear();
    
    for (int j = 1; j <= num_sw; ++j)
    {
        // printf("now sw %d: \n", j);
        int n = flow[j].size(), n_sk = sk[j].size();
        
        flow_itv[j].clear();
        flow_cnt[j].clear();
        for (int i = 0; i < n; ++i)
        {
            flow_cnt[j][ flow[j][i].ft ] = 0;
            banned[ flow[j][i].ft ] = true;
        }
        for (int i = 0; i < n; ++i)
        {
            flow_cnt[j][ flow[j][i].ft ] ++;
            if (flow_cnt[j][ flow[j][i].ft ] > 100)
                banned[ flow[j][i].ft ] = false;
        }

        for (int id = 0; id < n_sk; ++id)
        {
            if (init_first)
                sk[j][id]->init();

            LL las = 0;
            for (int i = 0; i < n; ++i)
            {
                int fid = flow[j][i].ft;
                LL stamp = flow[j][i].sc;

                sk[j][id]->insert_interval(fid, stamp);
                if (flow_itv[j].count(fid) == 0)
                {
                    flow_itv[j][fid] = {stamp};
                }
                else
                {
                    flow_itv[j][fid].push_back(stamp);
                }
            }
        }

        // printf("sw %d total_flow: %d\n", j, flow_cnt[j].size());
    }


    // real answer
    {
        int sz = real_ans.size(), bh_sz = 0, rp_sz = 0;
        double pr = 0, rr = 0;

        for (auto item: real_ans)
        {
            int flag = 0;
            int path_len = paths[item.ft].size();
            if (path_len == 1) // || flow_cnt[paths[item.ft][0]][item.ft] < 3)
            {
                sz--;
                continue;
            }

            int las_est;
            for (int i = 0; i < path_len; ++i)
            {
                int sw = paths[item.ft][i];
                int est = 0;

                int pkt_len = flow_itv[sw][item.ft].size();
                for (int j = 1; j < pkt_len; ++j)
                {
                    est = max(est, (int)((flow_itv[sw][item.ft][j] - flow_itv[sw][item.ft][j-1])/10));
                }

                if (i != 0)
                {
                    if (!flag && (double)est/las_est > threshold)
                    {
                        flag = paths[item.ft][i-1];
                        // break;
                    }
                }
                las_est = est;
            }
            real_ans[item.ft] = flag;

            if (item.sc)
                bh_sz++;
            if (flag)
                rp_sz++;
            pr += (flag == item.sc && flag);
            rr += (flag == item.sc && flag);
        }
        pr /= rp_sz;
        rr /= bh_sz;
        printf("(REAL) Pr: %.3lf  RR: %1.3lf (Over %d/%d)\n", pr, rr, bh_sz, sz);
    }



    // summary
    printf("Interval Test\n");
    printf("-------------------------------------\n");

    fstream fout(file_name, ios::out | ios::app);
    int n_sk = sk[1].size();
    for (int id = 0; id < n_sk; ++id)
    {
        printf("%d) %s\n", id, sk[1][id]->name);
        
        int sz = real_ans.size(), bh_sz = 0, rp_sz = 0;
        double pr = 0, rr = 0;

        for (auto item: real_ans)
        {
            int flag = 0;
            int path_len = paths[item.ft].size();
            if (path_len == 1 || flow_cnt[paths[item.ft][0]][item.ft] < 3)
            {
                sz--;
                continue;
            }

            int las_est;
            for (int i = 0; i < path_len; ++i)
            {
                int sw = paths[item.ft][i];
                int est = sk[sw][id]->query_freq(item.ft);
                if (i != 0)
                {
                    if (!flag && (double)est/las_est > threshold)
                    {
                        flag = paths[item.ft][i-1];
                        // break;
                    }
                }
                las_est = est;
            }

            if (item.sc)
                bh_sz++;
            if (flag)
                rp_sz++;
            pr += (flag == item.sc && flag);
            rr += (flag == item.sc && flag);
        }

        pr /= rp_sz;
        rr /= bh_sz;

        sk[1][id]->status();
        printf("Pr: %.3lf  RR: %1.3lf (Over %d/%d)\n", pr, rr, bh_sz, sz);
        fout << pr << ' ' << rr << endl;
    }
}

int main(int argc, char *argv[])
{
    srand(2020);
    // parse args
    ParseArg(argc, argv);

    // load data
    for (int i = 1; i <= num_sw; i++)
    {
        string filename = "../trace/interval/s";
        filename += to_string(i);
        filename += "_interval_10k.dat";
        cout << filename << endl;
        LoadData_Mininet((char*)filename.c_str(), i);
    }
    // LoadData_WebPage("data/webdocs_form00.dat");
    Load_Blackhole("../analysis/meta_10k.txt");

    file_name = string("../log/log_it_10k_1.9");

    // test
    if (true)
    {
        for (int i = 1; i <= 8; ++i)
        {
            // memory - KB
            int mem = i * 512;//(i == 1 ? 50 : i*100);
            if (i == 0)
                mem = 10;

            vector<Sketch*> sk[21];
            for (int j = 1; j <= num_sw; ++j)
            {
                HCUSketch *hcu = new HCUSketch(mem * 3, 3);
                // CMSketch *cms = new CMSketch(mem * 3, 3);
                // CUSketch *cus = new CUSketch(mem * 3, 3);
                sk[j].push_back(hcu);
                // sk[j].push_back(cms);
                // sk[j].push_back(cus);
            }
            
            // BlackHoleTest(sk, flow, 0.1, true);
            // LoopTest(sk, flow, 3.0, true);
            IntervalTest(sk, flow, 1.9, true);
        }
    }

    return 0;
}
