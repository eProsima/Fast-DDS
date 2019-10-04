To launch this test open two different consoles:

In the first one launch: TypeLookupExample publisher
(or TypeLookupExample.exe publisher on windows).
In the second one: TypeLookupExample subscriber.

In this example, the publisher loads a type from the XML file "example_type.xml".
The publisher shares the TypeInformation so other participants can discover it.

After retrieving the type using the TypeLookup service,
the subscriber will discover the type and introspecting it will show the received messages from the publisher.

The type defined in the XML file can be modified to see how the subscriber notices the changes automatically,
but take into account that the publisher should be modified to understand the changes.

For this example, the root type must be a structure.
