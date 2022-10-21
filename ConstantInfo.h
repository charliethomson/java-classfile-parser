//
// Created by charlie.thomson on 10/20/2022.
//

#ifndef SPEED_CONSTANTINFO_H
#define SPEED_CONSTANTINFO_H

#include "ClassFileParser.h"
#include <optional>
#include <variant>

class ConstantInfo {
private:
    cp_info_info m_inner;
public:
    std::string m_type;
    explicit ConstantInfo(cp_info_info inner, std::string type);

#define o(A,B)                                                         \
cp_info_##A expect##B();                                               \
bool is##B();                                                          \
std::optional<cp_info_##A> maybe##B();
    o(class_info,          ClassInfo)
    o(string_info,         StringInfo)
    o(short_info,          ShortInfo)
    o(method_type_info,    MethodTypeInfo)
    o(long_info,           LongInfo)
    o(name_and_type_info,  NameAndTypeInfo)
    o(utf8_info,           Utf8Info)
    o(method_handle_info,  MethodHandleInfo)
    o(invoke_dynamic_info, InvokeDynamicInfo)
    o(ref_info,            RefInfo)

#undef o
};

#endif //SPEED_CONSTANTINFO_H
