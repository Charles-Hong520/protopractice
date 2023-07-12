#include "parser.h"

Parser::Parser(const std::string & fn) {
    filename = fn;
    constexpr size_t MAX_CONTENT_SIZE = INT_MAX;  // Set an appropriate maximum content size
    content = new uint8_t[MAX_CONTENT_SIZE];
    content_size = 0;
    load_content();
    input = new gp::io::CodedInputStream(content, content_size);
}
void Parser::load_content() {
    std::ifstream file(filename, std::ios::binary);
    struct stat st;
    stat(filename.c_str(), &st);
    content_size = st.st_size;

    int fhand = open(filename.c_str(), O_RDONLY);
    content = (uint8_t*)mmap(0, content_size, PROT_READ, MAP_FILE|MAP_PRIVATE, fhand, 0);
}

Parser::~Parser() {
    if(input) delete input;
    // if(content) delete[] content;
    munmap(content, content_size);
    content = nullptr;
}



void Parser::give_to_worker(uint32_t currPos, uint32_t len, pp::Profile* msg) {
    // std::chrono::time_point<std::chrono::system_clock>  start = std::chrono::system_clock::now();
    gp::io::CodedInputStream *copyStream = new gp::io::CodedInputStream(
        content+currPos, len
    );
    msg->ParseFromCodedStream(copyStream);
    // std::chrono::time_point<std::chrono::system_clock>  end = std::chrono::system_clock::now();
    // std::chrono::duration<double>  elapsed_seconds = end - start;
    // sync_out.println("elapsed time for CUSTOM: ",currPos," ",elapsed_seconds.count(),"s");
}

void Parser::print_difference(const pp::Profile& correct_profile) {
    cout<<"Comparing Difference"<<endl;
    if(!gp::util::MessageDifferencer::ApproximatelyEquivalent(profile,correct_profile)) {
        std::cout<<"NOT APPROX EQUIV\n";
    } else {
        std::cout<<"APPROX EQUIV\n";
    }
    if(!gp::util::MessageDifferencer::Equivalent(profile, correct_profile)) {
        std::cout<<"NOT EQUIV\n";
        // if (std::string s; gp::TextFormat::PrintToString(profile, &s)) {
        //     // std::cout << "Your message: " << s;
        // } else {
        //     std::cerr << "Message not valid (partial content: "
        //     << profile.ShortDebugString() << ")\n";
        // }
        // cout<<endl;
        // if (std::string s; gp::TextFormat::PrintToString(correct_profile, &s)) {
        //     // std::cout << "correct message: " << s;
        // } else {
        //     std::cerr << "Message not valid (partial content: "
        //     << correct_profile.ShortDebugString() << ")\n";
        // }
    } else {
        std::cout<<"EQUAL\n";
    }
}

void Parser::parseProfile() {
    // profile.ParseFromCodedStream(input);
    while(consumeProfile()) {
    }

    cout<<endl;

    cout<<"time size profile"<<endl;
    for(auto &[num,val] : timemap) {
        cout<<num<<": "<<val<<"\t"<<sizemap[num]<<endl;
    }
    cout<<endl;

    cout<<"size/time ratio profile"<<endl;
    for(auto &[num,val] : sizemap) {
        cout<<std::fixed<<std::setprecision(4);
        cout<<num<<": "<<val/timemap[num]<<endl;
    }
    cout<<endl;
}

