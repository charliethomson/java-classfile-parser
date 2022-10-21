#include "ClassFileParser.h"

#include <cstdint>
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <variant>

typedef uint32_t u4;
typedef uint16_t u2;
typedef uint8_t u1;

template <class T>
void swap_endian(T *t)
{
    auto *memp = reinterpret_cast<unsigned char*>(t);
    std::reverse(memp, memp + sizeof(T));
}

template <class T>
T read_un(std::ifstream &file, std::streamsize n) {
    T v = 0;
    file.read(reinterpret_cast<char*>(&v), n);
    swap_endian(&v);
    return v;
}

#define o(T,S) T read_##T(std::ifstream &file) { return read_un<T>(file, S); }
o(u4,4)
o(u2,2)
o(u1,1)
#undef o

std::vector<u1> readn(std::ifstream &file, size_t n) {
    std::vector<u1> v;
    for (auto i = 0; i < n; ++i)
        v.emplace_back(read_u1(file));

    return v;
}

template <class T>
std::vector<T> read_array(std::ifstream &file, std::function<T(std::ifstream &file)> parser, size_t offset) {
    auto count = read_u2(file);

    std::vector<T> items;
    for (size_t i = 0; i < count-offset; ++i) {
        try {
            auto item = parser(file);
            items.emplace_back(item);
        } catch (const std::exception &e) {
            break;
        }
    }
    return items;
}

#define o1(T, AT1, AN1) T make_##T(AT1 AN1) { return { .AN1=AN1 }; }
#define o2(T, AT1, AN1, AT2, AN2) T make_##T(AT1 AN1, AT2 AN2) { return { .AN1=AN1, .AN2=AN2 }; }
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

cp_info make_cp_info(u1 tag, const char *tag_description, cp_info_info info) {
    return {
            .tag=tag,
            .tag_description=tag_description,
            .info=info,
    };
}

cp_info parse_cp_info(std::ifstream &file) {
    auto tag = read_u1(file);
    u1 reference_kind;

    u2 name_index;
    u2 string_index;
    u2 descriptor_index;
    u2 reference_index;
    u2 bootstrap_method_attr_index;
    u2 name_and_type_index;
    u2 class_index;

    u4 short_bytes;
    u4 low_bytes;
    u4 high_bytes;

    u2 utf8_length;
    char *utf8_bytes;

    switch (tag) {
        case 7:
            name_index = read_u2(file);
            return make_cp_info(tag, "CONSTANT_Class",make_cp_info_class_info(name_index));
        case 9:
            class_index = read_u2(file);
            name_and_type_index = read_u2(file);
            return make_cp_info(tag, "CONSTANT_Fieldref",make_cp_info_ref_info(class_index, name_and_type_index));
        case 10:
            class_index = read_u2(file);
            name_and_type_index = read_u2(file);
            return make_cp_info(tag, "CONSTANT_Methodref",make_cp_info_ref_info(class_index, name_and_type_index));
        case 11:
            class_index = read_u2(file);
            name_and_type_index = read_u2(file);
            return make_cp_info(tag, "CONSTANT_InterfaceMethodref",make_cp_info_ref_info(class_index, name_and_type_index));
        case 8:
            string_index = read_u2(file);
            return make_cp_info(tag, "CONSTANT_String", make_cp_info_string_info(string_index));
        case 3:
            short_bytes = read_u4(file);
            return make_cp_info(tag, "CONSTANT_Integer", make_cp_info_short_info(short_bytes));
        case 4:
            short_bytes = read_u4(file);
            return make_cp_info(tag, "CONSTANT_Float", make_cp_info_short_info(short_bytes));
        case 5:
            high_bytes = read_u4(file);
            low_bytes  = read_u4(file);
            return make_cp_info(tag, "CONSTANT_Long",make_cp_info_long_info(low_bytes, high_bytes));
        case 6:
            high_bytes = read_u4(file);
            low_bytes  = read_u4(file);
            return make_cp_info(tag, "CONSTANT_Double",make_cp_info_long_info(low_bytes, high_bytes));
        case 12:
            name_index = read_u2(file);
            descriptor_index = read_u2(file);
            return make_cp_info(tag, "CONSTANT_NameAndType", make_cp_info_name_and_type_info(name_index, descriptor_index));
        case 1:
            utf8_length = read_u2(file);
            utf8_bytes = (char*)malloc(utf8_length);
            file.read(utf8_bytes, utf8_length);
            utf8_bytes[utf8_length] = '\0';
            return make_cp_info(tag, "CONSTANT_Utf8", make_cp_info_utf8_info(utf8_length, utf8_bytes));
        case 15:
            reference_kind = read_u1(file);
            reference_index = read_u2(file);
            return make_cp_info(tag, "CONSTANT_MethodHandle",make_cp_info_method_handle_info(reference_kind, reference_index));
        case 16:
            descriptor_index = read_u2(file);
            return make_cp_info(tag, "CONSTANT_MethodType",make_cp_info_method_type_info(descriptor_index));
        case 18:
            bootstrap_method_attr_index = read_u2(file);
            name_and_type_index = read_u2(file);
            return make_cp_info(tag, "CONSTANT_InvokeDynamic", make_cp_info_invoke_dynamic_info(bootstrap_method_attr_index, name_and_type_index));
        default:
            std::cerr << "Unknown tag: " << static_cast<int>(tag) << std::endl;
            file.unget();
            throw std::exception();
    }
}
attribute_info parse_attribute_info(std::ifstream &file) {
    auto attribute_name_index = read_u2(file);
    auto length = read_u4(file);
    std::vector<u1> info = readn(file, length);

    return {
            .attribute_name_index=attribute_name_index,
            .info=info,
    };
}
field_info parse_field(std::ifstream &file) {
    auto access_flags = read_u2(file);
    auto name_index = read_u2(file);
    auto descriptor_index = read_u2(file);

    std::vector<attribute_info> attributes = read_array<attribute_info>(file, parse_attribute_info);

    return {
            .access_flags = access_flags,
            .name_index = name_index,
            .descriptor_index = descriptor_index,
            .attributes = attributes,
    };
}
method_info parse_method(std::ifstream &file) {
    auto access_flags = read_u2(file);
    auto name_index = read_u2(file);
    auto descriptor_index = read_u2(file);

    std::vector<attribute_info> attributes = read_array<attribute_info>(file, parse_attribute_info);

    return {
            .access_flags = access_flags,
            .name_index = name_index,
            .descriptor_index = descriptor_index,
            .attributes = attributes,
    };
}


