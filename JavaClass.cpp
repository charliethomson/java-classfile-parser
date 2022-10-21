#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
//
// Created by charlie.thomson on 10/19/2022.
//

#include "JavaClass.h"
#include "AccessFlags.h"
MethodDescriptor::MethodDescriptor(std::string raw): m_paramDescriptors() {
    m_raw = raw;

    auto firstChar = raw.at(0);
    if (firstChar != '(') {
        std::cerr << "Failed to parse MethodDescriptor: " << raw << " is not a valid method descriptor (missing opening parenthesis" << std::endl;
        throw std::exception();
    }

    auto paramsEnd = raw.find_last_of(')');
    auto paramDescriptors = std::string(raw.begin() + 1, raw.begin() + paramsEnd);

    auto paramOffset = 0;
    while (paramOffset < paramDescriptors.size()) {
    auto descriptor = parse_descriptor(std::move(paramDescriptors.substr(paramOffset)));
    paramOffset += static_cast<int>(descriptor->offset);
    m_paramDescriptors.emplace_back(FieldDescriptor(descriptor));
    }

    auto returnDescriptor = parse_descriptor(std::move(raw.substr(paramsEnd+1)));
    m_returnDescriptor = FieldDescriptor(returnDescriptor);
}
JavaClass::JavaClass(const ClassFileParser& parser) : m_constantsPool(), m_flags(parser.access_flags), m_version() {
    for (auto info : parser.constant_pool) {
        auto constantInfo = ConstantInfo(info.info, info.tag_description);
        m_constantsPool.emplace_back(constantInfo);
        // Duplicate the long entry because some fuck at Sun 40 years ago decided longs should
        // take up two pool entries, which affects literally every fucking lookup. I hate it here
        if (constantInfo.isLongInfo())
            m_constantsPool.emplace_back(constantInfo);
    }
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

JavaClass::~JavaClass() = default;

ConstantInfo JavaClass::getConstant(size_t index) {
    return m_constantsPool.at(index-1);
}

std::string JavaClass::getClassInfoString(size_t classInfoIndex) {
    auto classInfo = getConstant(classInfoIndex).expectClassInfo();
    return getUtf8String(classInfo.name_index);
}

std::string JavaClass::getUtf8String(size_t utf8Index) {
    return { getConstant(utf8Index).expectUtf8Info().bytes };
}

std::vector<std::string> JavaClass::populateInterfaces(const std::vector<u2>& indices) {
    std::vector<std::string> interfaces;
    for (u2 index : indices)
        interfaces.emplace_back(getClassInfoString(index));
    return interfaces;
}

std::vector<attribute_t> JavaClass::populateAttributes(const std::vector<attribute_info>& attrs) {
    std::vector<attribute_t> attributes;
    for (const auto& attr : attrs) {
        auto name = getUtf8String(attr.attribute_name_index);
        auto attribute = make_attribute(name, attr.info, m_constantsPool);
        attributes.emplace_back(attribute);
    }
    return attributes;
}

std::vector<Field> JavaClass::populateFields(const std::vector<field_info>& info) {
    std::vector<Field> fields;
    for (const auto& fieldInfo : info) {
        auto name = getUtf8String(fieldInfo.name_index);
        auto descriptor = parse_descriptor(getUtf8String(fieldInfo.descriptor_index));
        auto attributes = populateAttributes(fieldInfo.attributes);

        fields.emplace_back(Field(fieldInfo.access_flags, name, descriptor, attributes));
    }
    return fields;
}

std::vector<Method> JavaClass::populateMethods(const std::vector<method_info>& info) {
    std::vector<Method> methods;

    for (const auto& methodInfo : info) {
        auto name = getUtf8String(methodInfo.name_index);
        auto descriptor = getUtf8String(methodInfo.descriptor_index);
        auto attributes = populateAttributes(methodInfo.attributes);

        methods.emplace_back(Method(methodInfo.access_flags, name, descriptor, attributes));
    }

    return methods;
}

Method::Method(u2 flags, std::string name, std::string descriptor, std::vector<attribute_t> attributes) : m_flags(flags), m_descriptor(std::move(descriptor)), m_attributes(std::move(attributes)) {
    m_name = std::move(name);
}

Field::Field(u2 flags, std::string name, descriptor *desc, std::vector<attribute_t> attributes) : m_flags(flags), m_descriptor(desc), m_attributes(std::move(attributes)) {
    m_name = std::move(name);
}

#pragma clang diagnostic pop