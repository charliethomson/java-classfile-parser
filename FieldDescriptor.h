//
// Created by charlie.thomson on 10/21/2022.
//

#ifndef SPEED_FIELDDESCRIPTOR_H
#define SPEED_FIELDDESCRIPTOR_H
#include <cstdint>
#include <string>
#include <optional>


struct descriptor {
    std::string raw;
    std::string type;
    std::optional<descriptor *> inner;
    std::string remainder;
    size_t offset;
};
descriptor *parse_descriptor(std::string raw);

class FieldDescriptor {
public:
    std::string m_raw;
    std::string m_type;
    std::optional<FieldDescriptor *> m_inner;
    FieldDescriptor();
    explicit FieldDescriptor(descriptor *desc);
};
#endif //SPEED_FIELDDESCRIPTOR_H
