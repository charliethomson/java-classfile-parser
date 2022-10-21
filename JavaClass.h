//
// Created by charlie.thomson on 10/19/2022.
//
#pragma once
#ifndef SPEED_JAVACLASS_H
#define SPEED_JAVACLASS_H
#include "ClassFileParser.h"
#include "ConstantInfo.h"
#include "AccessFlags.h"
#include "Attribute.h"
#include "FieldDescriptor.h"

#include <utility>
#include <utility>
#include <vector>
#include <string>
#include <optional>
#include <memory>

struct java_version {
    u2 minor;
    u2 major;
};

class Field {
public:
    AccessFlags m_flags;
    std::string m_name;
    FieldDescriptor m_descriptor;
    std::vector<attribute_t> m_attributes;

    Field(u2 flags, std::string name, descriptor *desc, std::vector<attribute_t> attributes);

};
class MethodDescriptor {
public:
    std::string m_raw;
    std::vector<FieldDescriptor> m_paramDescriptors;
    FieldDescriptor m_returnDescriptor;
    explicit MethodDescriptor(std::string raw);
};
class Method {
public:
    AccessFlags m_flags;
    std::string m_name;
    MethodDescriptor m_descriptor;
    std::vector<attribute_t> m_attributes;
    Method(u2 flags, std::string name, std::string descriptor, std::vector<attribute_t> attributes);
};
class JavaClass {
private:
    std::vector<ConstantInfo> m_constantsPool;
    ConstantInfo getConstant(size_t index);
    std::string getClassInfoString(size_t classInfoIndex);
    std::string getUtf8String(size_t utf8Index);
    std::vector<std::string> populateInterfaces(const std::vector<u2>& indices);
    std::vector<attribute_t> populateAttributes(const std::vector<attribute_info>& attrs);
    std::vector<Field> populateFields(const std::vector<field_info>& info);
    std::vector<Method> populateMethods(const std::vector<method_info>& info);
public:
    java_version m_version{};
    AccessFlags m_flags;
    std::string m_thisClass;
    std::optional<std::string> m_superClass;
    std::vector<std::string> m_interfaces;
    std::vector<Field> m_fields;
    std::vector<Method> m_methods;
    std::vector<attribute_t> m_attributes;

    explicit JavaClass(const ClassFileParser& parser);
    ~JavaClass();
};


#endif //SPEED_JAVACLASS_H
