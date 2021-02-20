cmake_minimum_required(VERSION 3.9)
project(Glacier VERSION 0.1.0 DESCRIPTION "Glacier Engine")

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Glacier
option(GLACIER_DYNAMIC_LINK "Link Glacier dynamically" ON)
add_subdirectory(Glacier)

# Sandbox
add_subdirectory(Sandbox)

set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Sandbox)

#add_custom_command(TARGET Glacier POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE_DIR:Glacier>/Glacier.dll" "$<TARGET_FILE_DIR:Sandbox>")
#add_custom_command(TARGET Glacier POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE_DIR:Glacier>/Glacier.pdb" "$<TARGET_FILE_DIR:Sandbox>")
