#include "parser.h"

Parser::Parser(const std::string &fn) {
    filename = fn;
    THRESHOLD = 8e8;
    constexpr size_t MAX_CONTENT_SIZE = 1e9;  // Set an appropriate maximum content size
    content = new uint8_t[MAX_CONTENT_SIZE];
    content_size = 0;
    load_content();

    tracker.resize(15);
}
void Parser::load_content() {
    std::ifstream file(filename, std::ios::binary);
    struct stat st;
    stat(filename.c_str(), &st);
    content_size = st.st_size;

    int fhand = open(filename.c_str(), O_RDONLY);
    content = (uint8_t *)mmap(0, content_size, PROT_READ, MAP_FILE | MAP_PRIVATE, fhand, 0);
}

Parser::~Parser() {
    // if(content) delete[] content;
    munmap(content, content_size);
    content = nullptr;
}

void Parser::print_difference() {
    // cout << "Comparing Difference" << endl;
    // if (!gp::util::MessageDifferencer::ApproximatelyEquivalent(profile, correct_profile)) {
    //   std::cout << "NOT APPROX EQUIV\n";
    // } else {
    //   std::cout << "APPROX EQUIV\n";
    // }
    if (!gp::util::MessageDifferencer::Equivalent(profile, profile_correct)) {
        std::cout << "NOT EQUIV\n";
        std::string s_correct, s_manual;
        profile_correct.SerializeToString(&s_correct);
        profile.SerializeToString(&s_manual);
        cout << "size correct: " << s_correct.size() << endl;
        cout << "size manual: " << s_manual.size() << endl;

        for (int i = 0; i < s_correct.size(); i++) {
            if (s_correct[i] != s_manual[i]) {
                cout << "not match at " << i << endl;
                break;
            }
        }

    } else {
        std::cout << "EQUIV\n";
    }
}

void Parser::printLogistics() {
    // profile.ParseFromCodedStream(input);
    // cout << std::fixed << std::setprecision(4);

    cout << endl;

    cout << "len occurance profile" << endl;
    for (auto &[len, occur] : sizemap) {
        cout.width(10);
        std::string lenstr = std::to_string(len) + "-" + std::to_string(len + 29) + ":";
        cout << std::left << lenstr;
        cout.width(10);
        cout << std::left << occur << endl;
    }
    int sum = 0;
    cout << endl;

    cout << "packed order and their length" << endl;
    for (auto p : packedcheck) {
        cout << p[0] << ":\t" << p[1] << endl;
        sum += p[1];
    }

    cout << endl;

    cout << "total bytes of file: " << sum << endl;
}

void Parser::consumeChunk(uint32_t start, uint32_t end, uint32_t index) {
    pp::Profile profile_local;

    cout << start << " enter " << end << endl;
    gp::io::CodedInputStream *input = new gp::io::CodedInputStream(
        content + start, end - start);

    sub_profiles[index]->ParseFromCodedStream(input);
    cout << start << " exit " << end << endl;
}

