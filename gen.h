#ifndef __GEN__
#define __GEN__


#include "person.pb.h"
#include <google/protobuf/message.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <ios>
#include <stdio.h>
#include <filesystem>
void gen_address(Address* addr, int num, int histcnt) {
    std::string num_string = std::to_string(num);
    addr->set_street("STREETNAME_" + num_string);
    addr->set_city("CITY_"+num_string);
    addr->set_state("STATE_"+num_string);
    addr->set_zipcode(num);
    std::string hist = "";
    for(int i = 0; i < histcnt; i++) {
        hist += 'a' + (char)((i+num)%26);
    }
    addr->set_history(hist);
}

Person gen_person(int size, int histcnt) {

    Person per;
    per.set_name("NAME");
    per.set_age(22);
    per.set_phone_numbers("PHONE");
    per.set_hobby("HOBBY");
    for (int i = 0; i < size; i++) {
        Address* address = per.add_addr();
        gen_address(address,i,histcnt);
    }
    return per;
}

void make_file(int pcnt, int histcnt) {

    Person p = gen_person(pcnt, histcnt);
    std::string name = std::to_string(pcnt) + "_" + std::to_string(histcnt) + ".bin";
    std::fstream output(name, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!p.SerializeToOstream(&output)) {
        std::cerr << "Failed to write person." << std::endl;
        return;
    }
    output.close();
}

#endif