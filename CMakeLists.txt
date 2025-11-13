cmake_minimum_required(VERSION 3.16)
project(Hangman C)

set(CMAKE_C_STANDARD 11)

find_package(SDL2 CONFIG REQUIRED)

find_package(SDL2_image CONFIG REQUIRED)

find_package(glad CONFIG REQUIRED)

find_package(SDL2_ttf CONFIG REQUIRED)

add_executable(Hangman
        main.c
        game/hangman.h
        game/hangman.c
        utility/utilities.h
        utility/utilities.c
        resources/app_icon.rc
        screens/main_menu.c
        screens/main_menu.h
        screens/about_section.c
        screens/about_section.h
)

target_link_libraries(Hangman
        PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
        $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
        $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
        glad::glad
)