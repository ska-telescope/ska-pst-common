SKA PST COMMON Architecture
===========================

Classes
-------

A Stream represents a continuous stream of data, weights, and scale factors from some source (file, network, ring buffer, simulator, etc.), composed of multiple slices.

A Slice is a time-limited segment of a Stream, composed of multiple heaps.

A Heap is a time-limited segment of a Slice, composed of multiple packets.

A Packet is a time- and frequency-limited segment of a Heap.

The following diagram shows the classes involved in the definition and implementation of a stream.

.. uml:: stream_class_diagram.puml
  :caption: Class diagram showing main classes involved in the definition of a stream

PacketLayout
^^^^^^^^^^^^

Describes the layout of a packet in memory, including the offsets and sizes of data, weights, and scales.
The contents of a packet can be contained in either a single contiguous block of memory (e.g. a UDP packet), 
or distributed over different blocks of memory (e.g. data and weights blocks in shared memory).

HeapLayout
^^^^^^^^^^

Describes the layout of a heap in memory.