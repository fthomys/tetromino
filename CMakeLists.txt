cmake_minimum_required(VERSION 3.30)
project(Tetromino C)

set(CMAKE_C_STANDARD 11)

message(STATUS "🔍 Looking for SDL2...")
find_package(SDL2 REQUIRED)


message(STATUS "🔍 Looking for SDL2_mixer...")
find_package(SDL2_mixer REQUIRED)


message(STATUS "🔍 Looking for SDL2_ttf...")
find_package(SDL2_ttf REQUIRED)


include_directories(
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_MIXER_INCLUDE_DIRS}
        ${SDL2_TTF_INCLUDE_DIRS}
)

message(STATUS "🛠️  Building executable Tetromino...")
add_executable(Tetromino source/main.c source/piece.c)

message(STATUS "🔗 Linking with SDL2, SDL2_mixer, SDL2_ttf...")
target_link_libraries(Tetromino SDL2::SDL2 SDL2_mixer::SDL2_mixer SDL2_ttf::SDL2_ttf)

set(FONT_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/assets/Arial.ttf")
if(EXISTS "${FONT_SOURCE}")
    message(STATUS "📁 Copying font: Arial.ttf to binary directory...")
    file(COPY "${FONT_SOURCE}" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
else()
    message(WARNING "⚠️ Font Arial.ttf not found in assets/. Make sure the file exists!")
endif()

message(STATUS "✅ Build configuration complete! Ready to compile.")
