//
// Created by charlie.thomson on 10/19/2022.
//

#include "JavaClass.h"
#include "AccessFlags.h"


JavaClass::JavaClass(ClassFileParser parser) : m_constantsPool(), m_flags(parser.access_flags) {
    for (auto info : parser.constant_pool)
        m_constantsPool.emplace_back(ConstantInfo(info.info, info.tag_description));

    m_version = {
            .minor=parser.minor,
            .major=parser.major,
    };
    m_thisClass = getClassInfoString(parser.this_class);
    m_superClass = parser.super_class
            ? std::optional(getClassInfoString(parser.super_class))
            : std::nullopt;

    m_interfaces = populateInterfaces(parser.interfaces);
    m_fields = populateFields(parser.fields);
    m_methods = populateMethods(parser.methods);
    m_attributes = populateAttributes(parser.attributes);
}
//JavaClass::JavaClass(const char * filePath) {
//
//}

JavaClass::~JavaClass() {}