void Parser::consumeProfile() {
    start_T = std::chrono::system_clock::now();
    uint32_t index = 0;
    gp::io::CodedInputStream *input = new gp::io::CodedInputStream(content, content_size);

    uint32_t startPos = input->CurrentPosition();

    uint32_t tag = input->ReadTag();
    uint32_t field_id, wire, currPos, len;
    uint64_t i;

    while (tag != 0) {
        field_id = tag >> 3;
        wire = tag & 7;
        // handle a record

        if (wire == 0) {
            input->ReadVarint64(&i);
            switch (field_id) {
                case 7:
                    //  int64 drop_frames = 7;
                    profile.set_drop_frames(i);
                    break;
                case 8:
                    //  int64 keep_frames = 8;
                    profile.set_keep_frames(i);
                    break;
                case 9:
                    //  int64 time_nanos = 9;
                    profile.set_time_nanos(i);
                    break;
                case 10:
                    //  int64 duration_nanos = 10;
                    profile.set_duration_nanos(i);
                    break;
                case 12:
                    //  int64 period = 12;
                    profile.set_period(i);
                    break;
                case 14:
                    //  int64 default_sample_type = 14;
                    profile.set_default_sample_type(i);
                    break;
            }
        } else if (wire == 2) {
            input->ReadVarint32(&len);
            tracker[field_id].push_back({input->CurrentPosition(), len});
            input->Skip(len);
        }

        //   sizemap[(input->CurrentPosition() - startPos) / 30 * 30]++;
        //   if (packedcheck.empty()) {
        //     packedcheck.push_back({field_id, (input->CurrentPosition() - startPos)});
        //   } else {
        //     if (packedcheck.back()[0] == field_id) {
        //       packedcheck.back()[1] += (input->CurrentPosition() - startPos);
        //     } else {
        //       packedcheck.push_back({field_id, (input->CurrentPosition() - startPos)});
        //     }
        //   }
        //   startPos = input->CurrentPosition();
        tag = input->ReadTag();
    }

    profile.mutable_sample_type()->Reserve(tracker[1].size());
    profile.mutable_sample()->Reserve(tracker[2].size());
    profile.mutable_mapping()->Reserve(tracker[3].size());
    profile.mutable_location()->Reserve(tracker[4].size());
    profile.mutable_function()->Reserve(tracker[5].size());
    profile.mutable_string_table()->Reserve(tracker[6].size());

    // intiniatize each reserved vector
    for (int i = 0; i < tracker[1].size(); i++) {
        profile.add_sample_type();
    }
    for (int i = 0; i < tracker[2].size(); i++) {
        profile.add_sample();
    }
    for (int i = 0; i < tracker[3].size(); i++) {
        profile.add_mapping();
    }
    for (int i = 0; i < tracker[4].size(); i++) {
        profile.add_location();
    }
    for (int i = 0; i < tracker[5].size(); i++) {
        profile.add_function();
    }
    for (int i = 0; i < tracker[6].size(); i++) {
        profile.add_string_table();
    }

    end_T = std::chrono::system_clock::now();
    elapsed_seconds = end_T - start_T;
    cout << "master thread time with threshold " << THRESHOLD << ": " << elapsed_seconds.count() << endl;
    // cout << profile.mutable_sample()->Capacity() << endl;
    // cout << profile.mutable_sample()->size() << endl;
    // cout << tracker[2].size() << endl;

    // cout << endl;
    // for (int i = 0; i < tracker[1].size(); i++) {
    //   cout << "creating stream " << i << endl;
    //   gp::io::CodedInputStream *input = new gp::io::CodedInputStream(content + tracker[1][i].first, tracker[1][i].second);
    //   cout << "parsing " << i << endl;
    //   profile.mutable_sample_type(i)->ParseFromCodedStream(input);
    // }

    // for (int i = 0; i < tracker[2].size(); i++) {
    //   gp::io::CodedInputStream *input = new gp::io::CodedInputStream(content + tracker[2][i].first, tracker[2][i].second);
    //   profile.mutable_sample(i)->ParseFromCodedStream(input);
    // }

    // for (int i = 0; i < tracker[3].size(); i++) {
    //   gp::io::CodedInputStream *input = new gp::io::CodedInputStream(content + tracker[3][i].first, tracker[3][i].second);
    //   profile.mutable_mapping(i)->ParseFromCodedStream(input);
    // }

    // for (int i = 0; i < tracker[4].size(); i++) {
    //   gp::io::CodedInputStream *input = new gp::io::CodedInputStream(content + tracker[4][i].first, tracker[4][i].second);
    //   profile.mutable_location(i)->ParseFromCodedStream(input);
    // }

    // for (int i = 0; i < tracker[5].size(); i++) {
    //   gp::io::CodedInputStream *input = new gp::io::CodedInputStream(content + tracker[5][i].first, tracker[5][i].second);
    //   profile.mutable_function(i)->ParseFromCodedStream(input);
    // }

    // for (int i = 0; i < tracker[6].size(); i++) {
    //   gp::io::CodedInputStream *input = new gp::io::CodedInputStream(content + tracker[6][i].first, tracker[6][i].second);
    //   input->ReadString(profile.mutable_string_table(i), tracker[6][i].second);
    // }

    // parlay::parallel_for(0, tracker[6].size(), [&](int i) {
    //   gp::io::CodedInputStream *input = new gp::io::CodedInputStream(content + tracker[6][i].first, tracker[6][i].second);
    //   input->ReadString(profile.mutable_string_table(i), tracker[6][i].second);
    // });
    // cout << "exit6" << endl;

    // measure the time for each thread to parse the data
    start_T = std::chrono::system_clock::now();
    parlay::parallel_for(0, tracker[1].size(), [&](int i) {
        gp::io::CodedInputStream *input = new gp::io::CodedInputStream(content + tracker[1][i].first, tracker[1][i].second);
        profile.mutable_sample_type(i)->ParseFromCodedStream(input);
    });

    parlay::parallel_for(0, tracker[2].size(), [&](int i) {
        gp::io::CodedInputStream *input = new gp::io::CodedInputStream(content + tracker[2][i].first, tracker[2][i].second);
        profile.mutable_sample(i)->ParseFromCodedStream(input);
    });

    parlay::parallel_for(0, tracker[3].size(), [&](int i) {
        gp::io::CodedInputStream *input = new gp::io::CodedInputStream(content + tracker[3][i].first, tracker[3][i].second);
        profile.mutable_mapping(i)->ParseFromCodedStream(input);
    });

    parlay::parallel_for(0, tracker[4].size(), [&](int i) {
        gp::io::CodedInputStream *input = new gp::io::CodedInputStream(content + tracker[4][i].first, tracker[4][i].second);
        profile.mutable_location(i)->ParseFromCodedStream(input);
    });

    parlay::parallel_for(0, tracker[5].size(), [&](int i) {
        gp::io::CodedInputStream *input = new gp::io::CodedInputStream(content + tracker[5][i].first, tracker[5][i].second);
        profile.mutable_function(i)->ParseFromCodedStream(input);
    });

    parlay::parallel_for(0, tracker[6].size(), [&](int i) {
        gp::io::CodedInputStream *input = new gp::io::CodedInputStream(content + tracker[6][i].first, tracker[6][i].second);
        input->ReadString(profile.mutable_string_table(i), tracker[6][i].second);
    });

    gp::io::CodedInputStream *input11 = new gp::io::CodedInputStream(content + tracker[11][i].first, tracker[11][i].second);
    profile.mutable_period_type()->ParseFromCodedStream(input11);

    end_T = std::chrono::system_clock::now();
    elapsed_seconds = end_T - start_T;
    cout << "time for parsing: " << elapsed_seconds.count() << endl;
    // cout << "exit11" << endl;
}

