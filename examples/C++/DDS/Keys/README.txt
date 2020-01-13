-------------------------------------------------
- Use Case Example: Keys for eProsima Fast RTPS -
-------------------------------------------------

1 - Application description
---------------------------

eProsima Fast RTPS provides users with a wide range of configuration options. This example has the objective of providing a testing ground where you can experiment and see the influence different combinations of parameters can have on the behaviours on the Publisher/Subscriber scheme.

This example is a supplement to the UseCaseLauncher example, consisting on an application which ilustrates how keys work.

2 - Configuration options
--------------------------

These are the main parameters that affect the behaviour of eProsima Fast RTPS and that are used in the Use Case set of example:

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

    + Keep Last: The History will save and give access to the alst "k" received samples. This "k" number is called the History Depth and can be manually set too by the user.
    + Keep All: The History will save and give access to all received samples until the maximum samples size of the History is reached.

This parameter affects cases of "late-joining" Subscribers: Subscribers that come online after data transfers on a topic have started.

- Keys

    Keys allow to have multiple data endpoints within a topic as opposed to all data published in a topic going into the same "inbox".

    + On a topic without keys, all pieces of data go into a single endpoint.
    + On a topic without keys, the key field is used to determine which of the multiple endpoints the data goes into. If, for example, a history is set to transient local with depth=3 (the last three samples are stores) then eProsima FastRTPS will ensure that 3 samples per data endpoint are stored. This means that three samples are stored for each unique key instead of three samples total.

    It is important to note that even if you configure a Topic and your History to be able to hold items from multiple keys, this configuration does not take effect unless you explicitly enable Keys.

- Depth

    The depth is the amount of past samples that are stored in the history before starting to overwrite. Only takes effect when the History is on "Keep Last" mode.

- History Size
    
    This accounts for the total number of samples that can be stored in the history, regardless of any other configuration option. 

- Instances
    
    Instances are the different data sinks the History is divided and act as receptors of the Keys.

- Instance size

    As it happens with depth, you can define a maximun number of past samples to be stored. If you set one Instance and an instance size more restrictive than the depth, the instance size will be the limiting factor.

3. Application behaviour
------------------------

This application spawns a Publisher and a Subscriber set up to use 5 key. The Publisher posts 10 samples on each key and then 10 more on key number 3. After eximining the contents of the history you will see how each key has had a separate treatment: samples from a key are not overwritten by samples belonging to another key, even when they belong to the same topic.

4. Built-in tests
-----------------

Other than this application, he Use Case set of examples contains the following pre-defined demonstrators:

* Historykind: Shows how a Keep-All Subscriber stores all samples in its History and a Keep-Last subscriber starts to overwrite when it reacher its depth.
* Latejoiners: Shows how a Transient-Local Subscriber receives past samples while Volatile starts receiving from the moment of its creation.
* SampleConfig: Provides a basic Publish-Subscribe example for the three sample configurations specified in section 5.

