cmake_minimum_required(VERSION 3.10)
project(BulletHell)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(bullethell
    main.cpp
    Entity.cpp
    Player.cpp
    Enemy.cpp
    Bullet.cpp
    ShooterEnemy.cpp
    PowerUp.cpp
)

target_link_libraries(bullethell sfml-graphics sfml-window sfml-system)

# ==========================================
# АВТОМАТИЧЕСКОЕ КОПИРОВАНИЕ РЕСУРСОВ
# ==========================================
# Копируем всю папку resources/ в build/resources/ при каждой сборке
add_custom_target(copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        ${CMAKE_BINARY_DIR}/resources
    COMMENT "📦 Copying resources to build directory..."
)
add_dependencies(bullethell copy_resources)