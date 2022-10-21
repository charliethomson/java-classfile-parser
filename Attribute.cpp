#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
//
// Created by charlie.thomson on 10/19/2022.


#include "Attribute.h"

#include <utility>

template<typename T>
T read_vec_un(std::vector<u1> bytes, size_t &cursor, size_t n) {
    T v=0;
    for (auto i = 0; i < n; ++i) {
        v <<= 8;
        v |= bytes.at((cursor) + i);
    }
    cursor += n;
    return v;
}
#define o(T,N) static T read_vec_##T(std::vector<u1> bytes, size_t &cursor) { \
    return read_vec_un<T>(std::move(bytes), cursor, N);                                  \
}
o(u1, 1)
o(u2, 2)
o(u4, 4)
#undef o

attribute_t make_attribute(std::string name, const std::vector<u1>& bytes, std::vector<ConstantInfo> &constantsPool) {
#define o(A) if (is##A##Attribute(name)) return make##A##Attribute(name, bytes, constantsPool);
    o(ConstantValue)
    o(Code)
    o(StackMapTable)
    o(Exceptions)
    o(InnerClasses)
    o(EnclosingMethod)
    o(Synthetic)
    o(Signature)
    o(SourceFile)
    o(SourceDebugExtension)
    o(LineNumberTable)
    o(LocalVariableTable)
    o(LocalVariableTypeTable)
    o(Deprecated)
    o(RuntimeVisibleAnnotations)
    o(RuntimeInvisibleAnnotations)
    o(RuntimeVisibleParameterAnnotations)
    o(RuntimeInvisibleParameterAnnotations)
    o(AnnotationDefault)
    o(BootstrapMethods)
#undef o
    std::cerr << "No handler configured for attr of type " << name << std::endl;
    throw std::exception();
}

attribute_t makeConstantValueAttribute(const std::string& name, const std::vector<u1> &bytes, std::vector<ConstantInfo> &constantsPool) {
    size_t cursor = 0;
    auto constantValueIndex = read_vec_u2(bytes, cursor);
    auto constant = constantsPool.at(constantValueIndex - 1);
    if (constant.isLongInfo()) {
        auto constantInfo = constant.expectLongInfo();
        uint64_t constantValue = constantInfo.low_bytes | (static_cast<uint64_t>(constantInfo.high_bytes) << 32);

        if (constant.m_type == "CONSTANT_Long")
            return ConstantValueAttributeLong(name, constantValue);
        return ConstantValueAttributeDouble(name, static_cast<double>(constantValue));
    }
    if (constant.isShortInfo()) {
        auto constantInfo = constant.expectShortInfo();
        uint32_t constantValue = constantInfo.bytes;

        if (constant.m_type == "CONSTANT_Integer")
            return ConstantValueAttributeInteger(name, constantValue);
        return ConstantValueAttributeFloat(name, static_cast<float>(constantValue));
    }
    if (constant.isStringInfo()) {
        auto constantInfo = constant.expectStringInfo();
        auto stringInfo = constantsPool.at(constantInfo.string_index - 1);
        std::string constantValue = std::string(stringInfo.expectUtf8Info().bytes);

        return ConstantValueAttributeString(name, constantValue);
    }

    std::cerr <<  "Invalid ConstantValue Attribute" << std::endl;
    throw std::exception();
}

