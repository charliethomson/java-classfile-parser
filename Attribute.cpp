//
// Created by charlie.thomson on 10/19/2022.


#include "Attribute.h"

attribute_t make_attribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool) {
#define o(A) if (is##A##Attribute(name)) return make##A##Attribute(name, bytes, constantsPool);
    o(ConstantValue)
    o(Code)
    o(LineNumberTable)
    o(StackMapTable)
    o(SourceFile)
    o(Deprecated)
    o(RuntimeVisibleAnnotations)
#undef o
    std::cerr << "No handler configured for attr of type " << name << std::endl;
    throw std::exception();
}

attribute_t makeConstantValueAttribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool) {
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

attribute_t makeCodeAttribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool) {
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
    return CodeAttribute(std::move(name), maxStack, maxLocals, code, exceptionTable, attributes);
}

verification_type_info_t readVerificationType(std::vector<u1> bytes, size_t &cursor, std::vector<ConstantInfo> &constantsPool) {
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

attribute_t makeStackMapTableAttribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool) {
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

// attribute_t makeExceptionsAttribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool) {
//    return Attribute("TEST");
//}

// attribute_t makeInnerClassesAttribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool) {
//    return Attribute("TEST");
//}

// attribute_t makeEnclosingMethodAttribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool) {
//    return Attribute("TEST");
//}

// attribute_t makeSyntheticAttribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool) {
//    return Attribute("TEST");
//}

// attribute_t makeSignatureAttribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool) {
//    return Attribute("TEST");
//}

 attribute_t makeSourceFileAttribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool) {
    size_t cursor = 0;
    auto sourceFileIndex = read_vec_u2(bytes, cursor);
    auto sourceFileName = std::string(constantsPool.at(sourceFileIndex - 1).expectUtf8Info().bytes);

    return SourceFileAttribute(name, sourceFileName);
}

// attribute_t makeSourceDebugExtensionAttribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool) {
//    return Attribute("TEST");
//}

 attribute_t makeLineNumberTableAttribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool) {
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

// attribute_t makeLocalVariableTableAttribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool) {
//    return Attribute("TEST");
//}

// attribute_t makeLocalVariableTypeTableAttribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool) {
//    return Attribute("TEST");
//}

 attribute_t makeDeprecatedAttribute(std::string name, std::vector<u1> _bytes, std::vector<ConstantInfo> &_constantsPool) {
    return DeprecatedAttribute(name);
}

runtime_annotation_value_t readElementValue(std::vector<u1> bytes, size_t &cursor, std::vector<ConstantInfo> &constantsPool) {
    auto tag = read_vec_u1(bytes, cursor);
    if (std::string("BCDFIJSZs").find(tag) != std::string::npos) {
        auto constValueIndex = read_vec_u2(bytes, cursor);
        return ConstValueRuntimeAnnotationValue(tag, constValueIndex);
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

RuntimeAnnotation readRuntimeAnnotation(std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool, size_t &cursor) {
    auto typeIndex = read_vec_u2(bytes, cursor);
    auto numPairs = read_vec_u2(bytes, cursor);
    std::vector<RuntimeAnnotationsKeyValuePair> pairs;
    for (size_t i = 0; i < numPairs; ++i) {
        auto elementNameIndex = read_vec_u2(bytes, cursor);
        auto elementName = std::string(constantsPool.at(elementNameIndex - 1).expectUtf8Info().bytes);
        auto elementValue = readElementValue(bytes, cursor, constantsPool);
        pairs.emplace_back(RuntimeAnnotationsKeyValuePair(elementName, elementValue));
    }

    auto type = std::string(constantsPool.at(typeIndex).expectUtf8Info().bytes);
    return RuntimeAnnotation(type, pairs);
}

attribute_t makeRuntimeVisibleAnnotationsAttribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool) {
    size_t cursor = 0;
    auto numAnnotations = read_vec_u2(bytes, cursor);
    std::vector<RuntimeAnnotation> annotations;
    for (int i = 0; i < numAnnotations; ++i)
        annotations.emplace_back(readRuntimeAnnotation(bytes, constantsPool, cursor));

    return RuntimeVisibleAnnotationsAttribute(name, annotations);
}

// attribute_t makeRuntimeInvisibleAnnotationsAttribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool) {
//    return Attribute("TEST");
//}

// attribute_t makeRuntimeVisibleParameterAnnotationsAttribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool) {
//    return Attribute("TEST");
//}

// attribute_t makeRuntimeInvisibleParameterAnnotationsAttribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool) {
//    return Attribute("TEST");
//}

// attribute_t makeAnnotationDefaultAttribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool) {
//    return Attribute("TEST");
//}

// attribute_t makeBootstrapMethodsAttribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool) {
//    return Attribute("TEST");
//}
