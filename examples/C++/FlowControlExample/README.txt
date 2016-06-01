To launch this example open two consoles:

 1) "$ FlowControlExample subscriber" (or "FlowControlExample.exe subscriber" in Windows). 

 2..*) "$ FlowControlExample publisher" (or "FlowControlExample.exe publisher XX" in Windows).


This example illustrates the flow control feature. 

                              ================
                              = Flow Control =
                              ================

In FastRTPS, Flow Control is implemented through objects called Flow Filters. In 
particular, we will be looking at the simplest kind, the Size Filter.

A size filter is univocally defined by a Size Filter Descriptor, which is a simple
struct that includes two values:
   -> A size in bytes.
   -> A period in milliseconds.

Once instantiated from this descriptor, a size filter will make sure there is a
limit on the data it processes, so that no more than the specified size gets 
through it in the specified time. In other words, it limits data throughput.

Size filters can be placed at different points in the system. In this example, you
can see a size filter being placed on a Participant, and another on a particular
Writer. Filters allocated in this way display a hierarchical behaviour, so in order
for data to be sent, it must clear both the Participant filter and the Writer filter,
if available.

Looking at FlowControlExamplePublisher::init(), you can see the steps involved in 
adding a size filter to the participant parameters and the publisher parameters, 
respectively.

