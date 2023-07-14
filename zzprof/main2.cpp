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
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include "parser.h"
#include "profile.pb.h"  // Replace with the name of your generated header file.

/*
g++ -std=c++17 main2.cpp profile.pb.cc -lprotobuf -lpthread -O3 -o profile && ./profile
g++ -std=c++17 main2.cpp profile.pb.cc -lprotobuf -lpthread -O3 -o profile
*/

int main() {
  // Parser parser("zz.prof");
  // Parse input file using protobuf official API
  std::chrono::time_point<std::chrono::system_clock> start, end;
  std::chrono::duration<double> elapsed_seconds;

  std::fstream cinput("zz.prof", std::ios::in | std::ios::binary);
  std::cout << "Parsing zz.prof with protobuf API" << std::endl;
  start = std::chrono::system_clock::now();
  perftools::profiles::Profile correct_profile;
  correct_profile.ParseFromIstream(&cinput);
  end = std::chrono::system_clock::now();
  elapsed_seconds = end - start;
  std::cout << "elapsed time for CORRECT: " << elapsed_seconds.count() << "s\n";
  cinput.close();

  return 0;
}