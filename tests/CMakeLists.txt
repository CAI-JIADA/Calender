cmake_minimum_required(VERSION 3.16)

find_package(Qt6 REQUIRED COMPONENTS Test)

# 測試 - 適配器
add_executable(test_adapters
    test_adapters.cpp
    ../src/core/CalendarEvent.h
    ../src/core/Task.h
    ../src/core/Platform.h
    ../src/core/CalendarManager.cpp
    ../src/core/CalendarManager.h
)

target_include_directories(test_adapters PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(test_adapters PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME test_adapters COMMAND test_adapters)

# 測試 - 搜索引擎
add_executable(test_search
    test_search.cpp
    ../src/core/CalendarEvent.h
    ../src/core/Task.h
    ../src/core/Platform.h
    ../src/core/SearchCriteria.h
    ../src/core/CalendarManager.cpp
    ../src/core/CalendarManager.h
    ../src/search/SearchEngine.cpp
    ../src/search/SearchEngine.h
)

target_include_directories(test_search PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(test_search PRIVATE
    Qt6::Core
    Qt6::Test
)

add_test(NAME test_search COMMAND test_search)
