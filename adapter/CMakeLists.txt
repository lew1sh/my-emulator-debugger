add_executable(debug_adapter
    src/debug_adapter.cpp
)

target_include_directories(debug_adapter
    PRIVATE
        ${CMAKE_SOURCE_DIR}/emulator/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
    
target_link_libraries(debug_adapter
    PRIVATE emulator
)
