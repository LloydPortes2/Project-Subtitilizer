cmake_minimum_required(VERSION 3.26)
project(Project2 C CXX)
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(lib/glfw)

add_definitions(-DGL_SILENCE_DEPRECATION)
if(APPLE)
	list(APPEND EXTRA_LIBS
		"-framework OpenGL"
	)
elseif(WIN32)
	list(APPEND EXTRA_LIBS
		"-lglu32 -lopengl32"
	)
	set(CMAKE_EXE_LINKER_FLAGS "-std=gnu99 -static -static-libgcc -static-libstdc++ -mwindows")
else()
	list(APPEND EXTRA_LIBS
		"-lGL -lGLU -lX11"
	)
endif()

list (APPEND SOURCES
			src/main.cpp
			src/load_frame.cpp
			src/video_reader.cpp
			src/video_reader.hpp
)
add_executable(Project2 ${SOURCES})
target_link_libraries(Project2 glfw ${EXTRA_LIBS} ${OPENGL_gl_LIBRARY})
find_package(OpenGL REQUIRED)