bool Parser::consumeProfile() {
    start_S = input->CurrentPosition();
    uint32_t tag = input->ReadTag();
    if (tag == 0) {
        return false;
    }
    uint32_t field_id = tag >> 3;
    uint32_t wire = tag & 7;
    // handle a record
    switch (field_id) {
        case 1:
        //   repeated ValueType sample_type = 1;
            if(wire==2) {
                uint32_t len;
                input->ReadVarint32(&len);
                pp::ValueType* msg = profile.add_sample_type();

                //thread pool
                uint32_t currPos = input->CurrentPosition();
                input->Skip(len);
                // pool.push_task(&Parser::give_to_worker, this, currPos, len, msg);

                //seq
    gp::io::CodedInputStream *copyStream = new gp::io::CodedInputStream(
        content+currPos, len
    );
    start_T = std::chrono::system_clock::now();
    msg->ParseFromCodedStream(copyStream);
    end_T = std::chrono::system_clock::now();
    delete copyStream;

                //todo convert to while(consumeValueType());
            }
        break;
        case 2:
        //   repeated Sample sample = 2;
            if(wire==2) {
                uint32_t len;
                input->ReadVarint32(&len);
                pp::Sample* msg = profile.add_sample();
                
                //thread pool
                uint32_t currPos = input->CurrentPosition();
                input->Skip(len);
                // pool.push_task(&Parser::give_to_worker, this, currPos, len, msg);

                //seq
    gp::io::CodedInputStream *copyStream = new gp::io::CodedInputStream(
        content+currPos, len
    );
    start_T = std::chrono::system_clock::now();
    msg->ParseFromCodedStream(copyStream);
    end_T = std::chrono::system_clock::now();
    delete copyStream;

                //todo convert to while(consume...());
            }
        break;
        case 3:
        //  repeated Mapping mapping = 3;
            if(wire==2) {
                uint32_t len;
                input->ReadVarint32(&len);
                pp::Mapping* msg = profile.add_mapping();
                
                //thread pool
                uint32_t currPos = input->CurrentPosition();
                input->Skip(len);
                // pool.push_task(&Parser::give_to_worker, this, currPos, len, msg);

                //seq
    gp::io::CodedInputStream *copyStream = new gp::io::CodedInputStream(
        content+currPos, len
    );
    start_T = std::chrono::system_clock::now();
    msg->ParseFromCodedStream(copyStream);
    end_T = std::chrono::system_clock::now();
    delete copyStream;

                //todo convert to while(consume...());
            }
        break;
        case 4:
        //  repeated Location location = 4;
            if(wire==2) {
                uint32_t len;
                input->ReadVarint32(&len);
                pp::Location* msg = profile.add_location();
                
                //thread pool
                uint32_t currPos = input->CurrentPosition();
                input->Skip(len);
                // pool.push_task(&Parser::give_to_worker, this, currPos, len, msg);

                //seq
    gp::io::CodedInputStream *copyStream = new gp::io::CodedInputStream(
        content+currPos, len
    );
    start_T = std::chrono::system_clock::now();
    msg->ParseFromCodedStream(copyStream);
    end_T = std::chrono::system_clock::now();
    delete copyStream;

                //todo convert to while(consume...());
            }
        break;
        case 5:
        //  repeated Function function = 5;
            if(wire==2) {
                uint32_t len;
                input->ReadVarint32(&len);
                pp::Function* msg = profile.add_function();
                
                //thread pool
                uint32_t currPos = input->CurrentPosition();
                input->Skip(len);
                // pool.push_task(&Parser::give_to_worker, this, currPos, len, msg);

                //seq
    gp::io::CodedInputStream *copyStream = new gp::io::CodedInputStream(
        content+currPos, len
    );
    start_T = std::chrono::system_clock::now();
    msg->ParseFromCodedStream(copyStream);
    end_T = std::chrono::system_clock::now();
    delete copyStream;

                //todo convert to while(consume...());
            }
        break;
        case 6:
        //   repeated string string_table = 6;
        //  repeated strings are fine.
            if(wire==2) {
                uint32_t len;
                input->ReadVarint32(&len);
                std::string buffer;
                input->ReadString(&buffer, len);
                profile.add_string_table(buffer);
            }
        break;
        case 7:
        //   int64 drop_frames = 7;
            if(wire==0) {
                uint64_t i;
                input->ReadVarint64(&i);
                profile.set_drop_frames(i);
            }
        break;
        case 8:
        //  int64 keep_frames = 8;
            if(wire==0) {
                uint64_t i;
                input->ReadVarint64(&i);
                profile.set_keep_frames(i);
            }
        break;
        case 9:
        //  int64 time_nanos = 9;
            if(wire==0) {
                uint64_t i;
                input->ReadVarint64(&i);
                profile.set_time_nanos(i);
            }
        break;
        case 10:
        //  int64 duration_nanos = 10;
            if(wire==0) {
                uint64_t i;
                input->ReadVarint64(&i);
                profile.set_duration_nanos(i);
            }
        break;
        case 11:
        //  ValueType period_type = 11;
            if(wire==2) {
                uint32_t len;
                input->ReadVarint32(&len);
                pp::ValueType* msg = profile.mutable_period_type();
                
                //thread pool
                uint32_t currPos = input->CurrentPosition();
                input->Skip(len);
                // pool.push_task(&Parser::give_to_worker, this, currPos, len, msg);

                //seq
    gp::io::CodedInputStream *copyStream = new gp::io::CodedInputStream(
        content+currPos, len
    );
    start_T = std::chrono::system_clock::now();
    msg->ParseFromCodedStream(copyStream);
    end_T = std::chrono::system_clock::now();
    delete copyStream;
                //todo convert to while(consume...());
            }
        break;
        case 12:
        //  int64 period = 12;
            if(wire==0) {
                uint64_t i;
                input->ReadVarint64(&i);
                profile.set_period(i);
            }
        break;
        case 13: //ALL REPEATS ARE PACKED -> LEN
        //   repeated int64 comment = 13;
            if(wire==2) {
                uint32_t len;
                input->ReadVarint32(&len);

                //can be parallelized
                uint64_t end = len + input->CurrentPosition();
                uint64_t i;
                while(input->CurrentPosition() < end) {
                    input->ReadVarint64(&i);
                    profile.add_comment(i);
                }
            }
        break;
        case 14:
        //  int64 default_sample_type = 14;
            if(wire==0) {
                uint64_t i;
                input->ReadVarint64(&i);
                profile.set_default_sample_type(i);
            }
        break;
    }
    elapsed_seconds = end_T-start_T;
    end_S = input->CurrentPosition();

    timemap[field_id]+=elapsed_seconds.count();
    sizemap[field_id]+=end_S-start_S;
    return true;
}


