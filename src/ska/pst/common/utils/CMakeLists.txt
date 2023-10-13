
set(public_headers
    AsciiHeader.h
    BlockProducer.h
    BlockSegmentProducer.h
    DataUnpacker.h
    Endian.h
    FileBlockProducer.h
    FileReader.h
    FileSegmentProducer.h
    FileWriter.h
    GaussianNoiseGenerator.h
    HeapLayout.h
    Logging.h
    NormalSequence.h
    PacketGenerator.h
    PacketGeneratorFactory.h
    PacketLayout.h
    RandomDataGenerator.h
    RandomSequence.h
    ScaleWeightGenerator.h
    SegmentGenerator.h
    SegmentProducer.h
    SineWaveGenerator.h
    SquareWaveGenerator.h
    Time.h
    Timer.h
    UniformSequence.h
    ValidationContext.h
)

set(sources
    src/AsciiHeader.cpp
    src/BlockSegmentProducer.cpp
    src/DataUnpacker.cpp
    src/FileBlockProducer.cpp
    src/FileReader.cpp
    src/FileSegmentProducer.cpp
    src/FileWriter.cpp
    src/GaussianNoiseGenerator.cpp
    src/HeapLayout.cpp
    src/Logging.cpp
    src/NormalSequence.cpp
    src/PacketGenerator.cpp
    src/PacketGeneratorFactory.cpp
    src/RandomDataGenerator.cpp
    src/RandomSequence.cpp
    src/ScaleWeightGenerator.cpp
    src/SegmentGenerator.cpp
    src/SineWaveGenerator.cpp
    src/SquareWaveGenerator.cpp
    src/Time.cpp
    src/Timer.cpp
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
