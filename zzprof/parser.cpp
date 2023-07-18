#include "parser.h"

Parser::Parser(const std::string &fn) {
  filename = fn;
  THRESHOLD = 8e8;
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

void Parser::print_difference() {
  // cout << "Comparing Difference" << endl;
  // if (!gp::util::MessageDifferencer::ApproximatelyEquivalent(profile, correct_profile)) {
  //   std::cout << "NOT APPROX EQUIV\n";
  // } else {
  //   std::cout << "APPROX EQUIV\n";
  // }
  if (!gp::util::MessageDifferencer::Equivalent(*sub_profiles[0], profile_correct)) {
    std::cout << "NOT EQUIV\n";
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

  while (tag != 0) {
    field_id = tag >> 3;
    wire = tag & 7;
    // handle a record

    switch (field_id) {
      case 1:
        //   repeated ValueType sample_type = 1;

      case 2:
        //   repeated Sample sample = 2;

      case 3:
        //  repeated Mapping mapping = 3;

      case 4:
        //  repeated Location location = 4;

      case 5:
        //  repeated Function function = 5;

      case 6:
        //  repeated string string_table = 6;
        //  repeated strings are fine.
        if (wire == 2) {
          input->ReadVarint32(&len);
          input->Skip(len);
        }
        break;
      case 7:
        //  int64 drop_frames = 7;
        if (wire == 0) {
          uint64_t i;
          input->ReadVarint64(&i);
        }
        break;
      case 8:
        //  int64 keep_frames = 8;
        if (wire == 0) {
          uint64_t i;
          input->ReadVarint64(&i);
        }
        break;
      case 9:
        //  int64 time_nanos = 9;
        if (wire == 0) {
          uint64_t i;
          input->ReadVarint64(&i);
        }
        break;
      case 10:
        //  int64 duration_nanos = 10;
        if (wire == 0) {
          uint64_t i;
          input->ReadVarint64(&i);
        }
        break;
      case 11:
        //  ValueType period_type = 11;
        if (wire == 2) {
          input->ReadVarint32(&len);
          input->Skip(len);
        }
        break;
      case 12:
        //  int64 period = 12;
        if (wire == 0) {
          uint64_t i;
          input->ReadVarint64(&i);
        }
        break;
      case 13:  // ALL REPEATS ARE PACKED -> LEN
        //   repeated int64 comment = 13;
        if (wire == 2) {
          input->ReadVarint32(&len);
          input->Skip(len);
        }
        break;
      case 14:
        //  int64 default_sample_type = 14;
        if (wire == 0) {
          uint64_t i;
          input->ReadVarint64(&i);
        }
        break;
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
    if (input->CurrentPosition() - startPos > THRESHOLD) {
      sub_profiles.push_back(new pp::Profile);
      pool.push_task(&Parser::consumeChunk, this, startPos, input->CurrentPosition(), index++);
      startPos = input->CurrentPosition();
      THRESHOLD /= 8;
      THRESHOLD *= 6;
    }

    tag = input->ReadTag();
  }
  sub_profiles.push_back(new pp::Profile);
  pool.push_task(&Parser::consumeChunk, this, startPos, input->CurrentPosition(), index);

  end_T = std::chrono::system_clock::now();
  elapsed_seconds = end_T - start_T;
  cout << "master thread time with threshold " << THRESHOLD << ": " << elapsed_seconds.count() << endl;
  pool.wait_for_tasks();

  // make this parallel

  start_T = std::chrono::system_clock::now();

  cout << "starting merge" << endl;
  for (int i = 1; i < sub_profiles.size(); i++) {
    sub_profiles[0]->MergeFrom(*sub_profiles[i]);
  }
  end_T = std::chrono::system_clock::now();
  elapsed_seconds = end_T - start_T;
  cout << "merging time for " << THRESHOLD << ": " << elapsed_seconds.count() << endl;

  // int add(list, l, r) {
  //   if (r - l < thresh) {
  //     seq add
  //   } else {
  //     parallel recL = add(l, m);
  //     parallel recR = add(m + 1, r);
  //     return recL + recR;
  //   }
  // }
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