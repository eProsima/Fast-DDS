--------------------------------------------------------
- Use Case Publisher-Subscriber for eProsima Fast DDS -
--------------------------------------------------------

1 - Application description
---------------------------

eProsima Fast DDS provides users with a wide range of configuration options. This example has the objective of providing a testing ground where you can experiment and see the influence different combinations of parameters can have on the behaviours on the Publisher/Subscriber scheme.

This example consists on two application, a configurable  publisher and a subscriber for you to run your own tests:

- With the Publisher, you can choose to send any number of samples at any given moment. Each time you send a batch of samples, they will numbered starting from index '0', so it is easier to view the end of one batch and the start of the next on the Subscriber side.
- The subscriber passively stores samples. At any moment you can choose to view the stored samples. 
      
You can run any number of Publisher and Subscribers and use them to send a variable number of sample batches each of different size, reviewing the contents of the Subscriber to observe the effects of your configuration choices on the behaviour of the network.

2 - Configuration options
--------------------------

These are the main parameters that affect the behaviour of eProsima Fast DDS and that are used in this example:

- Reliability Kind 
    
    Defines how eProsima Fast DDS deals upon possible packet loss during data exchanges.    

    + Best Effort: No arrival confirmation. It is fast but lost samples are not re-sent.
    + Reliable: With arrival confirmation. It is slower but provides guarantee that all lost samples are re-sent and eventually received by the subscriber.

Since this application runs the Publisher and Subscriber on the same machine, data loss is highly unlikely. Therefore this option will not play a relevant role on the application behaviour.

- Durability Kind

    Defines what to do with samples that exist prior to the existence of the Subscriber.

    + Volatile: Past samples are ignored, the Subscriber starts to receive data generated after it has joined the network.
    + Transient Local: Past samples are sent.

- History Kind

    Defines the storage policy for past samples.

    + Keep Last: The History will save and give access to the last "k" received samples. This "k" number is called the History Depth and can be manually set too by the user.
    + Keep All: The History will save and give access to all received samples until the maximum samples size of the History is reached.

This parameter affects cases of "late-joining" Subscribers: Subscribers that come online after data transfers on a topic have started.

- Keys

    Keys allow to have multiple data endpoints within a topic as opposed to all data published in a topic going into the same "inbox".

    + On a topic without keys, all pieces of data go into a single endpoint.
    + On a topic without keys, the key field is used to determine which of the multiple endpoints the data goes into. If, for example, a history is set to transient local with depth=3 (the last three samples are stores) then eProsima Fast DDS will ensure that 3 samples per data endpoint are stored. This means that three samples are stored for each unique key instead of three samples total.

    It is important to note that even if you configure a Topic and your History to be able to hold items from multiple keys, this configuration does not take effect unless you explicitly enable Keys.

- Depth

    The depth is the amount of past samples that are stored in the history before starting to overwrite. Only takes effect when the History is on "Keep Last" mode.

- History Size
    
    This accounts for the total number of samples that can be stored in the history, regardless of any other configuration option. 

- Instances
    
    Instances are the different data sinks the History is divided and act as receptors of the Keys.

- Instance size

    As it happens with depth, you can define a maximum number of past samples to be stored. If you set one Instance and an instance size more restrictive than the depth, the instance size will be the limiting factor.

3. Recommended tests 
---------------------

The following examples provide sample tests you can perform to see how the influence of configuration parameters affect the behaviour of eProsima Fast DDS.

- Testing Keep-All, Keep Last and the influence of Depth.

* Select Keep-Last. Choose depth 10. Open a Publisher and write 5 samples. Then write 6 samples. Read the contents of the Subscriber.
The History will have the 6 samples from the last batch and 4 out of the 5 of the first one. The first sample is lost because it was removed to leave room for new samples.
* Select Keep-Last. Choose depth 20. Open a Publisher and write 10 samples. Then write another 10 samples. Read the contents of the Subscriber.
It will hold all 20 samples, because the number of samples written is lower or equal to the depth and not overwrite has happened yet.
* Repeat the previous tests but select Keep-All. See how the limiting factor now is the total size of the History.

-Testing a late joiner.

A late joiner is a Subscriber that joins a topic after data has been published on it.

