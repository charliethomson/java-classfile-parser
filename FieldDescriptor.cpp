//
// Created by charlie.thomson on 10/21/2022.
//

#include "FieldDescriptor.h"

descriptor *parse_descriptor(std::string raw) {
    size_t offset = 0;
    if (raw.starts_with('(')) offset++;
    auto start = raw.begin() + offset;
    auto end = raw.begin() + offset + 1;
    if (end >= raw.end()) end = raw.end();
    switch (raw.at(offset))
    {
#define o(C, T) case C: \
                    return new descriptor {  \
                        .raw=std::string(raw.begin() + offset, raw.begin() + offset + 1), \
                        .type=T,                                                          \
                        .inner=std::nullopt,                                              \
                        .remainder=std::string(raw.begin() + offset + 1, raw.end()),      \
                        .offset=offset + 1,                                               \
                    };
        o('B', "byte") // signed byte
        o('C', "char") // Unicode character code point in the Basic Multilingual Plane, encoded with UTF-16
        o('D', "double") // double-precision floating-point value
        o('F', "float") // single-precision floating-point value
        o('I', "int") // integer
        o('J', "long") // long integer
        o('S', "short") // signed short
        o('Z', "boolean") // true or false
        o('V', "void") // A null return type
#undef o
    }
    if (raw.at(offset) == 'L') { // an instance of class m_inner

        while (raw.at(offset++) != ';');

        auto text = std::string(start, raw.begin()+offset);
        std::string innerText = text.substr(1, text.size() - 2);
        auto inner = parse_descriptor(std::move(innerText));

        auto result = new descriptor {
                .raw=text,
                .type="class",
                .inner=std::optional(inner),
                .remainder=std::string(raw.begin() + offset, raw.end()),
                .offset=offset + inner->offset,
        };

        return result;
    } else if (raw.at(offset ) == '[') { // on array dimension
        auto innerText = raw.substr(++offset, raw.size());
        auto inner = parse_descriptor(std::move(innerText));

        auto result =  new descriptor{
                .raw=raw.substr(0, offset + inner->offset),
                .type="array",
                .inner=std::optional(inner),
                .remainder=std::string(raw.begin() + offset+inner->offset, raw.end()),
                .offset=offset + inner->offset,
        };
        return result;
    } else {
        return new descriptor {
                .raw=raw,
                .type=raw,
                .inner=std::nullopt,
                .remainder=std::string(),
                .offset=offset,
        };

    }
}

FieldDescriptor::FieldDescriptor() { m_raw = ""; }
FieldDescriptor::FieldDescriptor(descriptor *desc) {
    m_raw = desc->raw;
    m_type = desc->type;
    m_inner = std::nullopt;
    if (desc->inner.has_value()) {
        auto inner = new FieldDescriptor(desc->inner.value());
        m_inner = std::optional(inner);
    }
}