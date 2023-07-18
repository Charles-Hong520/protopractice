#ifndef __PARSER__
#define __PARSER__
#include <fcntl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/util/message_differencer.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include "profile.pb.h"  // Replace with the name of your generated header file.
// https://github.com/bshoshany/thread-pool.git
#include "BS_thread_pool.hpp"
using std::cerr;
using std::cout;
using std::endl;
using std::ios;
#define INPUT_SZ 10
#define gp google::protobuf
#define pp perftools::profiles
class Parser {
 private:
  std::string filename;
  uint8_t* content;
  size_t content_size;
  BS::synced_stream sync_out;
  BS::thread_pool pool;
  std::vector<std::vector<uint32_t>> packedcheck;
  std::map<int, long long> sizemap;
  std::chrono::time_point<std::chrono::system_clock> start_T, end_T;
  long long start_S, end_S;
  std::chrono::duration<double> elapsed_seconds;
  uint32_t THRESHOLD;
  std::vector<pp::Profile*> sub_profiles;

 public:
  Parser();
  Parser(const std::string& fn);
  ~Parser();

  pp::Profile profile;
  pp::Profile profile_correct;

  void print_difference();
  void printLogistics();
  void consumeProfile();

  void parseFromAPI() {
    gp::io::CodedInputStream* input = new gp::io::CodedInputStream(content, content_size);
    profile_correct.ParseFromCodedStream(input);
  }

 private:
  void load_content();

  // all parallel
  void consumeValueType(uint32_t currPos, uint32_t len, pp::ValueType* msg);
  void consumeSample(uint32_t currPos, uint32_t len, pp::Sample* msg);
  void consumeLabel(uint32_t currPos, uint32_t len, pp::Label* msg);
  void consumeMapping(uint32_t currPos, uint32_t len, pp::Mapping* msg);
  void consumeLocation(uint32_t currPos, uint32_t len, pp::Location* msg);
  void consumeLine(uint32_t currPos, uint32_t len, pp::Line* msg);
  void consumeFunction(uint32_t currPos, uint32_t len, pp::Function* msg);

  void seqConsumeSample(gp::io::CodedInputStream*, pp::Sample* msg);
  void seqConsumeLabel(gp::io::CodedInputStream*, pp::Label* msg);

  void consumeChunk(uint32_t start, uint32_t end, uint32_t index);
};
#endif