attribute_t makeCodeAttribute(const std::string& name, const std::vector<u1> &bytes, std::vector<ConstantInfo> &constantsPool) {
    size_t cursor = 0;
#define r_u4(N) auto N = read_vec_u4(bytes, cursor);
#define r_u2(N) auto N = read_vec_u2(bytes, cursor);
    r_u2(maxStack)
    r_u2(maxLocals)
    r_u4(codeLength)
    std::vector<u1> code;
    for (size_t i = 0; i < codeLength; ++i)
        code.emplace_back(read_vec_u1(bytes, cursor));
    r_u2(exceptionTableLength)
    std::vector<ExceptionTableEntry> exceptionTable;
    for (size_t i = 0; i < exceptionTableLength; ++i) {
        r_u2(startPc)
        r_u2(endPc)
        r_u2(handlerPc)
        r_u2(catchType)

        exceptionTable.emplace_back(ExceptionTableEntry(startPc, endPc, handlerPc, catchType));
    }
    r_u2(attrCount)
    std::vector<attribute_t> attributes;
    for (size_t i = 0; i < attrCount; ++i) {
        r_u2(nameIndex)
        r_u4(attrLength)

        std::vector<u1> attributeData;
        for (size_t j = 0; j < attrLength; ++j)
            attributeData.emplace_back(read_vec_u1(bytes, cursor));

        auto attributeName = std::string(constantsPool.at(nameIndex-1).expectUtf8Info().bytes);
        auto attribute = make_attribute(attributeName, attributeData, constantsPool);

        attributes.emplace_back(attribute);
    }

#undef r_u4
#undef r_u2
    return CodeAttribute(name, maxStack, maxLocals, code, exceptionTable, attributes);
}

verification_type_info_t readVerificationType(const std::vector<u1> &bytes, size_t &cursor, std::vector<ConstantInfo> &constantsPool) {
    auto tag = read_vec_u1(bytes, cursor);
    if (tag == VerificationTypeInfoTop::Tag()) return VerificationTypeInfoTop();
    if (tag == VerificationTypeInfoInteger::Tag()) return VerificationTypeInfoInteger();
    if (tag == VerificationTypeInfoFloat::Tag()) return VerificationTypeInfoFloat();
    if (tag == VerificationTypeInfoDouble::Tag()) return VerificationTypeInfoDouble();
    if (tag == VerificationTypeInfoLong::Tag()) return VerificationTypeInfoLong();
    if (tag == VerificationTypeInfoNull::Tag()) return VerificationTypeInfoNull();
    if (tag == VerificationTypeInfoUninitializedThis::Tag()) return VerificationTypeInfoUninitializedThis();
    if (tag == VerificationTypeInfoObject::Tag()) {
        auto classInfoIndex = read_vec_u2(bytes, cursor);
        auto classNameIndex = constantsPool.at(classInfoIndex - 1).expectClassInfo().name_index;
        auto className = std::string(constantsPool.at(classNameIndex - 1).expectUtf8Info().bytes);
        return VerificationTypeInfoObject(className);
    }
    if (tag == VerificationTypeInfoUninitializedVariable::Tag()) {
        auto offset = read_vec_u2(bytes, cursor);
        return VerificationTypeInfoUninitializedVariable(offset);
    }

    std::cerr << "Unknown verification tag " << tag << std::endl;
    throw std::exception();
}

attribute_t makeStackMapTableAttribute(const std::string& name, const std::vector<u1> &bytes, std::vector<ConstantInfo> &constantsPool) {
    size_t cursor = 0;

#define r_u2(N) auto N = read_vec_u2(bytes, cursor);
#define r_u1(N) auto N = read_vec_u1(bytes, cursor);

    r_u2(entryCount)
    std::vector<stack_map_frame_t> stackFrames;
    for (size_t i = 0; i < entryCount; ++i) {
        r_u1(frameType)
        if (frameType <= 63) { // same_frame
            stackFrames.emplace_back(StackMapFrameSame(frameType));
            continue;
        } else if (frameType <= 127) { // same_locals_1_stack_item_frame
            auto verificationType = readVerificationType(bytes, cursor, constantsPool);
            stackFrames.emplace_back(StackMapFrameSameLocals(frameType, std::vector<verification_type_info_t>({ verificationType })));
            continue;
        } else if (frameType <= 246) {
            // Reserved
        } else if (frameType == 247) {
            r_u2(offsetDelta)
            auto verificationType = readVerificationType(bytes, cursor, constantsPool);
            stackFrames.emplace_back(StackMapFrameSameLocalsExtended(frameType, offsetDelta, std::vector<verification_type_info_t>({ verificationType })));
        } else if (frameType <= 250) {
            r_u2(offsetDelta)
            stackFrames.emplace_back(StackMapFrameChop(frameType, offsetDelta));
        } else if (frameType == 251) {
            r_u2(offsetDelta)
            stackFrames.emplace_back(StackMapFrameExtended(frameType, offsetDelta));
        } else if (frameType <= 254) {
            r_u2(offsetDelta)
            std::vector<verification_type_info_t> locals;
            for (size_t j = 0; j < frameType - 251; ++j) {
                locals.emplace_back(readVerificationType(bytes, cursor, constantsPool));
            }
            stackFrames.emplace_back(StackMapFrameAppend(frameType, offsetDelta, locals));
        } else {
            r_u2(offsetDelta)
            r_u2(localCount)
            std::vector<verification_type_info_t> locals;
            for (size_t j = 0; j < localCount; ++j)
                locals.emplace_back(readVerificationType(bytes, cursor, constantsPool));
            r_u2(stackCount)
            std::vector<verification_type_info_t> stack;
            for (size_t j = 0; j < stackCount; ++j)
                stack.emplace_back(readVerificationType(bytes, cursor, constantsPool));
            stackFrames.emplace_back(StackMapFrameFull(frameType, offsetDelta, locals, stack));
        }
    }

    return StackMapTableAttribute(name, stackFrames);
#undef r_u2
#undef r_u1
}

