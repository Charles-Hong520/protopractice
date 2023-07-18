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
g++ -std=c++17 main2.cpp parser.cpp profile.pb.cc -lprotobuf -lpthread -O3 -o profile2 && ./profile2
g++ -std=c++17 main2.cpp parser.cpp profile.pb.cc -lprotobuf -lpthread -O3 -o profile2
*/

int main() {
  std::chrono::time_point<std::chrono::system_clock> start, end;
  std::chrono::duration<double> elapsed_seconds;

  std::string file = "zz.prof";
  Parser parser(file);

  {
    std::cout << "Parsing " << file << " with protobuf API" << std::endl;
    start = std::chrono::system_clock::now();
    // std::fstream cinput(file, std::ios::in | std::ios::binary);
    // perftools::profiles::Profile correct_profile;
    // correct_profile.ParseFromIstream(&cinput);
    parser.parseFromAPI();
    end = std::chrono::system_clock::now();
    elapsed_seconds = end - start;
    std::cout << "elapsed time for CORRECT: " << elapsed_seconds.count() << "s\n";

    std::string s_correct;
    parser.profile_correct.SerializeToString(&s_correct);
    cout << "size correct: " << s_correct.size() << endl;
  }

  return 0;
}