#include "parser.h"

Parser::Parser(const std::string& fn) {
  filename = fn;
  constexpr size_t MAX_CONTENT_SIZE = 1000000;  // Set an appropriate maximum content size
  content = new uint8_t[MAX_CONTENT_SIZE];
  content_size = 0;
  load_content();
  input = new gp::io::CodedInputStream(content, content_size);
}

Parser::~Parser() {
  if (input) delete input;
  // if(content) delete[] content;
  munmap(content, content_size);
  content = nullptr;
}

void Parser::load_content() {
  std::ifstream file(filename, std::ios::binary);
  struct stat st;
  stat(filename.c_str(), &st);
  content_size = st.st_size;

  int fhand = open(filename.c_str(), O_RDONLY);
  content = (uint8_t*)mmap(0, content_size, PROT_READ, MAP_FILE | MAP_PRIVATE, fhand, 0);
}
bool Parser::consume() {
  uint32_t tag = input->ReadTag();
  // cout<<input->CurrentPosition()<<endl;
  if (tag == 0) {
    // encounter EOF or tag == 0
    // 1. EOF: when ReadTag() returns 0 when called exactly at EOF
    // 2. tag parsed as 0: a byte 0x00 is interpreted as a tag, indicating a wrong parse
    return false;
  }
  uint32_t field_id = tag >> 3;
  uint32_t wire = tag & 7;
  // handle a record
  switch (field_id) {
    case 1:  // name
      if (wire == 2) {
        uint32_t len;
        input->ReadVarint32(&len);
        std::string buffer;
        input->ReadString(&buffer, len);
        person.set_name(buffer);
      }
      break;
    case 2:  // age
      if (wire == 0) {
        uint32_t i;
        input->ReadVarint32(&i);
        person.set_age(i);
      }
      break;
    case 3:  // phone number
      if (wire == 2) {
        // repeat email
        uint32_t len;
        input->ReadVarint32(&len);
        std::string buffer;
        input->ReadString(&buffer, len);
        person.set_phone_numbers(buffer);
      }
      break;
    case 4:  // hobby
      if (wire == 2) {
        uint32_t len;
        input->ReadVarint32(&len);
        std::string buffer;
        input->ReadString(&buffer, len);
        person.set_hobby(buffer);
      }
      break;
    case 5:  // message addr
      if (wire == 2) {
        uint32_t len;
        input->ReadVarint32(&len);

        uint32_t currPos = input->CurrentPosition();
        Address* addr = person.add_addr();
        input->Skip(len);

        // thread pool

        // trds[i]=std::thread(&Parser::give_to_worker, this, currPos, len, addr);
        // pool.push_task(&Parser::give_to_worker, this, currPos, len, addr);

        // seq
        gp::io::CodedInputStream* copyStream = new gp::io::CodedInputStream(
            content + currPos, len);
        addr->ParseFromCodedStream(copyStream);
      }
      break;
  }
  return true;
}

bool Parser::consumeAddress() {
  return true;
}

void Parser::give_to_worker(uint32_t currPos, uint32_t len, Address* addr) {
  // std::chrono::time_point<std::chrono::system_clock>  start = std::chrono::system_clock::now();
  gp::io::CodedInputStream* copyStream = new gp::io::CodedInputStream(
      content + currPos, len);
  addr->ParseFromCodedStream(copyStream);
  // std::chrono::time_point<std::chrono::system_clock>  end = std::chrono::system_clock::now();
  // std::chrono::duration<double>  elapsed_seconds = end - start;
  // sync_out.println("elapsed time for CUSTOM: ",currPos," ",elapsed_seconds.count(),"s");
}

double Parser::print_difference(const Person& correct_person) {
  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
  while (consume()) {
  }
  pool.wait_for_tasks();

  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  // cout<<"custom time total: "<<elapsed_seconds.count()<<"s"<<endl;

  if (!gp::util::MessageDifferencer::Equals(person, correct_person)) {
    if (std::string s; gp::TextFormat::PrintToString(person, &s)) {
      std::cout << "Your message: " << s;
    } else {
      std::cerr << "Message not valid (partial content: "
                << person.ShortDebugString() << ")\n";
    }
    cout << endl;
    if (std::string s; gp::TextFormat::PrintToString(correct_person, &s)) {
      std::cout << "correct message: " << s;
    } else {
      std::cerr << "Message not valid (partial content: "
                << correct_person.ShortDebugString() << ")\n";
    }
  }
  return elapsed_seconds.count();
}