attribute_t makeExceptionsAttribute(const std::string& name, const std::vector<u1> &bytes, std::vector<ConstantInfo> &constantsPool) {
    return Attribute("UNIMPLMENTED");
}

attribute_t makeInnerClassesAttribute(const std::string& name, const std::vector<u1> &bytes, std::vector<ConstantInfo> &constantsPool) {
    return Attribute("UNIMPLMENTED");
}

attribute_t makeEnclosingMethodAttribute(const std::string& name, const std::vector<u1> &bytes, std::vector<ConstantInfo> &constantsPool) {
    return Attribute("UNIMPLMENTED");
}

attribute_t makeSyntheticAttribute(const std::string& name, const std::vector<u1> &bytes, std::vector<ConstantInfo> &constantsPool) {
    return Attribute("UNIMPLMENTED");
}

attribute_t makeSignatureAttribute(const std::string& name, const std::vector<u1> &bytes, std::vector<ConstantInfo> &constantsPool) {
    return Attribute("UNIMPLMENTED");
}

attribute_t makeSourceFileAttribute(const std::string& name, const std::vector<u1> &bytes, std::vector<ConstantInfo> &constantsPool) {
    size_t cursor = 0;
    auto sourceFileIndex = read_vec_u2(bytes, cursor);
    auto sourceFileName = std::string(constantsPool.at(sourceFileIndex - 1).expectUtf8Info().bytes);

    return SourceFileAttribute(name, sourceFileName);
}

attribute_t makeSourceDebugExtensionAttribute(const std::string& name, const std::vector<u1> &bytes, std::vector<ConstantInfo> &constantsPool) {
    return Attribute("UNIMPLMENTED");
}

attribute_t makeLineNumberTableAttribute(const std::string& name, const std::vector<u1> &bytes, std::vector<ConstantInfo> &constantsPool) {
    size_t cursor = 0;
    auto tableLength = read_vec_u2(bytes, cursor);
    std::vector<LineNumberEntry> table;
    for (size_t i = 0; i < tableLength; ++i) {
        auto startPc = read_vec_u2(bytes, cursor);
        auto lineNumber = read_vec_u2(bytes, cursor);
        table.emplace_back(LineNumberEntry(startPc, lineNumber));
    }

    return LineNumberTableAttribute(name, table);
 }

attribute_t makeLocalVariableTableAttribute(const std::string& name, const std::vector<u1> &bytes, std::vector<ConstantInfo> &constantsPool) {
    return Attribute("UNIMPLMENTED");
}

attribute_t makeLocalVariableTypeTableAttribute(const std::string& name, const std::vector<u1> &bytes, std::vector<ConstantInfo> &constantsPool) {
    return Attribute("UNIMPLMENTED");
}

