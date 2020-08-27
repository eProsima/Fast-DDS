To launch this example open two consoles:

 1) "$ ./FlowControlExample subscriber" (or "FlowControlExample.exe subscriber" in Windows). 

 2) "$ ./FlowControlExample publisher" (or "FlowControlExample.exe publisher" in Windows).

This example illustrates the flow control feature. 

                              ================
                              = Flow Control =
                              ================

In Fast DDS, Flow Control is implemented through objects called Flow Controllers. In 
particular, we will be looking at the simplest kind, the Throughput Controller.

A throughput controller is univocally defined by a Throughput Controller Descriptor,
which is a simple struct that includes two values:
   -> A size in bytes.
   -> A period in milliseconds.

Once instantiated from this descriptor, a throughput controller will make sure there is a
limit on the data it processes, so that no more than the specified size gets 
through it in the specified time. In other words, it limits data throughput.

Throughput filters can be placed at different points in the system. In this example, you
can see a controller being placed on a particular Writer. Controllers allocated in this 
way display a hierarchical behaviour, so in order for data to be sent, it must clear 
both the Participant filter and the Writer filter, if available.

Looking at FlowControlExamplePublisher::init(), you can see the steps involved in 
adding a size filter to the publisher parameters. 