* Select Volatile mode. Open a Publisher and post 10 samples. Start a Subscriber. Publish a sample. Read the contents of the Subscriber.
It will only hold the sample written after its creation.
* Select Transient Local mode. Open a Subscriber and post 10 samples. Start a Subscriber. Read the contents of the Subscriber.
It will hold the samples that were written before its creation.

-Testing Keys

*Turn on Keys, choose 3 keys on the Publisher and 3 instances on the Subscriber History. Choose Keep-All. Send 5 samples. Read the contents of the History.
It will containt 15 samples, 5 per key.
*Turn on keys, choose 4 keys on the Publisher and 3 instances on the Subscriber History. Choose Keep-All. Send 5 samples. Read the contents of the History.
It will contain 15 samples belonging to the 3 first unique keys that were received. The fourth key and its data is ignored.
*You can test other configuration parameters in combination with keys. Run previous test but using keys and see the effects are the same as before but in a per-key manner.

- Testing assymetric configurations

For a Publisher and a Subscriber to be able to talk, it is first necessary that they perform a matching process. For this to happen the configuration of both elements has to be compatible.

Reliability is the most common cause for two elements being unable to match. A Best-Effort mode Publisher cannot communicate with a Reliable Subscriber. If you run A Publisher and a Subscriber with these configurations, you will see that the Subscriber never receives a single sample.

- Combined cases

You can take multiple of the previous cases and combine the test subject configurations into a single execution to see how different parameters interact with each other.

    - Influence of the instance size depth of the Publisher on a Transient Local late-joining Subscriber: The most restrictive depth or instance size always applies, even it is the Publisher one.

4. Built-in tests
-----------------

In addition to this Publisher and Subscriber, the examples folder of this distribution comes with a number of preset tests to exemplify the most behaviour-changing parameters:

* HistoryKind: Shows how a Keep-All Subscriber stores all samples in its History and a Keep-Last subscriber starts to overwrite when it reaches its depth.
* LateJoiners: Shows how a Transient-Local Subscriber receives past samples while Volatile starts receiving from the moment of its creation.
* Keys: Provides a working example of how data can be split into multiple endpoints based of keys.
* SampleConfig: Provides a basic Publish-Subscribe example for the three sample configurations specified in section 5.

5. Application examples
----------------------

The following list provides examples configurations for real-life scenarios, as shown in the SampleConfig examples provided with this distribution.

- Multimedia feed


Audio and Video transmission have a common characteristic: Having a stable, high datarate feed is more important than having a 100% lossless transmission.

	Reliability: Best-Effort. We want to have a fast transmission. If a sample is lost, it can be recovered via error-correcting algorithms.
	Durability: Volatile. We do not mind data from the past, we want to stream what is happening in the present.
	History: Keep-Last with Low Depth. Once displayed or recorded on the receiving application, they are not needed in the History.
	note: In the case of video, depth can be as low as 1. A missing sample of a 50 frames per second stream represents virtually no information loss. 

- Distributed measurement

Lets say we have a factory with a network of distributed temperature sensors and we want to use eProsima Fast DDS to send data from said sensors to an automation computer which
makes decisions based on the temperature distribution. We would group all sensors within one single topic, 

	Reliability: Reliable. We want to make sure samples are not lost. Furthermore, since reliable more ensures data delivery, it allows us to detect hardware problems when a sensor becomes silent
	Keys: We use a separate key for each sensor. That way new messages from one sensor dont affect the storage of past samples from another sensor.
	Durability: Transient Local. We dont want to lose samples due a sensor losing connection temporarily.
	History: Keep-Last with High Depth. We want to have access to past samples in order to be able to compute delta values.

- Event-based transmission

In some cases, you may want to transmit information only under certain circumstances that happen in your system, for example a photo from a surveillance camera when it detects movement.
In these cases it is important that all datagrams reach their destination
		
	Reliability: Reliable. All samples must reach their destination.
	Durability: Volatile. Since corrective actions are taken as events come, past triggers have no use.
	History: Keep-Last. No past alarm information is necessary for present-time transmissions.
	Additional Settings: Reduce heartbeat period, which dictates system response velocity when a sample is lost. A lower heartbeat period equals fast response on data delivery.

6. Run example
----------------------
To launch this test open two different consoles:

In the first one launch: './UseCasePublisher' (or UseCasePublisher.exe on windows).
In the second one: './UseCaseSubscriber'

Select the configuration.