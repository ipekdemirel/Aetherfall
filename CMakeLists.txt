cmake_minimum_required(VERSION 3.20)

project(Aetherfall
    VERSION 0.1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(raylib)

add_executable(Aetherfall
    src/main.cpp

    src/Player.cpp
    src/Player.h

    src/Weapon.cpp
    src/Weapon.h

    src/Enemy.cpp
    src/Enemy.h

    src/EnemyManager.cpp
    src/EnemyManager.h

    src/CombatSystem.cpp
    src/CombatSystem.h

    src/KeyItem.cpp
    src/KeyItem.h

    src/UI.cpp
    src/UI.h

    src/GameState.h

    src/CoinManager.cpp
    src/CoinManager.h

    src/Coin.cpp
    src/Coin.h

    src/Shop.cpp
    src/Shop.h
 "src/ExitDoor.h" "src/ExitDoor.cpp" "src/ScoreManager.h" "src/ScoreManager.cpp" "src/Coin.h" "src/Coin.cpp" "src/CoinManager.h" "src/CoinManager.cpp" "src/Shop.h" "src/Shop.cpp")

target_include_directories(Aetherfall
    PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(Aetherfall
    PRIVATE
    raylib
)

if (MSVC)
    target_compile_options(
        Aetherfall
        PRIVATE
        /W4
    )
else()
    target_compile_options(
        Aetherfall
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

set_target_properties(
    Aetherfall
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

foreach(CONFIG Debug Release RelWithDebInfo MinSizeRel)
    string(
        TOUPPER
        "${CONFIG}"
        CONFIG_UPPER
    )

    set_target_properties(
        Aetherfall
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER}
        "${CMAKE_BINARY_DIR}/bin/${CONFIG}"
    )
endforeach()

add_custom_command(
    TARGET Aetherfall
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:Aetherfall>/assets"
)