attribute_t makeDeprecatedAttribute(const std::string& name, const std::vector<u1> &bytes, std::vector<ConstantInfo> &constantsPool) {
    return DeprecatedAttribute(name);
}

runtime_annotation_value_t readElementValue(const std::vector<u1>& bytes, size_t &cursor, std::vector<ConstantInfo> &constantsPool) {
    auto tag = read_vec_u1(bytes, cursor);
    if (std::string("BCDFIJSZs").find(static_cast<char>(tag)) != std::string::npos) {
        auto constValueIndex = read_vec_u2(bytes, cursor);
        auto constItem = constantsPool.at(constValueIndex-1);
        const_value_t constValue;

        // Shorts
        if (std::string("BCFISZ").find(static_cast<char>(tag)) != std::string::npos) {
            auto shortBytes = constItem.expectShortInfo().bytes;
            // Reinterpret cast doesn't let me do this, but it's correct in this _one_ case :d
            if (tag == 'F') constValue = *((float *)(&shortBytes));
            else constValue = reinterpret_cast<uint32_t>(shortBytes);
        }
        // Longs
        else if (std::string("DJ").find(static_cast<char>(tag)) != std::string::npos) {
            auto longInfo = constItem.expectLongInfo();
            uint64_t lng = longInfo.low_bytes | (static_cast<uint64_t>(longInfo.high_bytes) << 32);
            if (tag == 'D') constValue = *((double *)(&lng));
            else constValue = reinterpret_cast<uint64_t>(lng);
        }
        // String
        else if (tag == 's') {
            constValue = std::string(constItem.expectUtf8Info().bytes);
        }

        return ConstValueRuntimeAnnotationValue(tag, constValue);
    }
    else if (tag == 'e') {
        auto typeNameIndex = read_vec_u2(bytes, cursor);
        auto constNameIndex = read_vec_u2(bytes, cursor);
        auto typeName = std::string(constantsPool.at(typeNameIndex - 1).expectUtf8Info().bytes);
        auto constName = std::string(constantsPool.at(constNameIndex - 1).expectUtf8Info().bytes);
        return EnumConstValueRuntimeAnnotationValue(tag, typeName, constName);
    }
    else if (tag == 'c') {
        auto classInfoIndex = read_vec_u2(bytes, cursor);
        auto returnDescriptorRaw = std::string(constantsPool.at(classInfoIndex - 1).expectUtf8Info().bytes);
        auto returnDescriptor = parse_descriptor(std::move(returnDescriptorRaw));
        auto returnFieldDescriptor = FieldDescriptor(returnDescriptor);
        return ClassInfoRuntimeAnnotationValue(tag, returnFieldDescriptor);
    }
    else if (tag == '@') {
        auto annotation = readRuntimeAnnotation(bytes, constantsPool, cursor);

        std::cerr << "Recursive annotations are not implemented." << std::endl;
        throw std::exception();
    }
    else if (tag == '[') {
        auto numValues = read_vec_u2(bytes, cursor);

        std::vector<runtime_annotation_value_t> elements;
        for (size_t j = 0; j < numValues; ++j) {
            auto element = readElementValue(bytes, cursor, constantsPool);
            elements.emplace_back(element);
        }

        return ArrayValueRuntimeAnnotationValue(tag, elements);
    }
    else {
        std::cerr << "Unknown annotation tag: " << tag << std::endl;
        throw std::exception();
    }
}

RuntimeAnnotation readRuntimeAnnotation(const std::vector<u1>& bytes, std::vector<ConstantInfo> &constantsPool, size_t &cursor) {
    auto typeIndex = read_vec_u2(bytes, cursor);
    auto numPairs = read_vec_u2(bytes, cursor);
    std::vector<RuntimeAnnotationsKeyValuePair> pairs;
    for (size_t i = 0; i < numPairs; ++i) {
        auto elementNameIndex = read_vec_u2(bytes, cursor);
        auto elementName = std::string(constantsPool.at(elementNameIndex - 1).expectUtf8Info().bytes);
        auto elementValue = readElementValue(bytes, cursor, constantsPool);
        pairs.emplace_back(RuntimeAnnotationsKeyValuePair(elementName, elementValue));
    }

    auto type = std::string(constantsPool.at(typeIndex-1).expectUtf8Info().bytes);
    return {type, pairs};
}

