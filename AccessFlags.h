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
    AccessFlags(u2 inner) { m_inner = inner; }

    std::vector<std::string>name();
    std::vector<std::string>description();
    const bool isPublic() {
        return (m_inner & 0x0001) != 0;
    }
    const bool isFinal() {
        return (m_inner & 0x0010) != 0;
    }
    const bool isSuper() {
        return (m_inner & 0x0020) != 0;
    }
    const bool isInterface() {
        return (m_inner & 0x0200) != 0;
    }
    const bool isAbstract() {
        return (m_inner & 0x0400) != 0;
    }
    const bool isSynthetic() {
        return (m_inner & 0x1000) != 0;
    }
    const bool isAnnotation() {
        return (m_inner & 0x2000) != 0;
    }
    const bool isEnum() {
        return (m_inner & 0x4000) != 0;
    }
};



#endif //SPEED_ACCESSFLAGS_H
