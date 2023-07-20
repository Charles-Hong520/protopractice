repeated2msg = {
    1: ("ValueType", "sample_type"),
    2: ("Sample", "sample"),
    3: ("Mapping", "mapping"),
    4: ("Location", "location"),
    5: ("Function", "function"),
    5: ("Function", "function"),
    6: ("std::string", "string_table"),
}

for field_id, var in repeated2msg.items():
    reserve = f"profile.mutable_{var[1]}()->Reserve(tracker[{field_id}].size());"
    print(reserve)

# for field_id, var in repeated2msg.items():
#     parfor = f"parlay::parallel_for(0, tracker[{field_id}].size(), [&](int i)"
#     parfor += "{\n"
#     parfor += f"gp::io::CodedInputStream *input = new gp::io::CodedInputStream(content + tracker[{field_id}][i].first, tracker[{field_id}][i].second);"
#     parfor += "\n"
#     parfor += f"profile.mutable_{var[1]}(i)->ParseFromCodedStream(input);\n"
#     parfor += "});\n"
#     print(parfor)


for field_id, var in repeated2msg.items():
    parfor = f"for(int i = 0; i<tracker[{field_id}].size(); i++)"
    parfor += "{\n"
    parfor += f"gp::io::CodedInputStream *input = new gp::io::CodedInputStream(content + tracker[{field_id}][i].first, tracker[{field_id}][i].second);"
    parfor += "\n"
    parfor += f"profile.mutable_{var[1]}(i)->ParseFromCodedStream(input);\n"
    parfor += "}\n"
    print(parfor)


# parlay::parallel_for(0, tracker[field_id].size(), [&](int i) {
#     #  // assign ith element in l to ith element in sample array
#
#      (prof.*mutable)(i)->ParseFromCodedStream(input);
