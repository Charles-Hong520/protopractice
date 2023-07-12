/*
g++ -std=c++17 genwidget.cpp widget.pb.cc -lprotobuf -lpthread -o gen && ./gen
*/
#include <google/protobuf/message.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <ios>
#include <stdio.h>
#include <filesystem>
#include "widget.pb.h"  // Replace with the name of your generated header file.
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

void gen_window(SomeMessage_Window* window, int input) {
    if(input == 1) {
        window->set_title("Sample Konfabulator Widget");
        window->set_name("main_window");
        window->set_width(500);
        window->set_height(500);
    } else {
        window->set_title("2 Konfabulator Widget");
        window->set_name("main_window");
        window->set_width(500);
        window->set_height(500);
    }
}

void gen_image(SomeMessage_Image* img) {
    img->set_src("Images/Sun.png");
    img->set_name("sun1");
    img->set_hoffset(250);
    img->set_voffset(250);
    img->set_alignment("center");
}

void gen_text(SomeMessage_Text* text) {
    text->set_data("Click Here");
    text->set_size(36);
    text->set_style("bold");
    text->set_name("text1");
    text->set_hoffset(250);
    text->set_voffset(100);
    text->set_alignment("center");
    text->set_onmouseup("sun1.opacity = (sun1.opacity / 100) * 90;");
}

void gen_widget(SomeMessage_Widget* wid) {

    wid->set_debug("on");
    SomeMessage_Window* win1 = wid->add_window();
    SomeMessage_Window* win2 = wid->add_window();
    SomeMessage_Image* image = wid->mutable_image();
    SomeMessage_Text* text = wid->mutable_text();

    gen_window(win1, 1);
    gen_window(win2, 2);

    gen_image(image);
    gen_text(text);

}

int main() {
    SomeMessage sm;
    SomeMessage_Widget* w = sm.mutable_widget();
    gen_widget(w);


    std::string name = "manual.bin";
    std::fstream output(name, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!sm.SerializeToOstream(&output)) {
        std::cerr << "Failed to write widget." << std::endl;
        return 1;
    }
    output.close();
    return 0;
}