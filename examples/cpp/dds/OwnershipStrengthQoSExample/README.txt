To launch this test open two or more consoles:

1) "$./DDSOwnershipStrengthQoSExample subscriber" 
    (or "DDSOwnershipStrengthQoSExample.exe subscriber" on Windows). 

2..*) "$./DDSOwnershipStrengthQoSExample publisher XX" 
      (or "DDSOwnershipStrengthQoSExample.exe publisher XX" on Windows).

For the publishers, XX represents an Ownership Strength QoS number. 
If left unspecified it will default to 10.

This example illustrates a user-level implementation of the Ownership Strength 
quality of service. Inbound messages will be filtered based on their ownership 
strength, so only one publisher will be allowed per topic until unmatched. The 
behaviour is implemented as follows:

                            ===============
                            == BEHAVIOUR ==
                            ===============

The IDL file (OwnershipStrength.idl) defines a message type with an 
ownership strength field. This field will be read by the subscriber and used
to decide whether or not a message must be processed at any given time.

OwnershipStrengthPublisher incorporates a method to define its ownership strength.
This value will be propagated to any messages sent from this publisher. The
ability to set this value is exposed through the command line as mentioned
above.

OwnershipStrengthSubscriber presents the most significant change from a default
example. Its SubscriberListener includes a field of type StrengthHierarchy. 
StrengthHierarchy encapsulates a map of publishers classified by their strength,
and offers a simple API encompassing two functionalities:
   * Checking whether a particular message belongs to the publisher currently 
     considered strongest (while implicitly registering said publisher on the 
     hierarchy).
   * Explicitly removing a particular publisher from the hierarchy.

Whenever the subscriber receives a message, it will call "IsMessageStrong" on it,
automatically starting to keep track of the publisher that originally sent it.
This and subsequent calls to IsMessageStrong will return true if the publisher
associated with the message has the highest strength value among all registered
publishers, or in case of a tie, the lowest GUID. This return value is used to 
choose whether to process the message.

The listener is responsible of explicitly removing a publisher from the hierarchy 
during the unmatching process, through a call to the "DeregisterPublisher" method. 
Other criteria for de-registration may also be included if desired, for example
on a lost deadline. 

Internally, the strength hierarchy makes use of the ordering properties of std::map
to provide quick access to the maximum strength value, and is made thread-safe by
a C++11 mutex.
