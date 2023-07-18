#include "parser.h"

Parser::Parser(const std::string &fn) {
  filename = fn;
  THRESHOLD = 1000000;
  constexpr size_t MAX_CONTENT_SIZE = 1e9;  // Set an appropriate maximum content size
  content = new uint8_t[MAX_CONTENT_SIZE];
  content_size = 0;
  load_content();
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

void Parser::print_difference(const pp::Profile &correct_profile) {
  // cout << "Comparing Difference" << endl;
  // if (!gp::util::MessageDifferencer::ApproximatelyEquivalent(profile, correct_profile)) {
  //   std::cout << "NOT APPROX EQUIV\n";
  // } else {
  //   std::cout << "APPROX EQUIV\n";
  // }
  if (!gp::util::MessageDifferencer::Equivalent(profile, correct_profile)) {
    std::cout << "NOT EQUIV\n";
  } else {
    std::cout << "EQUIV\n";
  }
}

void Parser::parseProfile() {
  // profile.ParseFromCodedStream(input);
  cout << std::fixed << std::setprecision(4);

  cout << endl;

  cout << "case occurance profile" << endl;
  for (auto &[num, val] : timemap) {
    cout << num << ":\t" << val << endl;
  }
  cout << endl;

  cout << "len occurance profile" << endl;
  for (auto &[len, occur] : sizemap) {
    cout << len << ":\t" << occur << endl;
  }
  cout << endl;
}

void Parser::consumeProfile() {
  gp::io::CodedInputStream *input = new gp::io::CodedInputStream(content, content_size);
  uint32_t tag = input->ReadTag();
  uint32_t field_id, wire, currPos, len;

  int cnt = 0;

  while (tag != 0) {
    len = 0;
    field_id = tag >> 3;
    wire = tag & 7;
    // handle a record
    switch (field_id) {
      case 1:
        //   repeated ValueType sample_type = 1;
        if (wire == 2) {
          input->ReadVarint32(&len);
          pp::ValueType *msg = profile.add_sample_type();

          if (len > THRESHOLD) {
            // thread pool
            currPos = input->CurrentPosition();
            input->Skip(len);

            pool.push_task(&Parser::consumeValueType, this, currPos, len, msg);
            //    create new stream with limits
            //    consume will be handled by this function by default
          } else {
            // seq
            // use pushlimit and poplimit
            // use API
            auto lim = input->PushLimit(len);
            msg->ParseFromCodedStream(input);
            input->PopLimit(lim);
          }
        }
        break;
      case 2:
        //   repeated Sample sample = 2;
        if (wire == 2) {
          input->ReadVarint32(&len);
          // pp::Sample *msg = profile.add_sample();

          if (len > THRESHOLD) {
            // thread pool
            currPos = input->CurrentPosition();
            input->Skip(len);

            // pool.push_task(&Parser::consumeSample, this, currPos, len, msg);
            //    create new stream with limits
            //    consume will be handled by this function by default
          } else {
            // seq
            // use pushlimit and poplimit
            // use API
            auto lim = input->PushLimit(len);
            if (cnt < 5) {
              pp::Sample *samplemsg = profile_sample.add_sample();
              samplemsg->ParseFromCodedStream(input);
              cnt++;
              cout << "Writing " << cnt << endl;
            } else {
              pp::Sample *msg = profile.add_sample();
              msg->ParseFromCodedStream(input);
            }

            // seqConsumeSample(input, msg, len);
            input->PopLimit(lim);
          }
        }
        break;
      case 3:
        //  repeated Mapping mapping = 3;
        if (wire == 2) {
          input->ReadVarint32(&len);
          pp::Mapping *msg = profile.add_mapping();

          if (len > THRESHOLD) {
            // thread pool
            currPos = input->CurrentPosition();
            input->Skip(len);

            pool.push_task(&Parser::consumeMapping, this, currPos, len, msg);
            //    create new stream with limits
            //    consume will be handled by this function by default
          } else {
            // seq
            // use pushlimit and poplimit
            // use API
            auto lim = input->PushLimit(len);
            msg->ParseFromCodedStream(input);
            input->PopLimit(lim);
          }
        }
        break;
      case 4:
        //  repeated Location location = 4;
        if (wire == 2) {
          input->ReadVarint32(&len);
          pp::Location *msg = profile.add_location();

          if (len > THRESHOLD) {
            // thread pool
            currPos = input->CurrentPosition();
            input->Skip(len);

            pool.push_task(&Parser::consumeLocation, this, currPos, len, msg);
            //    create new stream with limits
            //    consume will be handled by this function by default
          } else {
            // seq
            // use pushlimit and poplimit
            // use API
            auto lim = input->PushLimit(len);
            msg->ParseFromCodedStream(input);
            input->PopLimit(lim);
          }
        }
        break;
      case 5:
        //  repeated Function function = 5;
        if (wire == 2) {
          input->ReadVarint32(&len);
          pp::Function *msg = profile.add_function();

          if (len > THRESHOLD) {
            // thread pool

            currPos = input->CurrentPosition();
            input->Skip(len);

            pool.push_task(&Parser::consumeFunction, this, currPos, len, msg);
            //    create new stream with limits
            //    consume will be handled by this function by default
          } else {
            // seq
            // use pushlimit and poplimit
            // use API
            auto lim = input->PushLimit(len);
            msg->ParseFromCodedStream(input);
            input->PopLimit(lim);
          }
        }
        break;
      case 6:
        //   repeated string string_table = 6;
        //  repeated strings are fine.
        if (wire == 2) {
          uint32_t len;
          input->ReadVarint32(&len);
          std::string buffer;
          input->ReadString(&buffer, len);
          profile.add_string_table(buffer);
        }
        break;
      case 7:
        //   int64 drop_frames = 7;
        if (wire == 0) {
          uint64_t i;
          input->ReadVarint64(&i);
          profile.set_drop_frames(i);
        }
        break;
      case 8:
        //  int64 keep_frames = 8;
        if (wire == 0) {
          uint64_t i;
          input->ReadVarint64(&i);
          profile.set_keep_frames(i);
        }
        break;
      case 9:
        //  int64 time_nanos = 9;
        if (wire == 0) {
          uint64_t i;
          input->ReadVarint64(&i);
          profile.set_time_nanos(i);
        }
        break;
      case 10:
        //  int64 duration_nanos = 10;
        if (wire == 0) {
          uint64_t i;
          input->ReadVarint64(&i);
          profile.set_duration_nanos(i);
        }
        break;
      case 11:
        //  ValueType period_type = 11;
        if (wire == 2) {
          input->ReadVarint32(&len);
          pp::ValueType *msg = profile.mutable_period_type();

          if (len > THRESHOLD) {
            // thread pool
            currPos = input->CurrentPosition();
            input->Skip(len);

            pool.push_task(&Parser::consumeValueType, this, currPos, len, msg);
            //    create new stream with limits
            //    consume will be handled by this function by default
          } else {
            // seq
            // use pushlimit and poplimit
            // use API
            auto lim = input->PushLimit(len);
            msg->ParseFromCodedStream(input);
            input->PopLimit(lim);
          }
        }
        break;
      case 12:
        //  int64 period = 12;
        if (wire == 0) {
          uint64_t i;
          input->ReadVarint64(&i);
          profile.set_period(i);
        }
        break;
      case 13:  // ALL REPEATS ARE PACKED -> LEN
        //   repeated int64 comment = 13;
        if (wire == 2) {
          uint32_t len;
          input->ReadVarint32(&len);

          // can be parallelized
          uint64_t end = len + input->CurrentPosition();
          uint64_t i;
          while (input->CurrentPosition() < end) {
            input->ReadVarint64(&i);
            profile.add_comment(i);
          }
        }
        break;
      case 14:
        //  int64 default_sample_type = 14;
        if (wire == 0) {
          uint64_t i;
          input->ReadVarint64(&i);
          profile.set_default_sample_type(i);
        }
        break;
    }

    tag = input->ReadTag();
    // sizemap[len]++;
    // timemap[field_id]++;
  }

  std::string name = "sample.bin";
  std::fstream output(name, std::ios::out | std::ios::trunc | std::ios::binary);
  if (!profile_sample.SerializeToOstream(&output)) {
    std::cerr << "Failed to write profile sample." << std::endl;
    return;
  }
  output.close();
  std::string samplestr;
  google::protobuf::TextFormat::PrintToString(profile_sample, &samplestr);
  std::ofstream fout("sample.str");
  fout << samplestr << std::endl;
  pool.wait_for_tasks();
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
  while (tag != 0) {
    uint32_t field_id = tag >> 3;
    uint32_t wire = tag & 7;
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

void Parser::seqConsumeSample(gp::io::CodedInputStream *input, pp::Sample *msg, uint32_t msglen) {
  uint32_t start = input->CurrentPosition();

  uint32_t tag = input->ReadTag();
  uint32_t field_id, wire, len;
  uint32_t end;
  while (input->CurrentPosition() - start < msglen) {
    field_id = tag >> 3;
    wire = tag & 7;
    // handle a record

    switch (field_id) {
      case 1:  // location id
        if (wire == 2) {
          input->ReadVarint32(&len);
          // can be parallelized
          end = len + input->CurrentPosition();
          uint64_t i;

          cout << "len: " << len << endl;

          while (input->CurrentPosition() < end) {
            input->ReadVarint64(&i);
            msg->add_location_id(i);
            cout << i << endl;
          }
          cout << "leaving case 1, wire 2" << endl;
        } else if (wire == 0) {
          uint64_t i;
          input->ReadVarint64(&i);
          msg->add_location_id(i);
          cout << i << endl;
          cout << "leaving case 1, wire 0" << endl;
        }
        break;
      case 2:  // value
        if (wire == 2) {
          input->ReadVarint32(&len);
          // can be parallelized
          cout << "len: " << len << endl;
          end = len + input->CurrentPosition();
          uint64_t i;
          while (input->CurrentPosition() < end) {
            input->ReadVarint64(&i);
            msg->add_value(i);
            cout << i << endl;
          }
          cout << "leaving case 2, wire 2" << endl;
        } else if (wire == 0) {
          uint64_t i;
          input->ReadVarint64(&i);
          msg->add_value(i);
          cout << i << endl;
          cout << "leaving case 2, wire 0" << endl;
        }
        break;
      case 3:  // label
        if (wire == 2) {
          input->ReadVarint32(&len);

          uint32_t currPos = input->CurrentPosition();
          pp::Label *label = msg->add_label();
          auto lim = input->PushLimit(len);

          msg->ParseFromCodedStream(input);
          // seqConsumeLabel(input, label);
          input->PopLimit(lim);
          cout << "leaving case 3 in sample" << endl;
        }
        break;
      default: {
      }
    }
    // cout << field_id << " " << input->CurrentPosition() << endl;
    tag = input->ReadTag();
  }
  cout << "leaving seqsample\n\n";
}

void Parser::consumeSample(uint32_t currPos, uint32_t len, pp::Sample *msg) {
  gp::io::CodedInputStream *input = new gp::io::CodedInputStream(
      content + currPos, len);
  msg->ParseFromCodedStream(input);
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