// bool Parser::consumeValueType()
// {
//     uint32_t tag = input->ReadTag();
//     if (tag == 0)
//     {
//         return false;
//     }
//     uint32_t field_id = tag >> 3;
//     uint32_t wire = tag & 7;
//     // handle a record
//     switch (field_id)
//     {
//     case 1: // type
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             value_type.set_type(i);
//         }
//         break;
//     case 2: // unit
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             value_type.set_unit(i);
//         }
//         break;
//     }
//     return true;
// }

// bool Parser::consumeSample()
// {
//     uint32_t tag = input->ReadTag();
//     if (tag == 0)
//     {
//         return false;
//     }
//     uint32_t field_id = tag >> 3;
//     uint32_t wire = tag & 7;
//     // handle a record
//     switch (field_id)
//     {
//     case 1: // location id
//         if (wire == 2)
//         {
//             uint64_t len;
//             input->ReadVarint64(&len);
//             // can be parallelized
//             uint64_t end = len + input->CurrentPosition();
//             uint64_t i;
//             while (input->CurrentPosition() < end)
//             {
//                 input->ReadVarint64(&i);
//                 profile.add_location_id(i);
//             }
//         }
//         break;
//     case 2: // value
//         if (wire == 2)
//         {
//             uint64_t len;
//             input->ReadVarint64(&len);
//             // can be parallelized
//             uint64_t end = len + input->CurrentPosition();
//             uint64_t i;
//             while (input->CurrentPosition() < end)
//             {
//                 input->ReadVarint64(&i);
//                 profile.add_value(i);
//             }
//         }
//         break;
//     case 3: // label
//         if (wire == 2)
//         {
//             uint32_t len;
//             input->ReadVarint32(&len);

//             uint32_t currPos = input->CurrentPosition();
//             Label *label = profile.add_label();
//             input->Skip(len);

//             // thread pool
//             //  pool.push_task(&Parser::give_to_worker, this, currPos, len, label);

//             // seq
//             gp::io::CodedInputStream *copyStream = new gp::io::CodedInputStream(
//                 content + currPos, len);
//             label->ParseFromCodedStream(copyStream);
//         }
//         break;
//     }
//     return true;
// }

