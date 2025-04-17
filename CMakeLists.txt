cmake_minimum_required(VERSION 3.16)

project(mrscan2 VERSION 0.1 LANGUAGES CXX)

# 设置自动化工具
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找FFTW3库
set(FFTW3_DIR "D:/tools/vcpkg/installed/x64-windows/share/fftw3")
find_package(FFTW3 CONFIG REQUIRED)

# 查找Qt库
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools PrintSupport)

# 翻译文件
set(TS_FILES mrscan2_zh_CN.ts)

# 核心源文件
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
        
        # 界面相关文件
        examtab.h examtab.cpp examtab.ui
        historytab.h historytab.cpp historytab.ui
        preferencesdialog.h preferencesdialog.cpp preferencesdialog.ui
        preferencestyle.h preferencestyle.cpp preferencestyle.ui
        preferenceimage.h preferenceimage.cpp preferenceimage.ui
        tuningcentralfrequency.h tuningcentralfrequency.cpp tuningcentralfrequency.ui
        tuningradiofrequencypower.h tuningradiofrequencypower.cpp tuningradiofrequencypower.ui
        tuningshimming.h tuningshimming.cpp tuningshimming.ui
        patientinfodialog.h patientinfodialog.cpp patientinfodialog.ui
        exameditdialog.h exameditdialog.cpp exameditdialog.ui
        resultwidget.h resultwidget.cpp resultwidget.ui
        
        # 模型相关文件
        patient.h patient.cpp
        exammodel.h exammodel.cpp
        examitem.h examitem.cpp
        historymodel.h historymodel.cpp
        examhistory.h examhistory.cpp
        
        # 扫描仪相关文件
        scanneradapter.h scanneradapter.cpp
        virtualscanner.h virtualscanner.cpp
        sequenceencoder.h sequenceencoder.cpp
        sequencevalidator.h sequencevalidator.cpp
        
        # 配置和工具类
        configmanager.h configmanager.cpp
        custompreferences.h custompreferences.cpp
        abstractpreferencestab.h abstractpreferencestab.cpp
        utils.cpp utils.h
        mrdparser.cpp mrdparser.h
        
        # 图像处理相关
        qcustomplot.cpp qcustomplot.h
        scoutwidget.h scoutwidget.cpp
        QImagesWidget/qimageswidget.cpp QImagesWidget/qimageswidget.h
        components/qcheckcombobox.cpp components/qcheckcombobox.h components/qcheckcombobox.qrc components/qcheckcombobox.svg
        
        # 资源文件
        resources.qrc
        icons.qrc
        
        # 配置文件
        configs/sequences.json
        configs/exams.json
        configs/tunings.json
        configs/preferences.json
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

# 添加包含目录
target_include_directories(mrscan2 PUBLIC
    ${FFTW3_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/QImagesWidget
    ${CMAKE_CURRENT_SOURCE_DIR}/QImagesWidget/components
)

# 链接库
target_link_libraries(mrscan2 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt6::PrintSupport FFTW3::fftw3)

# 设置包属性
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

# 安装目标
include(GNUInstallDirs)
install(TARGETS mrscan2
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(mrscan2)
endif()
