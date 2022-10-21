//
// Created by charlie.thomson on 10/19/2022.
//

#pragma once
#ifndef SPEED_ATTRIBUTE_H
#define SPEED_ATTRIBUTE_H

#include "ClassFileParser.h"
#include "AccessFlags.h"
#include "ConstantInfo.h"
#include "FieldDescriptor.h"

#include <vector>
#include <string>
#include <optional>
#include <cstdint>
#include <variant>

class Attribute {
public:
    std::string m_name;
    Attribute(std::string name) : m_name(name) {}
};
class ConstantValueAttribute : public Attribute {
private:
    static const uint8_t ConstantValueType = 0;
    void *m_constant;
public:
    ConstantValueAttribute(std::string name) : Attribute(name) {
        m_constant = NULL;
    }
};
// ConstantValue typed attributes
#define o(N, T, F) class ConstantValueAttribute##N : ConstantValueAttribute {                \
private:                                                                                     \
    static const uint8_t ConstantValueType = F;                                              \
public:                                                                                      \
    T m_constant;                                                                            \
    ConstantValueAttribute##N(std::string name, T constant) : ConstantValueAttribute(name) { \
        m_constant = constant;                                                               \
    }                                                                                        \
};
o(Long, uint64_t, 1)
o(String, std::string, 2)
o(Float, float, 4)
o(Double, double, 8)
o(Integer, uint32_t, 16)
#undef o
// End: ConstantValue typed attributes

class ExceptionTableEntry {
public:
    u2 m_startPc;
    u2 m_endPc;
    u2 m_handlerPc;
    u2 m_catchType;
    ExceptionTableEntry( u2 startPc, u2 endPc, u2 handlerPc, u2 catchType ) {
        m_startPc = startPc;
        m_endPc = endPc;
        m_handlerPc = handlerPc;
        m_catchType = catchType;
    }
};

class VerificationTypeInfo {
public:
    static u1 Tag() { return 16; }
};
#define o(N,V) class VerificationTypeInfo##N : VerificationTypeInfo { public: static u1 Tag() { return V; } VerificationTypeInfo##N() {} };
o(Top,0)
o(Integer, 1)
o(Float, 2)
o(Double, 3)
o(Long, 4)
o(Null, 5)
o(UninitializedThis, 6)
#undef o
class VerificationTypeInfoObject : VerificationTypeInfo {
public:
    static u1 Tag() { return 7; }
    std::string m_className;
    VerificationTypeInfoObject(std::string className) { m_className = className; }
};
class VerificationTypeInfoUninitializedVariable : VerificationTypeInfo {
public:
    static u1 Tag() { return 8; }
    u2 m_offset;
    VerificationTypeInfoUninitializedVariable(u2 offset) { m_offset = offset; }
};
typedef std::variant<
        VerificationTypeInfoObject,
        VerificationTypeInfoUninitializedVariable,
        VerificationTypeInfoTop,
        VerificationTypeInfoInteger,
        VerificationTypeInfoFloat,
        VerificationTypeInfoDouble,
        VerificationTypeInfoLong,
        VerificationTypeInfoNull,
        VerificationTypeInfoUninitializedThis> verification_type_info_t;

