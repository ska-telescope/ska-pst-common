
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
    FileReader.h
    FileWriter.h
    GaussianNoiseGenerator.h
    NormalSequence.h
    RandomDataGenerator.h
    Logging.h
    SineWaveGenerator.h
    UniformSequence.h
    ValidationContext.h
)

set(sources
    src/AsciiHeader.cpp
    src/Time.cpp
    src/Timer.cpp
    src/NormalSequence.cpp
    src/RandomSequence.cpp
    src/DataGenerator.cpp
    src/DataGeneratorFactory.cpp
    src/DataUnpacker.cpp
    src/FileReader.cpp
    src/FileWriter.cpp
    src/GaussianNoiseGenerator.cpp
    src/RandomDataGenerator.cpp
    src/Logging.cpp
    src/SineWaveGenerator.cpp
    src/UniformSequence.cpp
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
    $<TARGET_OBJECTS:ska_pst_lmc_generated>
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
    ska_pst_lmc_generated
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
