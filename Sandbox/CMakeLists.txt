set(Headers
	include/SandboxApp.hpp)

set(Sources
	src/main.cpp)

add_executable(Sandbox ${Sources} ${Headers})
target_link_libraries(Sandbox Glacier)

target_include_directories(Sandbox PRIVATE ${PROJECT_SOURCE_DIR}/Sandbox/include)

add_custom_command(TARGET Sandbox POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE_DIR:Glacier>/Glacier.dll" "$<TARGET_FILE_DIR:Sandbox>")
add_custom_command(TARGET Sandbox POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE_DIR:Glacier>/Glacier.pdb" "$<TARGET_FILE_DIR:Sandbox>")