attribute_t makeRuntimeVisibleAnnotationsAttribute(const std::string& name, const std::vector<u1> &bytes, std::vector<ConstantInfo> &constantsPool) {
    size_t cursor = 0;
    if (bytes.empty())
        return RuntimeVisibleAnnotationsAttribute(name, std::vector<RuntimeAnnotation>());

    auto numAnnotations = read_vec_u2(bytes, cursor);
    std::vector<RuntimeAnnotation> annotations;
    for (int i = 0; i < numAnnotations; ++i)
        annotations.emplace_back(readRuntimeAnnotation(bytes, constantsPool, cursor));

    return RuntimeVisibleAnnotationsAttribute(name, annotations);
}


attribute_t makeRuntimeInvisibleAnnotationsAttribute(const std::string& name, const std::vector<u1> &bytes, std::vector<ConstantInfo> &constantsPool) {
    return Attribute("UNIMPLMENTED");
}

attribute_t makeRuntimeVisibleParameterAnnotationsAttribute(const std::string& name, const std::vector<u1> &bytes, std::vector<ConstantInfo> &constantsPool) {
    return Attribute("UNIMPLMENTED");
}

attribute_t makeRuntimeInvisibleParameterAnnotationsAttribute(const std::string& name, const std::vector<u1> &bytes, std::vector<ConstantInfo> &constantsPool) {
    return Attribute("UNIMPLMENTED");
}

attribute_t makeAnnotationDefaultAttribute(const std::string& name, const std::vector<u1> &bytes, std::vector<ConstantInfo> &constantsPool) {
    return Attribute("UNIMPLMENTED");
}

attribute_t makeBootstrapMethodsAttribute(const std::string& name, const std::vector<u1> &bytes, std::vector<ConstantInfo> &constantsPool) {
    return Attribute("UNIMPLMENTED");
}
Attribute::Attribute(std::string name) : m_name(std::move(name)) {}

ConstantValueAttribute::ConstantValueAttribute(std::string name) : Attribute(std::move(name)) {
    m_constant = nullptr;
}

// ConstantValue typed attributes
#define o(N, T) ConstantValueAttribute##N::ConstantValueAttribute##N(std::string name, T constant): ConstantValueAttribute(std::move(name)) { m_constant = constant; }
o(Long, uint64_t)
o(Float, float)
o(Double, double)
o(Integer, uint32_t)
#undef o
ConstantValueAttributeString::ConstantValueAttributeString(std::string name, std::string constant): ConstantValueAttribute(std::move(name)), m_constant(std::move(constant))  {}
// End: ConstantValue typed attributes

