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
    ShooterEnemy.cpp
)

#создание папки и копирование файлов, дальнейшие ресурсы добавляем так же
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/resources/fonts)
file(COPY ${CMAKE_SOURCE_DIR}/resources/fonts/DejaVuSans.ttf
     DESTINATION ${CMAKE_BINARY_DIR}/resources/fonts/) 

# Опционально: пересобирать при изменении ресурсов
set(RESOURCES 
    ${CMAKE_SOURCE_DIR}/resources/fonts/DejaVuSans.ttf
)

add_custom_target(resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/resources/fonts/DejaVuSans.ttf
        ${CMAKE_BINARY_DIR}/resources/fonts/
    DEPENDS ${RESOURCES}
    COMMENT "Copying resources to build directory"
)

add_dependencies(bullethell resources)
target_link_libraries(bullethell sfml-graphics sfml-window sfml-system)