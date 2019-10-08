To launch this test open two different consoles:

In the first one launch: DDSDynamicHelloWorldExample publisher
(or DDSDynamicHelloWorldExample.exe publisher on windows).
In the second one: DDSDynamicHelloWorldExample subscriber.

In this example, the publisher loads a type from the XML file "example_type.xml".
The publisher shares the TypeObject so another participants can discover it.

The subscriber will discover the type and introspecting it will show the received messages from the publisher.

For simplicity the subscriber doesn't parses wstring, union, sequences or arrays.

The type defined in the XML file can be modified to see how the subscriber notices the changes automatically,
but take into account that the publisher should be modified to understand the changes.

For this example, the root type must be an structure.
