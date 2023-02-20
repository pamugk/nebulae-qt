cmake_minimum_required(VERSION 3.5)

project(unofficial-gog-client VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES unofficial-gog-client_en_US.ts)

set(QRC_FILES resources.qrc)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp mainwindow.h mainwindow.ui
        pages/allgamespage.h pages/allgamespage.cpp pages/allgamespage.ui
        pages/orderspage.h pages/orderspage.cpp pages/orderspage.ui
        pages/storepage.h pages/storepage.cpp pages/storepage.ui
        pages/wishlistpage.h pages/wishlistpage.cpp pages/wishlistpage.ui
        widgets/collapsiblearea.h widgets/collapsiblearea.cpp widgets/collapsiblearea.ui
        widgets/filtercheckbox.h widgets/filtercheckbox.cpp widgets/filtercheckbox.ui
        widgets/ordergroup.h widgets/ordergroup.cpp widgets/ordergroup.ui
        widgets/orderitem.h widgets/orderitem.cpp widgets/orderitem.ui
        widgets/storegridtile.h widgets/storegridtile.cpp widgets/storegridtile.ui
        widgets/storelistitem.h widgets/storelistitem.cpp widgets/storelistitem.ui
        widgets/wishlistitem.h widgets/wishlistitem.cpp widgets/wishlistitem.ui
        ${TS_FILES}
        ${QRC_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(unofficial-gog-client
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET unofficial-gog-client APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(unofficial-gog-client SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(unofficial-gog-client
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(unofficial-gog-client PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(unofficial-gog-client PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS unofficial-gog-client
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(unofficial-gog-client)
endif()
