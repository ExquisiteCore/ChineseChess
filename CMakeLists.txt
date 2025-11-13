cmake_minimum_required(VERSION 3.16)

project(ChineseChess VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Windows下设置UTF-8编码
if(WIN32)
    if(MSVC)
        add_compile_options(/utf-8)
    else()
        # MinGW/GCC
        add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Quick Concurrent Multimedia)

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
    # AI引擎（模块化 + 增强功能）
    src/ai/TranspositionTable.h
    src/ai/TranspositionTable.cpp
    src/ai/Evaluator.h
    src/ai/Evaluator.cpp
    src/ai/MoveOrderer.h
    src/ai/MoveOrderer.cpp
    src/ai/SearchEngine.h
    src/ai/SearchEngine.cpp
    src/ai/OpeningBook.h
    src/ai/OpeningBook.cpp
    src/ai/EndgameTablebase.h
    src/ai/EndgameTablebase.cpp
    src/ai/ChessAI.h
    src/ai/ChessAI.cpp
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
        qml/SoundManager.qml
)

set_target_properties(appChineseChess PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appChineseChess
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appChineseChess
    PRIVATE Qt6::Quick Qt6::Concurrent Qt6::Multimedia
)

include(GNUInstallDirs)
install(TARGETS appChineseChess
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
