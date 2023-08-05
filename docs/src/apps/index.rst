
===========================
SKA PST COMMON Applications
===========================

ska_pst_common_info
-------------------

Prints the version of the ska-pst-common library to stdout.

ska_pst_generate_file
----------------------

Generates a pair of data and weights files containing a simulated signal,
in the DADA file format as output by the AA0.5 voltage recorder.

    Usage: ska_pst_generate_file [options]

    -d config     name of configuration file for output data
    -w config     name of configuration file for output weights
    -s signal     name of signal generator (Random, Sine, GaussianNoise)
    -T seconds    duration of simulated signal (default: 10)
    -h            print this help text
    -o            use O_DIRECT for writing file output
    -v            verbose output

Example: generate 0.1 seconds of data containing normally distributed GaussianNoise

    ska_pst_generate_file -d data_config.txt -w weights_config.txt -s GaussianNoise -T 0.1

The example data_config.txt

    HDR_SIZE      4096
    NPOL          2
    NDIM          2
    NCHAN         20736
    NBIT          16

    RESOLUTION              5308416

    UDP_NSAMP               32
    WT_NSAMP                32
    UDP_NCHAN               24

    UTC_START           2023-08-05-11:07:09

    FREQ                1000
    BW                  800    # MHz
    TSAMP               25.92  # microseconds

The example weights_config.txt

    HDR_SIZE      4096
    NPOL          1
    NDIM          1
    NCHAN         20736
    NBIT          16

    RESOLUTION              44928
    PACKET_SCALES_SIZE      4
    PACKET_WEIGHTS_SIZE     48

    UDP_NSAMP               32
    WT_NSAMP                32
    UDP_NCHAN               24