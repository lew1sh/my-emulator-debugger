add_library(emulator
    src/emulator.cpp
)

target_include_directories(emulator
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(emulator_test
    main.cpp
)

target_link_libraries(emulator_test
    PRIVATE emulator
)
