cmake_minimum_required(VERSION 3.9)
project(Glacier VERSION 0.1.0 DESCRIPTION "Glacier Engine")

set(CMAKE_CXX_STANDARD 17)

# Glacier
option(GLACIER_DYNAMIC_LINK "Link Glacier dynamically" ON)
add_subdirectory(Glacier)

# Sandbox
add_subdirectory(Sandbox)
