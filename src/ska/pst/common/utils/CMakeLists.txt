
set(public_headers
    AsciiHeader.h
    BlockLoader.h
    DataBlockGenerator.h
    DataBlockLoader.h
    FileSegmentProducer.h
    Endian.h
    Time.h
    Timer.h
    RandomSequence.h
    PacketLayout.h
    HeapLayout.h
    PacketGenerator.h
    PacketGeneratorFactory.h
    DataUnpacker.h
    FileBlockLoader.h
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
    src/DataBlockGenerator.cpp
    src/DataBlockLoader.cpp
    src/FileSegmentProducer.cpp
    src/Time.cpp
    src/Timer.cpp
    src/NormalSequence.cpp
    src/RandomSequence.cpp
    src/HeapLayout.cpp
    src/PacketGenerator.cpp
    src/PacketGeneratorFactory.cpp
    src/DataUnpacker.cpp
    src/FileBlockLoader.cpp
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