// bool Parser::consumeLabel()
// {
//     uint32_t tag = input->ReadTag();
//     if (tag == 0)
//     {
//         return false;
//     }
//     uint32_t field_id = tag >> 3;
//     uint32_t wire = tag & 7;
//     // handle a record
//     switch (field_id)
//     {
//     case 1: // key
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             label.set_key(i);
//         }
//         break;
//     case 2: // str
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             label.set_str(i);
//         }
//         break;
//     case 3: // num
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             label.set_num(i);
//         }
//         break;
//     case 4: // num unit
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             label.set_num_unit(i);
//         }
//         break;
//     }
//     return true;
// }

// bool Parser::consumeMapping()
// {
//     uint32_t tag = input->ReadTag();
//     if (tag == 0)
//     {
//         return false;
//     }
//     uint32_t field_id = tag >> 3;
//     uint32_t wire = tag & 7;
//     // handle a record
//     switch (field_id)
//     {
//     case 1: // id
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             profile.set_id(i);
//         }
//         break;
//     case 2: // memory start
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             profile.set_memory_start(i);
//         }
//         break;
//     case 3: // memory limit
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             profile.set_memory_limit(i);
//         }
//         break;
//     case 4: // file offset
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             profile.set_file_offset(i);
//         }
//         break;
//     case 5: // filename
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             profile.set_filename(i);
//         }
//         break;
//     case 6: // build id
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             profile.set_build_id(i);
//         }
//         break;
//     case 7: // has functions
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             profile.set_has_functions(i);
//         }
//         break;
//     case 8: // has filenames
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             profile.set_has_filenames(i);
//         }
//         break;
//     case 9: // has line numbers
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             profile.set_has_line_numbers(i);
//         }
//         break;
//     case 10: // has inline frames
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             profile.set_has_inline_frames(i);
//         }
//         break;
//     }
//     return true;
// }

// bool Parser::consumeLocation()
// {
//     uint32_t tag = input->ReadTag();
//     if (tag == 0)
//     {
//         return false;
//     }
//     uint32_t field_id = tag >> 3;
//     uint32_t wire = tag & 7;
//     // handle a record
//     switch (field_id)
//     {
//     case 1: // id
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             profile.set_id(i);
//         }
//         break;
//     case 2: // mappping id
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             profile.set_mapping_id(i);
//         }
//         break;
//     case 3: // address
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             profile.set_address(i);
//         }
//         break;
//     case 4: // line
//         if (wire == 2)
//         {
//             uint32_t len;
//             input->ReadVarint32(&len);

//             uint32_t currPos = input->CurrentPosition();
//             pp::Line *line = profile.add_line();
//             input->Skip(len);

//             // thread pool
//             //  pool.push_task(&Parser::give_to_worker, this, currPos, len, line);

//             // seq
//             gp::io::CodedInputStream *copyStream = new gp::io::CodedInputStream(
//                 content + currPos, len);
//             line->ParseFromCodedStream(copyStream);
//         }
//         break;
//     case 5: // is folded
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             profile.set_is_folded(i);
//         }
//         break;
//     }
//     return true;
// }

// bool Parser::consumeLine()
// {
//     uint32_t tag = input->ReadTag();
//     if (tag == 0)
//     {
//         return false;
//     }
//     uint32_t field_id = tag >> 3;
//     uint32_t wire = tag & 7;
//     // handle a record
//     switch (field_id)
//     {
//     case 1: // function id
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             line.set_function_id(i);
//         }
//         break;
//     case 2: // line
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             line.set_line(i);
//         }
//         break;
//     }
//     return true;
// }

// bool Parser::consumeFunction()
// {
//     uint32_t tag = input->ReadTag();
//     if (tag == 0)
//     {
//         return false;
//     }
//     uint32_t field_id = tag >> 3;
//     uint32_t wire = tag & 7;
//     // handle a record
//     switch (field_id)
//     {
//     case 1: // id
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             profile.set_id(i);
//         }
//         break;
//     case 2: // name
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             profile.set_name(i);
//         }
//         break;
//     case 3: // system name
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             profile.set_system_name(i);
//         }
//         break;
//     case 4: // file name
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             profile.set_file_name(i);
//         }
//         break;
//     case 5: // start line
//         if (wire == 0)
//         {
//             uint64_t i;
//             input->ReadVarint64(&i);
//             profile.set_start_line(i);
//         }
//         break;
//     }
//     return true;
// }