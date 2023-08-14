SKA PST COMMON Architecture
===========================

Classes
-------

A SegmentProducer represents a continuous time series of data, weights, and scale factors from some source (file, network, ring buffer, simulator, etc.), composed of a series of temporally-contiguous segments.

A Segment is a time-bound part of a Stream, composed of a series of temporally-contiguous heaps.

A Heap is a time-bound part of a Segment, composed of a set of spectrally-contiguous packets.  The packets in a heap span the entire bandwidth of the signal that is being processed,
and all packets in a heap span the same time.

A Packet is a frequency-bound part of a Heap.  Each packet is composed of a set of spectrally-contiguous channels, each containing a temporally-contiguous set of time samples.

In some sources, such as files and ring buffers, the content of each heap is divided into separate files/buffers for data and weights+scales.

A Block is one part of a Heap, either data or weights+scales.

The following diagram shows the classes involved in the definition and implementation of a time series.

.. uml:: segment_producer_class_diagram.puml
  :caption: Class diagram showing main classes involved in the definition of a stream

PacketLayout
^^^^^^^^^^^^

Describes the layout of a packet in memory, including the offsets and sizes of data, weights, and scales.
The contents of a packet can be contained in either a single contiguous block of memory (e.g. a UDP packet), 
or distributed over different blocks of memory (e.g. data and weights blocks in shared memory).

HeapLayout
^^^^^^^^^^

Describes the layout of a heap in memory.
