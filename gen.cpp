/*
g++ -std=c++17 gen.cpp parser.cpp person.pb.cc -lprotobuf -lpthread -o tp && ./tp
*/
#include "parser.h"
#include "gen.h"

//seq vs parallel faster?
int main() {
    std::chrono::time_point<std::chrono::system_clock> start, end;
    std::map<std::pair<int,int>, std::pair<double,double>> timemap;
    std::vector<int> plist = {1, 10, 20, 50, 100, 200, 500, 1000};
    std::vector<int> hlist;
    for(int i = 1; i <= 1e8; i*=10) {
        hlist.push_back(i);
    }
    
    for(int p : plist) { 
        for(int hist : hlist) {
            long long lim = (long long) p * (long long) hist;
            if(lim < 2e9) {
                // cout<<lim<<endl;
                std::string filename = "bin/"+std::to_string(p) + "_" + std::to_string(hist) + ".bin";
                if(!std::filesystem::exists(filename)) {
                    cout<<p<<" "<<hist<<"making"<<endl;
                    make_file(filename,p,hist);
                    cout<<p<<" "<<hist<<"finish"<<endl;
                }
                double google = 0, custom = 0;
                int itr = 2;
                for(int i = 0; i < itr; i++) {
                    Parser parser(filename);
                    std::fstream cinput(filename, std::ios::in | std::ios::binary);
                    start = std::chrono::system_clock::now();
                    Person correct_person;
                    correct_person.ParseFromIstream(&cinput);
                    end = std::chrono::system_clock::now();
                    std::chrono::duration<double> elapsed_seconds = end - start;
                    google += elapsed_seconds.count();
                    custom += parser.print_difference(correct_person);
                }
                timemap[{p,hist}]={google/itr,custom/itr};
            }
        }
    }

    std::ofstream timesheet("timemap.txt");
    for(auto & [type,res] : timemap) {
        timesheet<<type.first<<" "<<type.second<<" "<<res.first<<" "<<res.second<<endl;
    }
    timesheet.close();
    std::cout<<"done generating timemap"<<std::endl;
    //p are rows, h are cols
}