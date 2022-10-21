//
// Created by charlie.thomson on 10/20/2022.
//

#include "AccessFlags.h"
#include <vector>
#include <string>

std::vector<std::string>AccessFlags::description() {
    std::vector<std::string> descriptions;
    if (isPublic()) descriptions.emplace_back("Declared public; may be accessed from outside its package.");
    if (isFinal()) descriptions.emplace_back("Declared final; no subclasses allowed.");
    if (isSuper()) descriptions.emplace_back("Treat superclass methods specially when invoked by the invokespecial instruction.");
    if (isInterface()) descriptions.emplace_back("Is an interface, not a class.");
    if (isAbstract()) descriptions.emplace_back("Declared abstract; must not be instantiated.");
    if (isSynthetic()) descriptions.emplace_back("Declared synthetic; not present in the source code.");
    if (isAnnotation()) descriptions.emplace_back("Declared as an annotation type.");
    if (isEnum()) descriptions.emplace_back("Declared as an enum type.");
    return descriptions;
}
std::vector<std::string> AccessFlags::name() {
    std::vector<std::string> names;
    if (isPublic()) names.emplace_back("ACC_PUBLIC");
    if (isFinal()) names.emplace_back("ACC_FINAL");
    if (isSuper()) names.emplace_back("ACC_SUPER");
    if (isInterface()) names.emplace_back("ACC_INTERFACE");
    if (isAbstract()) names.emplace_back("ACC_ABSTRACT");
    if (isSynthetic()) names.emplace_back("ACC_SYNTHETIC");
    if (isAnnotation()) names.emplace_back("ACC_ANNOTATION");
    if (isEnum()) names.emplace_back("ACC_ENUM");
    return names;
}