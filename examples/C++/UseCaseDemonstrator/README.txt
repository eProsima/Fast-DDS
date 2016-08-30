------------------------------------------------
- Use Case Demonstrator for eProsima Fast RTPS -
------------------------------------------------

1 - Application description
---------------------------

eProsima Fast RTPS provides users with a wide range of configuration options which can be daunting at first. This example has the objective of providing a testing ground where you can experiment and see the influence different combinations of parameters can have on the behaviours of the Publisher/Subscriber scheme.

This example consinst of two applications, one to spawn a configurable Publisher and the other a configurable Subscriber. The configuration is selected during program startup, 

     -With the Publisher, you can choose to send any number of samples at any given moment. Each time you send a batch of samples, they will numbered starting from index '0', so it is easier to view the end of one batch and the start of the next on the Subscriber side.
     -The subscriber is constantly receiving samples and storing them internally. At any moment you can choose to view the stored samples. 
     
     
You can run any number of Publisher and Subscribers, and use them to send a variable number of sample batches each of different size, reviewing the contents of the Subscriber to observe the effects of your configuration choices on the behaviour of the network.

Due to the current limitations of eProsima Fast RTPS, you can only see each piece of data one time. So each time you print the contents of the Subscriber on the screen you are effectively resetting sample storage.


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

- Depth

    The depth is the ammount of past samples that are stored in the history before starting to overwrite. Only takes effect when the History is on "Keep Last" mode.

- History Size
    
    This accounts for the total number of samples that can be stored in the history, regardless of any other configuration option. 

- Instances
    
    Instances are the different data sinks the History is divided and act as receptors of the Keys.

- Instance size

    As it happens with depth, you can define a maximun number of past samples to be stored. If you set one Instance and an instance size more restrictive than the depth, the instance size will be the limiting factor.

3 - Influence of the configuration parameters on the behaviour of the scenario
------------------------------------------------------------------------------

    As a user you can execute any nu

