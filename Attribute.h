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
    explicit Attribute(std::string name);
};
class ConstantValueAttribute : public Attribute {
private:
    static const uint8_t ConstantValueType = 0;
    void *m_constant;
public:
    explicit ConstantValueAttribute(std::string name);
};
// ConstantValue typed attributes
#define o(N, T, F) class ConstantValueAttribute##N : ConstantValueAttribute {                \
private:                                                                                     \
    static const uint8_t ConstantValueType = F;                                              \
public:                                                                                      \
    T m_constant;                                                                            \
    ConstantValueAttribute##N(std::string name, T constant);                                 \
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
    explicit VerificationTypeInfoObject(std::string className);
};
class VerificationTypeInfoUninitializedVariable : VerificationTypeInfo {
public:
    static u1 Tag() { return 8; }
    u2 m_offset;
    explicit VerificationTypeInfoUninitializedVariable(u2 offset);
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
    explicit StackMapFrame(u1 type);;
};
class StackMapFrameExtended : StackMapFrame {
public:
    u2 m_offsetDelta;
    StackMapFrameExtended(u1 type, u2 offsetDelta);
};
class StackMapFrameSame : StackMapFrame {
public:
    explicit StackMapFrameSame(u1 type);
};
class StackMapFrameSameExtended : StackMapFrameExtended {};
class StackMapFrameSameLocals : StackMapFrame {
public:
    std::vector<verification_type_info_t> m_stack;
    StackMapFrameSameLocals(u1 type, std::vector<verification_type_info_t> stack);
};
class StackMapFrameSameLocalsExtended : StackMapFrameExtended {
public:
    std::vector<verification_type_info_t> m_stack;
    StackMapFrameSameLocalsExtended(u1 type, u2 offset, std::vector<verification_type_info_t> stack);
};
class StackMapFrameChop : StackMapFrameExtended {
public:
    StackMapFrameChop(u1 type, u2 offsetDelta);
};
class StackMapFrameAppend : StackMapFrameExtended {
public:
    std::vector<verification_type_info_t> m_locals;
    StackMapFrameAppend(u1 type, u2 offsetDelta, std::vector<verification_type_info_t> locals);
};
class StackMapFrameFull : StackMapFrameExtended {
public:
    std::vector<verification_type_info_t> m_locals;
    std::vector<verification_type_info_t> m_stack;
    StackMapFrameFull(u1 type, u2 offsetDelta, std::vector<verification_type_info_t> locals, std::vector<verification_type_info_t> stack);
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
    StackMapTableAttribute(std::string name, std::vector<stack_map_frame_t> entries);
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
    SourceFileAttribute(std::string name, std::string sourceFileName);
};
class SourceDebugExtensionAttribute : public Attribute {
    std::string m_debugExtensions;
};

class LineNumberEntry {
public:
    u2 m_startPc;
    u2 m_lineNumber;
    LineNumberEntry(u2 startPc, u2 lineNumber);
};
class LineNumberTableAttribute : public Attribute {
public:
    std::vector<LineNumberEntry> m_lineNumberTable;
    LineNumberTableAttribute(std::string name, std::vector<LineNumberEntry> table);
};

class BytecodeSpan { u2 m_start; u2 m_length; u2 m_end; };

class LocalVariableEntry {
    BytecodeSpan m_span;
    std::string m_name;
    std::string m_descriptor;
    u2 m_index;
};

class LocalVariableTableAttribute : public Attribute {
    std::vector<LocalVariableEntry> m_localVariableTable;
};

class LocalVariableTypeEntry {
    BytecodeSpan m_span;
    std::string m_name;
    std::string m_signature;
    u2 m_index;
};
class LocalVariableTypeTableAttribute : public Attribute {
    std::vector<LocalVariableTypeEntry> m_localVariableTypeTable;
};

class DeprecatedAttribute : public Attribute {
public:
    explicit DeprecatedAttribute(std::string name);
};


class RuntimeAnnotationValue {
public:
    u1 m_tag;
    explicit RuntimeAnnotationValue(u1 tag);
};
typedef std::variant<uint32_t, uint64_t, float, double, std::string> const_value_t;

class ConstValueRuntimeAnnotationValue : RuntimeAnnotationValue {
public:
    const_value_t m_constValue;
    ConstValueRuntimeAnnotationValue(u1 tag, const_value_t constValue);
};
class EnumConstValue {
public:
    std::string m_typeName;
    std::string m_constName;
    EnumConstValue(std::string typeName, std::string constName);
};
class EnumConstValueRuntimeAnnotationValue : RuntimeAnnotationValue {
public:
    EnumConstValue m_enumConstValue;
    EnumConstValueRuntimeAnnotationValue(u1 tag, std::string typeName, std::string constName);
};
class ClassInfoRuntimeAnnotationValue : RuntimeAnnotationValue {
public:
    FieldDescriptor m_returnDescriptor;
    ClassInfoRuntimeAnnotationValue(u1 tag, FieldDescriptor returnDescriptor);
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
    ArrayValueRuntimeAnnotationValue(u1 tag, std::vector<runtime_annotation_value_t> values);
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
    RuntimeAnnotationsKeyValuePair(std::string elementName, const runtime_annotation_value_t& value);
};
class RuntimeAnnotation {
public:
    FieldDescriptor m_type;
    std::vector<RuntimeAnnotationsKeyValuePair> m_annotationElements;
    RuntimeAnnotation(std::string type, std::vector<RuntimeAnnotationsKeyValuePair> elements);
};
class RuntimeVisibleAnnotationsAttribute : public Attribute {
public:
    std::vector<RuntimeAnnotation> m_runtimeVisibleAnnotations;
    RuntimeVisibleAnnotationsAttribute(std::string name, std::vector<RuntimeAnnotation> annotations);;
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
    u2 m_maxStack;
    u2 m_maxLocals;
    std::vector<u1> m_code;
    std::vector<ExceptionTableEntry> m_exceptionTable;
    std::vector<attribute_t> m_attributes;

    CodeAttribute(
            std::string name,
            u2 maxStack,
            u2 maxLocals,
            std::vector<u1> code,
            std::vector<ExceptionTableEntry> exceptionTable,
            std::vector<attribute_t> attributes
            );
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

#define o(A)                                                           \
static bool is##A##Attribute(std::string &name) { return name == #A; } \
attribute_t make##A##Attribute(const std::string& name, const std::vector<u1> &bytes, std::vector<ConstantInfo> &constantsPool);
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
T read_vec_un(std::vector<u1> bytes, size_t &cursor, size_t n);
#define o(T,N) static T read_vec_##T(std::vector<u1> bytes, size_t &cursor);
o(u1, 1)
o(u2, 2)
o(u4, 4)
#undef o

RuntimeAnnotation readRuntimeAnnotation(const std::vector<u1>& bytes, std::vector<ConstantInfo> &constantsPool, size_t &cursor);

#endif //SPEED_ATTRIBUTE_H

attribute_t make_attribute(std::string name, const std::vector<u1>& bytes, std::vector<ConstantInfo> &constantsPool);
verification_type_info_t readVerificationType(const std::vector<u1>& bytes, size_t &cursor, std::vector<ConstantInfo> &constantsPool);