class StackMapFrame {
public:
    u1 m_type;
    StackMapFrame(u1 type) { m_type = type; };
};
class StackMapFrameExtended : StackMapFrame {
public:
    u2 m_offsetDelta;
    StackMapFrameExtended(u1 type, u2 offsetDelta) : StackMapFrame(type) { m_offsetDelta = offsetDelta; }
};
class StackMapFrameSame : StackMapFrame {
public:
    StackMapFrameSame(u1 type) : StackMapFrame(type) {}
};
class StackMapFrameSameExtended : StackMapFrameExtended {};
class StackMapFrameSameLocals : StackMapFrame {
public:
    std::vector<verification_type_info_t> m_stack;
    StackMapFrameSameLocals(u1 type, std::vector<verification_type_info_t> stack) : StackMapFrame(type) { m_stack = stack; }
};
class StackMapFrameSameLocalsExtended : StackMapFrameExtended {
public:
    std::vector<verification_type_info_t> m_stack;
    StackMapFrameSameLocalsExtended(u1 type, u2 offset, std::vector<verification_type_info_t> stack) : StackMapFrameExtended(type, offset) { m_stack = stack; }
};
class StackMapFrameChop : StackMapFrameExtended {
public:
    StackMapFrameChop(u1 type, u2 offsetDelta) : StackMapFrameExtended(type, offsetDelta) {}
};
class StackMapFrameAppend : StackMapFrameExtended {
public:
    std::vector<verification_type_info_t> m_locals;
    StackMapFrameAppend(u1 type, u2 offsetDelta, std::vector<verification_type_info_t> locals) : StackMapFrameExtended(type,offsetDelta) { m_locals = locals; }
};
class StackMapFrameFull : StackMapFrameExtended {
public:
    std::vector<verification_type_info_t> m_locals;
    std::vector<verification_type_info_t> m_stack;
    StackMapFrameFull(u1 type, u2 offsetDelta, std::vector<verification_type_info_t> locals, std::vector<verification_type_info_t> stack) : StackMapFrameExtended(type, offsetDelta) {
        m_locals = locals;
        m_stack = stack;
    }
};
typedef std::variant<
        StackMapFrameExtended,
        StackMapFrameSame,
        StackMapFrameSameExtended,
        StackMapFrameSameLocals,
        StackMapFrameSameLocalsExtended,
        StackMapFrameChop,
        StackMapFrameAppend,
        StackMapFrameFull> stack_map_frame_t;

class StackMapTableAttribute : public Attribute {
public:
    std::vector<stack_map_frame_t> m_entries;
    StackMapTableAttribute(std::string name, std::vector<stack_map_frame_t> entries) : Attribute(name) { m_entries = entries; }
};

class ExceptionsAttribute : public Attribute {
    std::vector<std::string> m_exceptions;
};

class InnerClass {
    std::string m_innerClassName;
    std::optional<std::string> m_outerClassName;
    AccessFlags m_innerClassAccessFlags;
};

class InnerClassesAttribute : public Attribute {
    std::vector<InnerClass> m_classes;
};

class EnclosingMethodAttribute : Attribute  {
    std::string m_containingClassName;
    std::string m_methodName;
    std::string m_descriptor;
};

class SyntheticAttribute : public Attribute {
};
class SignatureAttribute : public Attribute {
    std::string m_signature;
};
class SourceFileAttribute : public Attribute {
public:
    std::string m_sourceFileName;
    SourceFileAttribute(std::string name, std::string sourceFileName) : Attribute(name) { m_sourceFileName = sourceFileName; }
};
class SourceDebugExtensionAttribute : public Attribute {
    std::string m_debugExtensions;
};

class LineNumberEntry {
public:
    u2 m_startPc;
    u2 m_lineNumber;
    LineNumberEntry(u2 startPc, u2 lineNumber) {
        m_startPc = startPc;
        m_lineNumber = lineNumber;
    }
};
class LineNumberTableAttribute : public Attribute {
public:
    std::vector<LineNumberEntry> m_lineNumberTable;
    LineNumberTableAttribute(std::string name, std::vector<LineNumberEntry> table) : Attribute(name) {
        m_lineNumberTable = table;
    }
};

class BytecodeSpan { u2 m_start; u2 m_length; u2 m_end; };

class LocalVariableEntry {
    BytecodeSpan m_span;
    std::string m_name;
    std::string m_descriptor;
    u2 m_index;
};

class LocalVariableTableAttribute : public Attribute {
    std::vector<LocalVariableEntry> m_localVairableTable;
};

class LocalVariableTypeEntry {
    BytecodeSpan m_span;
    std::string m_name;
    std::string m_signature;
    u2 m_index;
};
class LocalVariableTypeTableAttribute : public Attribute {
    std::vector<LocalVariableTypeEntry> m_localVairableTypeTable;
};

class DeprecatedAttribute : public Attribute {
public:
    DeprecatedAttribute(std::string name) : Attribute(name) {}
};