VerificationTypeInfoObject::VerificationTypeInfoObject(std::string className) { m_className = std::move(className); }
VerificationTypeInfoUninitializedVariable::VerificationTypeInfoUninitializedVariable(u2 offset) { m_offset = offset; }
StackMapFrame::StackMapFrame(u1 type) { m_type = type; }
StackMapFrameExtended::StackMapFrameExtended(u1 type, u2 offsetDelta) : StackMapFrame(type) { m_offsetDelta = offsetDelta; }
StackMapFrameSame::StackMapFrameSame(u1 type) : StackMapFrame(type) {}
StackMapFrameSameLocals::StackMapFrameSameLocals(u1 type, std::vector<verification_type_info_t> stack) : StackMapFrame(type) { m_stack = std::move(stack); }
StackMapFrameSameLocalsExtended::StackMapFrameSameLocalsExtended(u1 type, u2 offset, std::vector<verification_type_info_t> stack) : StackMapFrameExtended(type, offset) { m_stack = std::move(stack); }
StackMapFrameChop::StackMapFrameChop(u1 type, u2 offsetDelta) : StackMapFrameExtended(type, offsetDelta) {}
StackMapFrameAppend::StackMapFrameAppend(u1 type, u2 offsetDelta, std::vector<verification_type_info_t> locals) : StackMapFrameExtended(type,offsetDelta) { m_locals = std::move(locals); }
StackMapFrameFull::StackMapFrameFull(u1 type, u2 offsetDelta, std::vector<verification_type_info_t> locals, std::vector<verification_type_info_t> stack) : StackMapFrameExtended(type, offsetDelta) {
    m_locals = std::move(locals);
    m_stack = std::move(stack);
}
StackMapTableAttribute::StackMapTableAttribute(std::string name, std::vector<stack_map_frame_t> entries) : Attribute(std::move(name)) { m_entries = std::move(entries); }
SourceFileAttribute::SourceFileAttribute(std::string name, std::string sourceFileName) : Attribute(std::move(name)) { m_sourceFileName = std::move(sourceFileName); }
LineNumberEntry::LineNumberEntry(u2 startPc, u2 lineNumber) {
    m_startPc = startPc;
    m_lineNumber = lineNumber;
}
LineNumberTableAttribute::LineNumberTableAttribute(std::string name, std::vector<LineNumberEntry> table) : Attribute(std::move(name)) {
    m_lineNumberTable = std::move(table);
}
DeprecatedAttribute::DeprecatedAttribute(std::string name) : Attribute(std::move(name)) {}
RuntimeAnnotationValue::RuntimeAnnotationValue(u1 tag) { m_tag = tag; }
ConstValueRuntimeAnnotationValue::ConstValueRuntimeAnnotationValue(u1 tag, const_value_t constValue) : RuntimeAnnotationValue(tag) { m_constValue = std::move(constValue); }
EnumConstValue::EnumConstValue(std::string typeName, std::string constName) {
    m_typeName = std::move(typeName);
    m_constName = std::move(constName);
}
EnumConstValueRuntimeAnnotationValue::EnumConstValueRuntimeAnnotationValue(u1 tag, std::string typeName, std::string constName) : RuntimeAnnotationValue(tag), m_enumConstValue(std::move(typeName), std::move(constName)) {}
ClassInfoRuntimeAnnotationValue::ClassInfoRuntimeAnnotationValue(u1 tag, FieldDescriptor returnDescriptor) : RuntimeAnnotationValue(tag) {
    m_returnDescriptor = std::move(returnDescriptor);
}
ArrayValueRuntimeAnnotationValue::ArrayValueRuntimeAnnotationValue(u1 tag, std::vector<runtime_annotation_value_t> values) : RuntimeAnnotationValue(tag) {
    m_values = std::move(values);
}
RuntimeAnnotationsKeyValuePair::RuntimeAnnotationsKeyValuePair(std::string elementName, const runtime_annotation_value_t& value) : m_value(value) {
    m_elementName = std::move(elementName);
    m_value = value;
}
RuntimeAnnotation::RuntimeAnnotation(std::string type, std::vector<RuntimeAnnotationsKeyValuePair> elements) : m_annotationElements(std::move(elements)), m_type() {
    auto descriptor = parse_descriptor(std::move(type));
    m_type = FieldDescriptor(descriptor);
}
RuntimeVisibleAnnotationsAttribute::RuntimeVisibleAnnotationsAttribute(std::string name, std::vector<RuntimeAnnotation> annotations) : Attribute(std::move(name)), m_runtimeVisibleAnnotations(std::move(annotations)) {}
CodeAttribute::CodeAttribute(std::string name, u2 maxStack, u2 maxLocals, std::vector<u1> code, std::vector<ExceptionTableEntry> exceptionTable, std::vector<attribute_t> attributes) : Attribute(std::move(name)) {
    m_maxStack = maxStack;
    m_maxLocals = maxLocals;
    m_code = std::move(code);
    m_exceptionTable = std::move(exceptionTable);
    m_attributes = std::move(attributes);
}
#pragma clang diagnostic pop