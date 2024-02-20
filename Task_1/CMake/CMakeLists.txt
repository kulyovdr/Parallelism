cmake_minimum_required(VERSION 3.22)

project ("Task_1")

add_executable (Task1Float "Task_1.cpp")

#FL = 1 for "float", FL = {other values} for "double"
target_compile_definitions (Task1Float PRIVATE FL=1)