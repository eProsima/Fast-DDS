------------------------------------------------
- Use Case Demonstrator for eProsima Fast RTPS -
------------------------------------------------

This example provides a scenario for users to test the influence different configuration parameters have on the behaviour of eProsima Fast RTPS.
The application promts the user for his desired parameter combination, executes the scenario and reports back with performance metrics the user can interpret.
This document provides a description of the sequence of events that make the scenario, the configuration options that can be set by the user and and explanation of how these choices affect the application.

1 - Execution Scenario Description
----------------------------------

The following steps are executed:

1 - A Publisher and a Subscriber are created and matched.
2 - The Publisher sends 20 pieces of data. The contents of the Subscriber are shown in the screen.
3 - A second Subscriber is created and matched. The contents of this late-joining Subscriber are shown in the screen.
4 - The Publisher sends 10 more pieces of data. The contests of both Subscribers are shown in the screen.

2 - Configuration options
--------------------------

You will be prompted for multiple configuration parameters at application startup:

- Reliability Kind 
    
    Defines how eProsima Fast RTPS deals upon possible packet loss during data exchanges.    

    + Best Effort: No arrival confirmation. It is fast but lost samples are not re-sent.
    + Reliable: With arrival confirmation. It is slower but provides guarantee that all lost samples are re-sent and eventually received by the subscriber.

Since this application runs the Publisher and Subscriber on the same machine, data loss is highly unlikely. Therefore this option will not play a relevant role on the application behaviour.

- Durability Kind

    Defines what to do with samples that exist prior to the existence of the Subscriber.

    + Volatile: Past samples are ignored, the Subscriber starts to receive data generated after it has joined the network.
    + Transient Local: Past samples are sent.

- History Kind

    Defines the storage policy for past samples.

    + Keep Last: The History will save and give aaces to the alst "k" received samples. THis "k" number is called the History Depth and can be manually set too by the user.
    + Keep All: The History will save and give access to all received samples until the maximum samples size of the History is reached.

- Keys

    Keys allow to have multiple data endpoints within a topic as opposed to all data published in a topic going into the same "inbox".

    + On a topic without keys, all pieces of data go into a single endpoint.
    + On a topic without keys, the key field is used to determine which of the multiple endpoints the data goes into. If, for example, a history is set to transient local with depth=3 (the last three samples are stores) then eProsima FastRTPS will ensure that 3 samples per data endpoint are stored. This means that three samples are stored for each unique key instead of three samples total.

    If Keys are selected, the application will use 3 keys. This means, when sending 20 pieces of data it will send 20 pieces of data on each key (totalling 60 samples).


For the sake of simplicity and in order to keep execution time of the application  under control, the following parameters are automatically set:

- Depth = 5. 
    
    The depth is the ammount of past samples that are stored in the history before starting to overwrite. Only takes effect when the History is on "Keep Last" mode.

- Max_Samples = 15.
    
    This accounts for the total number of samples that can be stored in the history, regardless of any other configuration option. 

3 - Influence of the configuration parameters on the behaviour of the scenario
------------------------------------------------------------------------------

The effects of the configuration are split into three blocks based on the three points the application provides feedback in during execution.

If keys are being used, you will notice the effects described below on a "per key" basis. Note that the maximum samples size of the history is the only parameter that affects all keys altogether.
- Step 2: At this point, 20 pieces of data have been received by the Subscriber.

If communications are set to Best-Effort, it is possible (although unlikely) that some message has become lost during transmission.

If the History was set to keep all, then samples have been stored until the History became full (15 samples) and then older samples started to become overwritten. This means the loss of samples 1 to 5. Now the History holds samples 5 to 20.

- Step 3: A Late-joining subscriber has come online.

If the Durability kind was set to Transient Local, samples from the 1-20 batch are sent to the new Subscriber and the contents will be printed on the screen. Otherwise, it can be seen that this new Subscriber does not hold any data.


- Step 4: 10 new pieces of data have been sent and received by both Subscribers.

The first subscriber, regarless of its configuration, will have a full history and will need to have samples overwritten to accomodate the newly received ones. 

The second subscriber may or may not hold samples depending on what happened in step 2. You can see that th



