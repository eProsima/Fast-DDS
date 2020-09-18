------------------------------------------------
- Use Case Sample Configuration: Controller for eProsima Fast DDS -
------------------------------------------------

1 - Application description
---------------------------

eProsima Fast DDS provides users with a wide range of configuration options. This example has the objective of providing a testing ground where you can experiment and see the influence different combinations of parameters can have on the behaviours on the Publisher/Subscriber scheme.

This example is a supplement to the UseCaseLauncher example, consisting on an application which illustrates the effect the different kinds of Subscriber Histories have on sample storage.

2 - Configuration options
--------------------------

These are the main parameters that affect the behaviour of eProsima Fast DDS and that are used in the Use Case set of example:

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

    + Keep Last: The History will save and give access to the alst "k" received samples. This "k" number is called the History Depth and can be manually set too by the user.
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

5. Application examples
----------------------

The following list provides examples configurations for real-life scenarios, including the present example.

- Multimedia feed

Audio and Video transmission have a common characteristic: Having a stable, high datarate feed is more important than having a 100% lossless transmission.

	Reliability: Best-Effort. We want to have a fast transmission. If a sample is lost, it can be recovered via error-correcting algorithms.
	Durability: Volatile. We do not mind data from the past, we want to stream what is happening in the present.
	History: Keep-Last with Low Depth. Once displayed or recorded on the receiving application, they are not needed in the History.
	note: In the case of video, depth can be as low as 1. A missing sample of a 50 frames per second stream represents virtually no information loss. 

- Distributed measurement: Controllers

Lets say we have a factory with a network of distributed temperature sensors and we want to use eProsima Fast RTPS to send data from said sensors to an automation computer which
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
To launch this test execute: './sampleconfig_multimedia' (or sampleconfig_multimedia.exe on windows).