class RuntimeAnnotationValue {
public:
    u1 m_tag;
    RuntimeAnnotationValue(u1 tag) { m_tag = tag; }
};
class ConstValueRuntimeAnnotationValue : RuntimeAnnotationValue {
public:
    u2 m_constValueIndex;
    ConstValueRuntimeAnnotationValue(u1 tag, u2 constValueIndex) : RuntimeAnnotationValue(tag) { m_constValueIndex = constValueIndex; }
};
class EnumConstValue {
public:
    std::string m_typeName;
    std::string m_constName;
    EnumConstValue(std::string typeName, std::string constName) {
        m_typeName = typeName;
        m_constName = constName;
    }
};
class EnumConstValueRuntimeAnnotationValue : RuntimeAnnotationValue {
public:
    EnumConstValue m_enumConstValue;
    EnumConstValueRuntimeAnnotationValue(u1 tag, std::string typeName, std::string constName) : RuntimeAnnotationValue(tag),
                                                                                  m_enumConstValue(typeName, constName) {}
};
class ClassInfoRuntimeAnnotationValue : RuntimeAnnotationValue {
public:
    FieldDescriptor m_returnDescriptor;
    ClassInfoRuntimeAnnotationValue(u1 tag, FieldDescriptor returnDescriptor) : RuntimeAnnotationValue(tag) {
        m_returnDescriptor = returnDescriptor;
    }
};
class RecursiveRuntimeAnnotationValue : RuntimeAnnotationValue {
public:
    // TODO:
};
class ArrayValueRuntimeAnnotationValue : RuntimeAnnotationValue {
public:
    typedef std::variant<
            RuntimeAnnotationValue,
            ConstValueRuntimeAnnotationValue,
            ClassInfoRuntimeAnnotationValue,
            EnumConstValueRuntimeAnnotationValue,
            ArrayValueRuntimeAnnotationValue> runtime_annotation_value_t;
    std::vector<runtime_annotation_value_t> m_values;
    ArrayValueRuntimeAnnotationValue(u1 tag, std::vector<runtime_annotation_value_t> values) : RuntimeAnnotationValue(tag) {
        m_values = values;
    }
};

typedef std::variant<
        RuntimeAnnotationValue,
        ConstValueRuntimeAnnotationValue,
        ClassInfoRuntimeAnnotationValue,
        EnumConstValueRuntimeAnnotationValue,
        ArrayValueRuntimeAnnotationValue> runtime_annotation_value_t;

class RuntimeAnnotationsKeyValuePair {
public:
    std::string m_elementName;
    runtime_annotation_value_t m_value;
    RuntimeAnnotationsKeyValuePair(std::string elementName, runtime_annotation_value_t value) : m_value(value) {
        m_elementName = elementName;
        m_value = value;
    }
};
class RuntimeAnnotation {
public:
    std::string m_type;
    std::vector<RuntimeAnnotationsKeyValuePair> m_annotationElements;
    RuntimeAnnotation(std::string type, std::vector<RuntimeAnnotationsKeyValuePair> elements) : m_annotationElements(elements) {
        m_type = type;
    }
};
class RuntimeVisibleAnnotationsAttribute : public Attribute {
public:
    std::vector<RuntimeAnnotation> m_runtimeVisibleAnnotations;
    RuntimeVisibleAnnotationsAttribute(std::string name, std::vector<RuntimeAnnotation> annotations)
        : Attribute(name), m_runtimeVisibleAnnotations(annotations) {};
};
class AnnotationValueRuntimeAnnotationValue : RuntimeAnnotationValue {
    RuntimeAnnotation annotation;
};
class RuntimeInvisibleAnnotationsAttribute : public Attribute {
    std::vector<RuntimeAnnotation> m_runtimeInvisibleAnnotations;
};
class RuntimeVisibleParameterAnnotationsAttribute : public Attribute {
    std::vector<std::vector<RuntimeAnnotation>> m_runtimeVisibleParameterAnnotations;
};
class RuntimeInvisibleParameterAnnotationsAttribute : public Attribute {
    std::vector<std::vector<RuntimeAnnotation>> m_runtimeInvisibleParameterAnnotations;
};
class AnnotationDefaultAttribute : public Attribute {
    RuntimeAnnotationValue defaultValue;
};

class BootstrapMethodsAttribute : public Attribute {
    std::vector<u2> m_bootstrapMethodIndices;
};

