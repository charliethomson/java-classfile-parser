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

    Field(u2 flags, std::string name, descriptor *desc, std::vector<attribute_t> attributes) : m_flags(flags), m_descriptor(desc), m_attributes(attributes) {
        m_name = name;
    }

};
class MethodDescriptor {
public:
    std::string m_raw;
    std::vector<FieldDescriptor> m_paramDescriptors;
    FieldDescriptor m_returnDescriptor;

    MethodDescriptor(std::string raw): m_paramDescriptors() {
        m_raw = raw;

        auto firstChar = raw.at(0);
        if (!firstChar == '(') {
            std::cerr << "Failed to parse MethodDescriptor: " << raw << " is not a valid method descriptor (missing opening parenthesis" << std::endl;
            throw std::exception();
        }

        size_t paramsEnd = raw.find_last_of(')');
        auto paramDescriptors = std::string(raw.begin() + 1, raw.begin() + paramsEnd);

        auto paramOffset = 0;
        while (paramOffset < paramDescriptors.size()) {
            auto descriptor = parse_descriptor(std::move(paramDescriptors.substr(paramOffset)));
            paramOffset += descriptor->offset;
            m_paramDescriptors.emplace_back(FieldDescriptor(descriptor));
        }

        auto returnDescriptor = parse_descriptor(std::move(raw.substr(paramsEnd+1)));
        m_returnDescriptor = FieldDescriptor(returnDescriptor);
    }
};
class Method {
public:
    AccessFlags m_flags;
    std::string m_name;
    MethodDescriptor m_descriptor;
    std::vector<attribute_t> m_attributes;

    Method(u2 flags, std::string name, std::string descriptor, std::vector<attribute_t> attributes) : m_flags(flags), m_descriptor(descriptor), m_attributes(attributes) {
        m_name = name;
    }
};
class JavaClass {
private:
    std::vector<ConstantInfo> m_constantsPool;

    ConstantInfo getConstant(size_t index) {
        return m_constantsPool.at(index-1);
    }

    std::string getClassInfoString(size_t classInfoIndex) {
        auto classInfo = getConstant(classInfoIndex).expectClassInfo();
        return getUtf8String(classInfo.name_index);
    }
    std::string getUtf8String(size_t utf8Index) {
        return std::string(getConstant(utf8Index).expectUtf8Info().bytes);
    }

    std::vector<std::string> populateInterfaces(std::vector<u2> indices) {
        std::vector<std::string> interfaces;
        for (u2 index : indices)
            interfaces.emplace_back(getClassInfoString(index));
        return interfaces;
    }

    std::vector<attribute_t> populateAttributes(std::vector<attribute_info> attrs) {
        std::vector<attribute_t> attributes;
        for (auto attr : attrs) {
            auto name = getUtf8String(attr.attribute_name_index);
            auto attribute = make_attribute(name, attr.info, m_constantsPool);
            auto isCode = isCodeAttribute(name);
            attributes.emplace_back(attribute);
        }
        return attributes;
    }

    std::vector<Field> populateFields(std::vector<field_info> info) {
        std::vector<Field> fields;
        for (auto fieldInfo : info) {
            auto name = getUtf8String(fieldInfo.name_index);
            auto descriptor = parse_descriptor(getUtf8String(fieldInfo.descriptor_index));
            auto attributes = populateAttributes(fieldInfo.attributes);

            fields.emplace_back(Field(fieldInfo.access_flags, name, descriptor, attributes));
        }
        return fields;
    }

    std::vector<Method> populateMethods(std::vector<method_info> info) {
        std::vector<Method> methods;

        for (auto methodInfo : info) {
            auto name = getUtf8String(methodInfo.name_index);
            auto descriptor = getUtf8String(methodInfo.descriptor_index);
            auto attributes = populateAttributes(methodInfo.attributes);

            methods.emplace_back(Method(methodInfo.access_flags, name, descriptor, attributes));
        }

        return methods;
    }
public:
    java_version m_version;
    AccessFlags m_flags;
    std::string m_thisClass;
    std::optional<std::string> m_superClass;
    std::vector<std::string> m_interfaces;
    std::vector<Field> m_fields;
    std::vector<Method> m_methods;
    std::vector<attribute_t> m_attributes;

    JavaClass(ClassFileParser parser);
    JavaClass(const char * filePath);

    ~JavaClass();
};


#endif //SPEED_JAVACLASS_H
