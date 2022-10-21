//
// Created by charlie.thomson on 10/20/2022.
//

#include "ConstantInfo.h"

ConstantInfo::ConstantInfo(cp_info_info inner, std::string type) {
    m_inner = inner;
    m_type = std::move(type);
}


#define o(A,B)                                                         \
cp_info_##A ConstantInfo::expect##B() {                                \
    return std::get<cp_info_##A>(m_inner);                             \
};                                                                     \
bool ConstantInfo::is##B() {                                           \
    return std::holds_alternative<cp_info_##A>(m_inner);               \
};                                                                     \
std::optional<cp_info_##A> ConstantInfo::maybe##B() {                  \
    auto v = std::get_if<cp_info_##A>(&m_inner);                       \
    return v ? std::optional(*v) : std::nullopt;                       \
};
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