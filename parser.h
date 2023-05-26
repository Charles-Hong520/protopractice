#ifndef __PARSER__
#define __PARSER__
#include "person.pb.h"  // Replace with the name of your generated header file.
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
#include <sys/stat.h>
#include <sys/mman.h>
#include "parallel.h"
//https://github.com/bshoshany/thread-pool.git
#include "BS_thread_pool.hpp"
using std::ios;
using std::cerr;
using std::cout;
using std::endl;
#define INPUT_SZ 10
#define gp google::protobuf
class Parser {
private:
    Person person;
    Person correct_person;
    std::string filename;
    gp::io::CodedInputStream* input;
    uint8_t* content;
    size_t content_size;
    BS::synced_stream sync_out;
    BS::thread_pool pool;

public:
    Parser();
    Parser(const std::string& fn);
    ~Parser();

    double print_difference(const Person&);
private:
    void load_content();    
    bool consume();
    void give_to_worker(uint32_t, uint32_t, Address*);
    
    bool consumeAddress();
};
#endif
