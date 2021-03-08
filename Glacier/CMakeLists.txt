# Glacier
set(Headers
	include/glacier.hpp
	include/core/common.hpp
	include/core/Application.hpp
	include/io/Buffer.hpp
	include/io/File.hpp
	include/graphics/BufferElement.hpp
	include/graphics/IndexBuffer.hpp
	include/graphics/LifecycleObject.hpp
	include/graphics/Pipeline.hpp
	include/graphics/Renderer.hpp
	include/graphics/Shader.hpp
	include/graphics/UniformBuffer.hpp
	include/graphics/VertexBuffer.hpp
	include/graphics/Window.hpp
	include/internal/utility.hpp
)

set(Sources
	src/core/common.cpp
	src/core/Application.cpp
	src/io/File.cpp
	src/graphics/IndexBuffer.cpp
	src/graphics/Pipeline.cpp
	src/graphics/Renderer.cpp
	src/graphics/Shader.cpp
	src/graphics/UniformBuffer.cpp
	src/graphics/VertexBuffer.cpp
	src/graphics/Window.cpp
	src/internal/utility.cpp
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