// bool Parser::consumeValueType() {
//   uint32_t tag = input->ReadTag();
//   if (tag == 0) {
//     return false;
//   }
//   uint32_t field_id = tag >> 3;
//   uint32_t wire = tag & 7;
//   // handle a record
//   switch (field_id) {
//     case 1:  // type
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         value_type.set_type(i);
//       }
//       break;
//     case 2:  // unit
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         value_type.set_unit(i);
//       }
//       break;
//   }
//   return true;
// }

// bool Parser::consumeSample() {

// }

void Parser::seqConsumeLabel(gp::io::CodedInputStream *input, pp::Label *msg) {
    uint32_t tag = input->ReadTag();
    uint32_t field_id, wire, len;
    uint32_t end;
    while (tag != 0) {
        field_id = tag >> 3;
        wire = tag & 7;
        // handle a record
        switch (field_id) {
            case 1:  // key
                if (wire == 0) {
                    uint64_t i;
                    input->ReadVarint64(&i);
                    msg->set_key(i);
                }
                break;
            case 2:  // str
                if (wire == 0) {
                    uint64_t i;
                    input->ReadVarint64(&i);
                    msg->set_str(i);
                }
                break;
            case 3:  // num
                if (wire == 0) {
                    uint64_t i;
                    input->ReadVarint64(&i);
                    msg->set_num(i);
                }
                break;
            case 4:  // num unit
                if (wire == 0) {
                    uint64_t i;
                    input->ReadVarint64(&i);
                    msg->set_num_unit(i);
                }
                break;
        }
        tag = input->ReadTag();
    }
}

