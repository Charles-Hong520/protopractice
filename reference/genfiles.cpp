#include "output/person.pb.h"  // Replace with the name of your generated header file.
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/util/message_differencer.h>
#include <google/protobuf/message.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "parallel.h"
using std::ios;
using std::cerr;
using std::endl;
#define INPUT_SZ 10

/*
Compile
g++ -std=c++17 main.cpp output/person.pb.cc -lprotobuf -lpthread -o person
*/

void decode(std::string filename) {
    std::cout<<"decoding: "<<filename<<std::endl;
    Person person;
    std::ifstream input(filename, std::ios::binary);
    if (!person.ParseFromIstream(&input)) {
        std::cerr << "Failed to parse binary protobuf data." << std::endl;
        return;
    }

    std::string outfile = filename;
    outfile[0]='d';
    outfile[1]='e';
    std::fstream output(outfile);
    if (!person.SerializeToOstream(&output)) {
      cerr << "Failed to write person output." << endl;
      return;
    }
}

int main(int argc, char** argv) { 
  // Create a Person message.
    Person person;
    person.set_name("Alice");
    person.set_age(30);
    person.add_phone_numbers("555-1234");
    person.add_phone_numbers("555-5678");

    // Serialize the Person message to a string.


    for(int i = 0; i < INPUT_SZ; i++) {
        std::fstream output("encoded"+std::to_string(i)+".bin", ios::out | ios::trunc | ios::binary);
        person.set_name("Alice"+std::to_string(i));
        if (!person.SerializeToOstream(&output)) {
        std::cerr << "Failed to write person." << std::endl;
        return -1;
        }
    }
    parlay::parallel_for(0, INPUT_SZ, [&] (int i) {
        decode("encoded"+std::to_string(i)+".bin");
    });
}
