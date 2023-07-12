#ifndef __PARSER__
#define __PARSER__
#include "profile.pb.h"  // Replace with the name of your generated header file.
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/util/message_differencer.h>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <stdio.h>
#include <fcntl.h>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sys/stat.h>
#include <sys/mman.h>
//https://github.com/bshoshany/thread-pool.git
#include "BS_thread_pool.hpp"
using std::ios;
using std::cerr;
using std::cout;
using std::endl;
#define INPUT_SZ 10
#define gp google::protobuf
#define pp perftools::profiles
class Parser {
private:
    std::string filename;
    gp::io::CodedInputStream* input;
    uint8_t* content;
    size_t content_size;
    BS::synced_stream sync_out;
    BS::thread_pool pool;
    std::map<int,double> timemap;
    std::map<int,long long> sizemap;
    std::chrono::time_point<std::chrono::system_clock>  start_T,end_T;
    long long start_S,end_S;
    std::chrono::duration<double>  elapsed_seconds;
public:
    Parser();
    Parser(const std::string& fn);
    ~Parser();

    pp::Profile profile;
    void print_difference(const pp::Profile&);
    void parseProfile();
private:
    void load_content();  


    
      
    bool consumeProfile();
    bool consumeValueType();
    bool consumeSample();
    bool consumeLabel();
    bool consumeMapping();
    bool consumeLocation();
    bool consumeLine();
    bool consumeFunction();

    void give_to_worker(uint32_t, uint32_t, pp::Profile*);
};
#endif
