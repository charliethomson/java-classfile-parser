//
// Created by charlie.thomson on 10/19/2022.
//

#ifndef SPEED_CLASSFILEPARSER_H
#define SPEED_CLASSFILEPARSER_H

#include <cstdint>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <variant>

template <class T>
void swap_endian(T *t);
typedef uint32_t u4;
typedef uint16_t u2;
typedef uint8_t u1;


template <class T>
T read_un(std::ifstream &file, std::streamsize n);

#define o(T,S) T read_##T(std::ifstream &file);
o(u4,4)
o(u2,2)
o(u1,1)
#undef o

template <class T>
std::vector<T> read_array(std::ifstream &file, std::function<T(std::ifstream &file)> parser, size_t offset = 0);
std::vector<u1> readn(std::ifstream &file, size_t n);


struct cp_info_class_info { u2 name_index; };
struct cp_info_string_info { u2 string_index; };
struct cp_info_short_info { u4 bytes; };
struct cp_info_long_info { u4 low_bytes; u4 high_bytes; };
struct cp_info_name_and_type_info { u2 name_index; u2 descriptor_index; };
struct cp_info_utf8_info { u2 length; char * bytes; };
struct cp_info_method_handle_info { u1 reference_kind; u2 reference_index; };
struct cp_info_method_type_info { u2 descriptor_index; };
struct cp_info_invoke_dynamic_info { u2 bootstrap_method_attr_index; u2 name_and_type_index; };
struct cp_info_ref_info { u2 class_index; u2 name_and_type_index; };


#define o1(T, AT1, AN1) T make_##T(AT1 AN1);
#define o2(T, AT1, AN1, AT2, AN2) T make_##T(AT1 AN1, AT2 AN2);
o1(cp_info_class_info, u2, name_index)
o1(cp_info_string_info, u2, string_index)
o1(cp_info_short_info, u4, bytes)
o1(cp_info_method_type_info, u2, descriptor_index)
o2(cp_info_long_info, u4, low_bytes, u4, high_bytes)
o2(cp_info_name_and_type_info, u2, name_index, u2, descriptor_index)
o2(cp_info_utf8_info, u2, length, char *, bytes)
o2(cp_info_method_handle_info, u1, reference_kind, u2, reference_index)
o2(cp_info_invoke_dynamic_info, u2, bootstrap_method_attr_index, u2, name_and_type_index)
o2(cp_info_ref_info, u2, class_index, u2, name_and_type_index)
#undef o1
#undef o2

typedef std::variant<cp_info_class_info,
        cp_info_string_info,
        cp_info_short_info,
        cp_info_long_info,
        cp_info_name_and_type_info,
        cp_info_utf8_info,
        cp_info_method_handle_info,
        cp_info_method_type_info,
        cp_info_invoke_dynamic_info,
        cp_info_ref_info> cp_info_info;
struct cp_info {
    u1 tag;
    const char *tag_description;
    cp_info_info info;
};
cp_info make_cp_info(u1 tag, const char *tag_description, cp_info_info info);
cp_info parse_cp_info(std::ifstream &file);

struct attribute_info {
    u2 attribute_name_index;
    std::vector<u1> info;
};
struct field_info {
    u2 access_flags;
    u2 name_index;
    u2 descriptor_index;
    std::vector<attribute_info> attributes;
};
struct method_info {
    u2 access_flags;
    u2 name_index;
    u2 descriptor_index;
    std::vector<attribute_info> attributes;
};
attribute_info parse_attribute_info(std::ifstream &file);
field_info parse_field(std::ifstream &file);
method_info parse_method(std::ifstream &file);

class ClassFileParser {
public:
    u4 magic;
    u2 minor;
    u2 major;
    u2 access_flags;
    u2 this_class;
    u2 super_class;
    std::vector<cp_info> constant_pool;
    std::vector<u2> interfaces;
    std::vector<field_info> fields;
    std::vector<method_info> methods;
    std::vector<attribute_info> attributes;

    explicit ClassFileParser(const char *filePath);
    ~ClassFileParser() = default;
};


#endif //SPEED_CLASSFILEPARSER_H
