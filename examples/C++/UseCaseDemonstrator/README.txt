------------------------------------------------
- Use Case Demonstrator for eProsima Fast RTPS -
------------------------------------------------

1 - Application description
---------------------------

eProsima Fast RTPS provides users with a wide range of configuration options which can be daunting at first. This example has the objective of providing a testing ground where you can experiment and see the influence different combinations of parameters can have on the behaviours of the Publisher/Subscriber scheme.

This example consist of two applications, one to spawn a configurable Publisher and the other a configurable Subscriber. The configuration is selected during program startup, 

- With the Publisher, you can choose to send any number of samples at any given moment. Each time you send a batch of samples, they will numbered starting from index '0', so it is easier to view the end of one batch and the start of the next on the Subscriber side.
- The subscriber passively stores samples. At any moment you can choose to view the stored samples. 
      
You can run any number of Publisher and Subscribers and use them to send a variable number of sample batches each of different size, reviewing the contents of the Subscriber to observe the effects of your configuration choices on the behaviour of the network.

2 - Configuration options
--------------------------

You will be prompted for multiple configuration parameters during startup:

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

3 - Using the application to verify the configuration 
-----------------------------------------------------
    
    One of the main reasons for providing this examples is so that you can verify what happens in particular cases you come up with while designing your application or simply studying eProsima Fast RTPS.
That being said, we recommend you to try all of the previously mentioned configuration parameters by setting up a Publisher, a Subscriber and then sending several batches of data.




You can try the following suggested experiments to test the reponse of eProsima Fast RTPS on some corner cases not normally considered during application behaviour:

- Choose an instance size lower than the actual chosen depth. See how the most restrictive number applies.
- Start a Publisher and a Subscriber with incompatible configuration. See how samples are not received by the Subscriber.
- Start a Publisher, post data, and then start a Subscriber. Note how only Transient Local allows the Subscriber to receive past samples. Experiment with the depth and History Size parameters to see how they affect the number of past samples the late-joining Subscriber receives.
- Configure keys to be used and choose Transient Local. Post enough data on each key to go over your chosen depth. Observe how the number of defined instances and their size influence the actual number of stored

4 - Limitations
---------------

Due to the current limitations of eProsima Fast RTPS, samples can be read from the History only once. Unless stored externally by your user application, read samples are lost. This means each time you query the Subscriber for samples you are resetting the status of the History. Future versions of eProsima Fast RTPS are scheduled to provide non-destructive sample acquisition functions.
