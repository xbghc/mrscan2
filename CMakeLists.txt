cmake_minimum_required(VERSION 3.16)

project(mrscan2 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FFTW3_DIR "D:/tools/vcpkg/installed/x64-windows/share/fftw3")
find_package(FFTW3 CONFIG REQUIRED)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools PrintSupport)

set(TS_FILES mrscan2_zh_CN.ts)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(mrscan2
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        examtab.h examtab.cpp examtab.ui
        historytab.h historytab.cpp historytab.ui
        preferencesdialog.h preferencesdialog.cpp preferencesdialog.ui
        preferencestyle.h preferencestyle.cpp preferencestyle.ui
        preferenceimage.h preferenceimage.cpp preferenceimage.ui
        tuningcentralfrequency.h tuningcentralfrequency.cpp tuningcentralfrequency.ui
        tuningradiofrequencypower.h tuningradiofrequencypower.cpp tuningradiofrequencypower.ui
        tuningshimming.h tuningshimming.cpp tuningshimming.ui
        patient.h patient.cpp
        resources.qrc
        patientinfodialog.h patientinfodialog.cpp patientinfodialog.ui

        configs/sequences.json
        configs/exams.json
        exameditdialog.h exameditdialog.cpp exameditdialog.ui
        scanneradapter.h scanneradapter.cpp
        virtualscanner.h virtualscanner.cpp
        sequenceencoder.h sequenceencoder.cpp
        exammodel.h exammodel.cpp
        sequencevalidator.h sequencevalidator.cpp
        configs/tunings.json

        historymodel.h historymodel.cpp

        examhistory.h examhistory.cpp
        qcustomplot.cpp qcustomplot.h
        configs/preferences.json
        custompreferences.h custompreferences.cpp
        abstractpreferencestab.h abstractpreferencestab.cpp

        QImagesWidget/components/qcheckcombobox.cpp QImagesWidget/components/qcheckcombobox.h QImagesWidget/components/qcheckcombobox.qrc QImagesWidget/components/qcheckcombobox.svg
        QImagesWidget/contentwidget.cpp QImagesWidget/contentwidget.h QImagesWidget/icons.qrc QImagesWidget/mrdparser.cpp QImagesWidget/mrdparser.h QImagesWidget/qimageswidget.cpp QImagesWidget/qimageswidget.h QImagesWidget/qimageswidget.ui QImagesWidget/utils.cpp QImagesWidget/utils.h
        scoutwidget.h scoutwidget.cpp


    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET mrscan2 APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(mrscan2 SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(mrscan2
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_include_directories(mrscan2 PUBLIC
    ${FFTW3_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/QImagesWidget
    ${CMAKE_CURRENT_SOURCE_DIR}/QImagesWidget/components
)
target_link_libraries(mrscan2 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::PrintSupport FFTW3::fftw3)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.mrscan2)
endif()
set_target_properties(mrscan2 PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS mrscan2
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(mrscan2)
endif()