void Parser::consumeLabel(uint32_t currPos, uint32_t len, pp::Label *msg) {
    gp::io::CodedInputStream *input = new gp::io::CodedInputStream(
        content + currPos, len);
    msg->ParseFromCodedStream(input);
}

void Parser::consumeMapping(uint32_t currPos, uint32_t len, pp::Mapping *msg) {
    gp::io::CodedInputStream *input = new gp::io::CodedInputStream(
        content + currPos, len);
    msg->ParseFromCodedStream(input);
}

void Parser::seqConsumeSample(gp::io::CodedInputStream *input, pp::Sample *msg) {
    uint32_t tag = input->ReadTag();
    uint32_t field_id, wire, len;
    uint32_t end;
    while (tag != 0) {
        field_id = tag >> 3;
        wire = tag & 7;
        // handle a record

        switch (field_id) {
            case 1:  // location id
                if (wire == 2) {
                    input->ReadVarint32(&len);
                    end = len + input->CurrentPosition();
                    uint64_t i;
                    while (input->CurrentPosition() < end) {
                        input->ReadVarint64(&i);
                        msg->add_location_id(i);
                    }
                } else if (wire == 0) {
                    uint64_t i;
                    input->ReadVarint64(&i);
                    msg->add_location_id(i);
                }
                break;
            case 2:  // value
                if (wire == 2) {
                    input->ReadVarint32(&len);
                    end = len + input->CurrentPosition();
                    uint64_t i;
                    while (input->CurrentPosition() < end) {
                        input->ReadVarint64(&i);
                        msg->add_value(i);
                    }
                } else if (wire == 0) {
                    uint64_t i;
                    input->ReadVarint64(&i);
                    msg->add_value(i);
                }
                break;
            case 3:  // label
                if (wire == 2) {
                    input->ReadVarint32(&len);

                    uint32_t currPos = input->CurrentPosition();
                    pp::Label *label = msg->add_label();
                    auto lim = input->PushLimit(len);

                    // label->ParseFromCodedStream(input);
                    seqConsumeLabel(input, label);
                    input->PopLimit(lim);
                }
                break;
            default: {
            }
        }
        tag = input->ReadTag();
    }
}

void Parser::consumeSample(uint32_t currPos, uint32_t len, pp::Sample *msg) {
    gp::io::CodedInputStream *input = new gp::io::CodedInputStream(
        content + currPos, len);
    // msg->ParseFromCodedStream(input);
    seqConsumeSample(input, msg);
}

void Parser::consumeLocation(uint32_t currPos, uint32_t len, pp::Location *msg) {
    gp::io::CodedInputStream *input = new gp::io::CodedInputStream(
        content + currPos, len);
    msg->ParseFromCodedStream(input);
}

void Parser::consumeLine(uint32_t currPos, uint32_t len, pp::Line *msg) {
    gp::io::CodedInputStream *input = new gp::io::CodedInputStream(
        content + currPos, len);
    msg->ParseFromCodedStream(input);
}

void Parser::consumeFunction(uint32_t currPos, uint32_t len, pp::Function *msg) {
    gp::io::CodedInputStream *input = new gp::io::CodedInputStream(
        content + currPos, len);
    msg->ParseFromCodedStream(input);
}

