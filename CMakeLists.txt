cmake_minimum_required(VERSION 3.23)
project(speed)

set(CMAKE_CXX_STANDARD 20)

add_executable(speed main.cpp ClassFileParser.cpp ClassFileParser.h JavaClass.cpp JavaClass.h Attribute.cpp Attribute.h AccessFlags.cpp AccessFlags.h ConstantInfo.cpp ConstantInfo.h FieldDescriptor.cpp FieldDescriptor.h)
