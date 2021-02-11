# Glacier
set(Headers
	include/glacier.hpp
	include/common.hpp
	include/Window.hpp
	include/Application.hpp
	include/Shader.hpp
	include/PipelineInfo.hpp
	include/File.hpp
	include/Buffer.hpp
)

set(Sources
	src/common.cpp
	src/Window.cpp
	src/Application.cpp
	src/Shader.cpp
	src/File.cpp
)

if(GLACIER_DYNAMIC_LINK)
add_library(Glacier SHARED ${Sources} ${Headers})
target_compile_definitions(Glacier PUBLIC GLACIER_BUILD_DLL PRIVATE GLACIER_BUILD)
else()
add_library(Glacier STATIC ${Sources} ${Headers})
endif()

set_target_properties(Glacier PROPERTIES PUBLIC_HEADER include/glacier.hpp)

target_include_directories(Glacier PUBLIC include)

# Add library directory
set(CMAKE_PREFIX_PATH libraries/spdlog/cmake/)

# Add GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(${PROJECT_SOURCE_DIR}/libraries/glfw ${PROJECT_BINARY_DIR}/glfw)
target_link_libraries(Glacier PRIVATE glfw)

# Add GLM
add_subdirectory(${PROJECT_SOURCE_DIR}/libraries/glm ${PROJECT_BINARY_DIR}/glm)
target_link_libraries(Glacier PUBLIC glm)

# Add Vulkan
find_package(Vulkan REQUIRED FATAL_ERROR)
target_link_libraries(Glacier PRIVATE Vulkan::Vulkan)
target_include_directories(Glacier PRIVATE ${VULKAN_INCLUDE_DIRS})

# Add spdlog
add_subdirectory(${PROJECT_SOURCE_DIR}/libraries/spdlog ${PROJECT_SOURCE_DIR}/libraries/spdlog)
target_link_libraries(Glacier PUBLIC spdlog::spdlog)

#include(GNUInstallDirs)

#install(TARGETS Glacier
#	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
