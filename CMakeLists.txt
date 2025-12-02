cmake_minimum_required(VERSION 3.16)
project(CalendarIntegration VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Qml
    Quick
    QuickControls2
    Network
    NetworkAuth
    Sql
    Concurrent
)

# Core library
set(CORE_SOURCES
    src/core/CalendarEvent.h
    src/core/Task.h
    src/core/Platform.h
    src/core/SearchCriteria.h
    src/core/CalendarManager.cpp
    src/core/CalendarManager.h
)

# Adapters
set(ADAPTER_SOURCES
    src/adapters/CalendarAdapter.h
    src/adapters/GoogleCalendarAdapter.cpp
    src/adapters/GoogleCalendarAdapter.h
    src/adapters/AppleCalendarAdapter.cpp
    src/adapters/AppleCalendarAdapter.h
    src/adapters/OutlookCalendarAdapter.cpp
    src/adapters/OutlookCalendarAdapter.h
)

# Sync module
set(SYNC_SOURCES
    src/sync/SyncManager.cpp
    src/sync/SyncManager.h
)

# Search module
set(SEARCH_SOURCES
    src/search/SearchEngine.cpp
    src/search/SearchEngine.h
)

# Storage module
set(STORAGE_SOURCES
    src/storage/DatabaseManager.cpp
    src/storage/DatabaseManager.h
)

# UI module
set(UI_SOURCES
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
)

# Resources
set(QML_RESOURCES
    resources/qml.qrc
)

add_executable(${PROJECT_NAME}
    src/main.cpp
    ${CORE_SOURCES}
    ${ADAPTER_SOURCES}
    ${SYNC_SOURCES}
    ${SEARCH_SOURCES}
    ${STORAGE_SOURCES}
    ${UI_SOURCES}
    ${QML_RESOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Network
    Qt6::NetworkAuth
    Qt6::Sql
    Qt6::Concurrent
)

# Install rules
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Tests
enable_testing()
add_subdirectory(tests)
