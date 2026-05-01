cmake_minimum_required(VERSION 3.10)
project(BulletHell)

set(CMAKE_CXX_STANDARD 17)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(bullethell
    main.cpp
    Entity.cpp
    Player.cpp
    Enemy.cpp
    Bullet.cpp
)

target_link_libraries(bullethell sfml-graphics sfml-window sfml-system)