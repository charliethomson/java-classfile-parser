//
// Created by charlie.thomson on 10/20/2022.
//

#ifndef SPEED_ACCESSFLAGS_H
#define SPEED_ACCESSFLAGS_H

#include "ClassFileParser.h"
#include <string>
#include <vector>

class AccessFlags {
private:
    u2 m_inner;
public:
    explicit AccessFlags(u2 inner);

    [[nodiscard]] std::vector<std::string>name() const;
    [[nodiscard]] std::vector<std::string>description() const;
    [[nodiscard]] bool isPublic() const {
        return (m_inner & 0x0001) != 0;
    }
    [[nodiscard]] bool isFinal() const {
        return (m_inner & 0x0010) != 0;
    }
    [[nodiscard]] bool isSuper() const {
        return (m_inner & 0x0020) != 0;
    }
    [[nodiscard]] bool isInterface() const {
        return (m_inner & 0x0200) != 0;
    }
    [[nodiscard]] bool isAbstract() const {
        return (m_inner & 0x0400) != 0;
    }
    [[nodiscard]] bool isSynthetic() const {
        return (m_inner & 0x1000) != 0;
    }
    [[nodiscard]] bool isAnnotation() const {
        return (m_inner & 0x2000) != 0;
    }
    [[nodiscard]] bool isEnum() const {
        return (m_inner & 0x4000) != 0;
    }
};



#endif //SPEED_ACCESSFLAGS_H
