import json
import argparse


def json_to_protobuf(json_schema):
    protobuf_schema = 'syntax = "proto3";\n\n'
    protobuf_schema += handle_message("Root", json_schema, 1)
    return protobuf_schema


def handle_message(message_name, message_fields, index):
    protobuf_schema = f"message {message_name.capitalize()} " + "{\n"
    if "properties" in message_fields:
        for field_name, field_type in message_fields["properties"].items():
            protobuf_schema += handle_field(field_name, field_type, index)
            index += 1
    protobuf_schema += "}\n\n"
    return protobuf_schema


def handle_field(field_name, field_type, index):
    if field_type["type"] == "object":
        nested_message = handle_message(field_name.capitalize(), field_type, 1)
        return nested_message + f"  {field_name.capitalize()} {field_name} = {index};\n"
    elif field_type["type"] == "array":
        protobuf_field_type = json_type_to_protobuf_type(field_type["items"]["type"])
        return f"  repeated {protobuf_field_type} {field_name} = {index};\n"
    else:
        protobuf_field_type = json_type_to_protobuf_type(field_type["type"])
        return f"  {protobuf_field_type} {field_name} = {index};\n"


def json_type_to_protobuf_type(json_type):
    if json_type == "string":
        return "string"
    elif json_type == "integer":
        return "int32"
    elif json_type == "boolean":
        return "bool"
    else:
        raise ValueError(f"Unsupported JSON type: {json_type}")


def main():
    parser = argparse.ArgumentParser(
        description="Convert JSON schema to Protobuf schema."
    )
    parser.add_argument("input_file", help="Input JSON schema file")
    parser.add_argument("output_file", help="Output Protobuf schema file")

    args = parser.parse_args()

    with open(args.input_file, "r") as f:
        json_schema = json.load(f)

    protobuf_schema = json_to_protobuf(json_schema)

    with open(args.output_file, "w") as f:
        f.write(protobuf_schema)


if __name__ == "__main__":
    main()