std::string dump_cp_info_name(cp_info &info) {
auto s = std::string(info.tag_description);
auto offset = s.find('_');
return s.substr(offset);
}
std::string dump_cp_info_args(cp_info &info) {
#define o(T,A) if (holds_alternative<T>(info.info)) { \
    auto i = get<T>(info.info);                         \
    auto ss = std::stringstream();                      \
    A;                                                  \
    return ss.str();                                    \
}
#define o1(T, N) o(T, ss << #N << "=" << i.N;)
#define o2(T, N1, N2) o(T, ss << #N1 << "=" << i.N1 << ",\t" << #N2 << "=" << i.N2;)
o1(cp_info_class_info, name_index)
o1(cp_info_string_info, string_index)
o1(cp_info_short_info, bytes)
o1(cp_info_method_type_info, descriptor_index)
o2(cp_info_ref_info, class_index, name_and_type_index)
o2(cp_info_long_info, low_bytes, high_bytes)
o2(cp_info_name_and_type_info, name_index, descriptor_index)
o2(cp_info_utf8_info, length, bytes)
o2(cp_info_method_handle_info, reference_kind, reference_index)
o2(cp_info_invoke_dynamic_info, bootstrap_method_attr_index, name_and_type_index)
#undef o
#undef o1
#undef o2
return "UNKNOWN";
}

std::string dump_cp_info(cp_info info) {
std::string typestr = dump_cp_info_name(info);
std::string argsstr = dump_cp_info_args(info);

auto ss = std::stringstream();
ss << typestr << "\t" << argsstr << std::endl;
return ss.str();
}

ClassFileParser::ClassFileParser(const char *filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (file.fail()) {
        std::cerr << "Error opening file?" << std::endl;
        return;
    }

#define o(N, S) N = read_##S(file); printf("%s=0x%X\n",#N,N);
    o(magic, u4)
    if (magic != 0xCAFEBABE) {
        std::cerr << "This is not a class file :/" << std::endl;
        throw;
    }
    o(minor, u2)
    o(major, u2)
    constant_pool = read_array<cp_info>(file, parse_cp_info, 1);
    std::cout << "Got " << constant_pool.size() << " constants: " << std::endl;
    for (int i = 0; i < constant_pool.size(); ++i) {
        std::cout << "\t#" << i << dump_cp_info(constant_pool.at(i));
    }
    o(access_flags, u2)
    o(this_class, u2)
    o(super_class, u2)
#undef o

    interfaces = read_array<u2>(file, [](std::ifstream &file) -> u2 { return read_u2(file); });
    std::cout << "Got " << interfaces.size() << " interfaces" << std::endl;

#define o(N, T, P) N = read_array<T>(file, P); std::cout << "Got " << N.size() << " " << #N << std::endl;
    o(fields, field_info, parse_field)
    o(methods, method_info, parse_method)
    o(attributes, attribute_info, parse_attribute_info)
#undef o
}
