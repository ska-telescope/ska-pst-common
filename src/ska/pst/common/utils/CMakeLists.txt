
set(public_headers
    AsciiHeader.h
    Endian.h
    Time.h
    Timer.h
    RandomSequence.h
    DataLayout.h
    DataGenerator.h
    DataGeneratorFactory.h
    DataUnpacker.h
    RandomDataGenerator.h
    Logging.h
    SineWaveGenerator.h
    ValidationContext.h
)

set(sources
    src/AsciiHeader.cpp
    src/Time.cpp
    src/Timer.cpp
    src/RandomSequence.cpp
    src/DataGenerator.cpp
    src/DataGeneratorFactory.cpp
    src/DataUnpacker.cpp
    src/RandomDataGenerator.cpp
    src/Logging.cpp
    src/SineWaveGenerator.cpp
    src/ValidationContext.cpp
)

set(private_headers
)

add_library(
    ska_pst_common-utils
    OBJECT
    ${sources}
    ${private_headers}
    ${public_headers}
)

target_include_directories(
    ska_pst_common-utils
    PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(
    ska_pst_common-utils
    PUBLIC
    spdlog::spdlog
)

if (BUILD_TESTING)
  add_subdirectory(tests)
endif()

install(
    FILES
        ${public_headers}
    DESTINATION
        include/ska/pst/common/utils
)
