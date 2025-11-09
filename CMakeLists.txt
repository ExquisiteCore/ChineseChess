cmake_minimum_required(VERSION 3.16)

project(ChineseChess VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appChineseChess
    src/main.cpp
    # 核心游戏逻辑
    src/core/ChessPiece.h
    src/core/ChessPiece.cpp
    src/core/Board.h
    src/core/Board.cpp
    src/core/Position.h
    src/core/Position.cpp
    src/core/ChessRules.h
    src/core/ChessRules.cpp
    src/core/GameController.h
    src/core/GameController.cpp
    # QML 适配层
    src/model/ChessBoardModel.h
    src/model/ChessBoardModel.cpp
)

qt_add_qml_module(appChineseChess
    URI ChineseChess
    QML_FILES
        qml/Main.qml
        qml/MenuPage.qml
        qml/MenuButton.qml
        qml/GamePage.qml
        qml/ChessBoard.qml
        qml/ChessPiece.qml
)

set_target_properties(appChineseChess PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appChineseChess
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appChineseChess
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS appChineseChess
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
