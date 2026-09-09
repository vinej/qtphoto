cmake_minimum_required(VERSION 3.21)
project(qtphoto VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(qtphoto
    src/main.cpp
    src/Visionneuse.cpp  src/Visionneuse.h
    src/ChoixDossier.cpp src/ChoixDossier.h
)
target_link_libraries(qtphoto PRIVATE Qt6::Widgets)
target_compile_options(qtphoto PRIVATE -Wall -Wextra)

include(GNUInstallDirs)
install(TARGETS qtphoto RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES qtphoto.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