class CodeAttribute : Attribute
{
public:
    u2 m_maxStack;
    u2 m_maxLocals;
    std::vector<u1> m_code;
    std::vector<ExceptionTableEntry> m_exceptionTable;
    std::vector<std::variant<
            Attribute,
            ConstantValueAttribute,
            CodeAttribute,
            StackMapTableAttribute,
            ExceptionsAttribute,
            InnerClassesAttribute,
            SyntheticAttribute,
            SignatureAttribute,
            SourceFileAttribute,
            SourceDebugExtensionAttribute,
            LineNumberTableAttribute,
            LocalVariableTableAttribute,
            LocalVariableTypeTableAttribute,
            DeprecatedAttribute,
            RuntimeVisibleAnnotationsAttribute,
            RuntimeInvisibleAnnotationsAttribute,
            RuntimeVisibleParameterAnnotationsAttribute,
            RuntimeInvisibleParameterAnnotationsAttribute,
            AnnotationDefaultAttribute,
            BootstrapMethodsAttribute,
            ConstantValueAttributeLong,
            ConstantValueAttributeString,
            ConstantValueAttributeFloat,
            ConstantValueAttributeDouble,
            ConstantValueAttributeInteger>> m_attributes;

    CodeAttribute(
            std::string name,
            u2 maxStack,
            u2 maxLocals,
            std::vector<u1> code,
            std::vector<ExceptionTableEntry> exceptionTable,
            std::vector<std::variant<
                    Attribute,
                    ConstantValueAttribute,
                    CodeAttribute,
                    StackMapTableAttribute,
                    ExceptionsAttribute,
                    InnerClassesAttribute,
                    SyntheticAttribute,
                    SignatureAttribute,
                    SourceFileAttribute,
                    SourceDebugExtensionAttribute,
                    LineNumberTableAttribute,
                    LocalVariableTableAttribute,
                    LocalVariableTypeTableAttribute,
                    DeprecatedAttribute,
                    RuntimeVisibleAnnotationsAttribute,
                    RuntimeInvisibleAnnotationsAttribute,
                    RuntimeVisibleParameterAnnotationsAttribute,
                    RuntimeInvisibleParameterAnnotationsAttribute,
                    AnnotationDefaultAttribute,
                    BootstrapMethodsAttribute,
                    ConstantValueAttributeLong,
                    ConstantValueAttributeString,
                    ConstantValueAttributeFloat,
                    ConstantValueAttributeDouble,
                    ConstantValueAttributeInteger>> attributes
            ) : Attribute(name) {
        m_maxStack = maxStack;
        m_maxLocals = maxLocals;
        m_code = code;
        m_exceptionTable = exceptionTable;
        m_attributes = attributes;
    }
};
typedef std::variant<
        Attribute,
        ConstantValueAttribute,
        CodeAttribute,
        StackMapTableAttribute,
        ExceptionsAttribute,
        InnerClassesAttribute,
        SyntheticAttribute,
        SignatureAttribute,
        SourceFileAttribute,
        SourceDebugExtensionAttribute,
        LineNumberTableAttribute,
        LocalVariableTableAttribute,
        LocalVariableTypeTableAttribute,
        DeprecatedAttribute,
        RuntimeVisibleAnnotationsAttribute,
        RuntimeInvisibleAnnotationsAttribute,
        RuntimeVisibleParameterAnnotationsAttribute,
        RuntimeInvisibleParameterAnnotationsAttribute,
        AnnotationDefaultAttribute,
        BootstrapMethodsAttribute,
        ConstantValueAttributeLong,
        ConstantValueAttributeString,
        ConstantValueAttributeFloat,
        ConstantValueAttributeDouble,
        ConstantValueAttributeInteger>  attribute_t;

attribute_t make_attribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constants);

#define o(A)                                                           \
static bool is##A##Attribute(std::string &name) { return name == #A; } \
attribute_t make##A##Attribute(std::string name, std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool);
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

template <typename T>
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
    return read_vec_un<T>(bytes, cursor, N);                                  \
}
o(u1, 1)
o(u2, 2)
o(u4, 4)
#undef o

RuntimeAnnotation readRuntimeAnnotation(std::vector<u1> bytes, std::vector<ConstantInfo> &constantsPool, size_t &cursor);

#endif //SPEED_ATTRIBUTE_H
