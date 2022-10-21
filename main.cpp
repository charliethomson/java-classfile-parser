
#include "ClassFileParser.h"
#include "JavaClass.h"

const char *FILEPATH = R"(C:\src\pocs\speed\Main.class)";
int main() {
    auto parsedClass = ClassFileParser(FILEPATH);
    auto clazz = new JavaClass(parsedClass);
    return 0;
}