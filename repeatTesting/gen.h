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

Person gen_person() {
    Person per;
    per.add_age(-2);
    per.add_age(4);
    per.add_age(-6);
    per.add_age(8);
    per.add_age(10000000);

    return per;
}

Person make_file(const std::string & filename) {

    Person p = gen_person();
    std::string name = filename;
    std::fstream output(name, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!p.SerializeToOstream(&output)) {
        std::cerr << "Failed to write person." << std::endl;
        return {};
    }
    output.close();
    return p;
}

#endif