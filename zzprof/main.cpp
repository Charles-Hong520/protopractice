#include "profile.pb.h"  // Replace with the name of your generated header file.
#include "parser.h"
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

/*
g++ -std=c++17 main.cpp parser.cpp profile.pb.cc -lprotobuf -lpthread -o profile && ./profile
g++ -std=c++17 main.cpp parser.cpp profile.pb.cc -lprotobuf -lpthread -o profile
*/

int main() {

  // Parser parser("zz.prof");
   // Parse input file using protobuf official API
  std::chrono::time_point<std::chrono::system_clock>  start,end;
  std::fstream cinput("zz.prof", std::ios::in | std::ios::binary);
  std::cout<<"Parsing zz.prof with protobuf API"<<std::endl;
  start = std::chrono::system_clock::now();
  perftools::profiles::Profile correct_profile;
  correct_profile.ParseFromIstream(&cinput);
  end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  std::cout<< "elapsed time for CORRECT: " << elapsed_seconds.count() << "s\n";
  
  Parser parser("zz.prof");
  std::cout<<"Parsing zz.prof manually"<<endl;
  start = std::chrono::system_clock::now();
  parser.parseProfile();
  end = std::chrono::system_clock::now();
  elapsed_seconds = end - start;
  std::cout<< "elapsed time for MANUAL: " << elapsed_seconds.count() << "s\n";

  // parser.print_difference(correct_profile);


  // write string of message to see what it looks like.
  std::ofstream fout1("zzprof_corr.txt");
  std::ofstream fout2("zzprof_man.txt");
  std::string s_correct, s_manual, write; 
  // google::protobuf::TextFormat::PrintToString(correct_profile, &s_correct);
  // google::protobuf::TextFormat::PrintToString(correct_profile, &s_manual);
  



  correct_profile.SerializeToString(&s_correct);
  parser.profile.SerializeToString(&s_manual);
  cout<<"size correct: "<<s_correct.size()<<endl;
  cout<<"size manual: "<<s_manual.size()<<endl;

  int mark = 0;
  int n = s_correct.size();
  for(int i = 0; i < n; i++) {
    if(s_correct[i]!=s_manual[i]) {
      cout<<"does not match at "<<i<<endl;
      mark = i;
      break;
    }
  }


  s_correct = s_correct.substr(mark, std::min(mark+10000, n));
  s_manual = s_manual.substr(mark, std::min(mark+10000, n));
  fout1 << s_correct;
  fout2 << s_manual;
  fout1.close();
  fout2.close();



  
  return 0;
}