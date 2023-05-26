#include "parser.h"
#include "gen.h"
/*
Compile
g++ -std=c++17 main.cpp parser.cpp person.pb.cc -lprotobuf -lpthread -o person && ./person
*/
int main() {
    std::chrono::time_point<std::chrono::system_clock> start, end;
    int pcnt = 100;
    int histcnt = 10000000;
    std::string name = std::to_string(pcnt) + "_" + std::to_string(histcnt) + ".bin";
    std::string filename = name;
    if(!std::filesystem::exists(filename)) {
        make_file(pcnt,histcnt);
    }
    Parser parser(filename);

    // Parse input file using protobuf official API
    std::fstream cinput(filename, std::ios::in | std::ios::binary);
    start = std::chrono::system_clock::now();
    Person correct_person;
    correct_person.ParseFromIstream(&cinput);
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout<< "elapsed time for CORRECT: " << elapsed_seconds.count() << "s\n";

    parser.print_difference(correct_person);

}