void Parser::consumeValueType(uint32_t currPos, uint32_t len, pp::ValueType *msg) {
    gp::io::CodedInputStream *input = new gp::io::CodedInputStream(
        content + currPos, len);
    msg->ParseFromCodedStream(input);
}
//   uint32_t tag = input->ReadTag();
//   if (tag == 0) {
//     return false;
//   }
//   uint32_t field_id = tag >> 3;
//   uint32_t wire = tag & 7;
//   // handle a record
//   switch (field_id) {
//     case 1:  // id
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         profile.set_id(i);
//       }
//       break;
//     case 2:  // memory start
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         profile.set_memory_start(i);
//       }
//       break;
//     case 3:  // memory limit
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         profile.set_memory_limit(i);
//       }
//       break;
//     case 4:  // file offset
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         profile.set_file_offset(i);
//       }
//       break;
//     case 5:  // filename
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         profile.set_filename(i);
//       }
//       break;
//     case 6:  // build id
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         profile.set_build_id(i);
//       }
//       break;
//     case 7:  // has functions
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         profile.set_has_functions(i);
//       }
//       break;
//     case 8:  // has filenames
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         profile.set_has_filenames(i);
//       }
//       break;
//     case 9:  // has line numbers
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         profile.set_has_line_numbers(i);
//       }
//       break;
//     case 10:  // has inline frames
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         profile.set_has_inline_frames(i);
//       }
//       break;
//   }
//   return true;
// }

// bool Parser::consumeLocation() {
//   uint32_t tag = input->ReadTag();
//   if (tag == 0) {
//     return false;
//   }
//   uint32_t field_id = tag >> 3;
//   uint32_t wire = tag & 7;
//   // handle a record
//   switch (field_id) {
//     case 1:  // id
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         profile.set_id(i);
//       }
//       break;
//     case 2:  // mappping id
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         profile.set_mapping_id(i);
//       }
//       break;
//     case 3:  // address
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         profile.set_address(i);
//       }
//       break;
//     case 4:  // line
//       if (wire == 2) {
//         uint32_t len;
//         input->ReadVarint32(&len);

//         uint32_t currPos = input->CurrentPosition();
//         pp::Line *line = profile.add_line();
//         input->Skip(len);

//         // thread pool
//         //  pool.push_task(&Parser::give_to_worker, this, currPos, len, line);

//         // seq
//         gp::io::CodedInputStream *copyStream = new gp::io::CodedInputStream(
//             content + currPos, len);
//         line->ParseFromCodedStream(copyStream);
//       }
//       break;
//     case 5:  // is folded
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         profile.set_is_folded(i);
//       }
//       break;
//   }
// }

// bool Parser::consumeLine() {
//   uint32_t tag = input->ReadTag();
//   if (tag == 0) {
//     return false;
//   }
//   uint32_t field_id = tag >> 3;
//   uint32_t wire = tag & 7;
//   // handle a record
//   switch (field_id) {
//     case 1:  // function id
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         line.set_function_id(i);
//       }
//       break;
//     case 2:  // line
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         line.set_line(i);
//       }
//       break;
//   }
// }

// bool Parser::consumeFunction() {
//   uint32_t tag = input->ReadTag();
//   if (tag == 0) {
//     return false;
//   }
//   uint32_t field_id = tag >> 3;
//   uint32_t wire = tag & 7;
//   // handle a record
//   switch (field_id) {
//     case 1:  // id
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         profile.set_id(i);
//       }
//       break;
//     case 2:  // name
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         profile.set_name(i);
//       }
//       break;
//     case 3:  // system name
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         profile.set_system_name(i);
//       }
//       break;
//     case 4:  // file name
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         profile.set_file_name(i);
//       }
//       break;
//     case 5:  // start line
//       if (wire == 0) {
//         uint64_t i;
//         input->ReadVarint64(&i);
//         profile.set_start_line(i);
//       }
//       break;
//   }
//   return true;
// }