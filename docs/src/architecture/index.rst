SKA PST COMMON Architecture
===========================

Classes
-------

A SegmentProducer represents a continuous stream of data, weights, and scale factors from some source (file, network, ring buffer, simulator, etc.), composed of multiple contiguous segments.

A Segment is a time-bound part of a Stream, composed of multiple contiguous heaps.

A Heap is a time-bound part of a Segment, composed of multiple packets.

A Packet is a frequency-bound part of a Heap.

In some sources, such as files and ring buffers, the content of each heap is divided into separate files/buffers for data and weights+scales.

A Block is one part of a Heap, either data or weights+scales.

The following diagram shows the classes involved in the definition and implementation of a stream.

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
