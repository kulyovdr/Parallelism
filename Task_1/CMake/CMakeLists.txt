project ("Task_1")

add_executable (Task1Double "Task_1.cpp")

#FL = 1 for "float", FL = {other values} for "double"
target_compile_definitions (Task1Double PRIVATE FL=0)