set(Headers
	include/SandboxApp.hpp)

set(Sources
	src/main.cpp)

add_executable(Sandbox ${Sources} ${Headers})
target_link_libraries(Sandbox Glacier)

target_include_directories(Sandbox PRIVATE ${PROJECT_SOURCE_DIR}/Sandbox/include)
