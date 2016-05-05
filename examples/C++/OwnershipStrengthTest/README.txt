To launch this test open two or more consoles:

 1) "$OwnershipTestPublisherSubscriber subscriber" (or "OwnershipTestPublisherSubscriber.exe subscriber" in Windows). 

 2..*) "$OwnershipTestPublisherSubscriber publisher XX" (or "OwnershipTestPublisherSubscriber.exe publisher XX" in Windows).

 For the publishers, XX represents an Ownership Strength QoS number. If left unspecified it will default to 10.

 This example illustrates a user-level implementation of the Ownership Strength quality of service. Inbound messages will be filtered based on their ownership strength, so only one publisher will be allowed per topic until unmatched.
