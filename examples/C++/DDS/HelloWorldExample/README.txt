To launch this test open two different consoles:

In the first one launch: HelloWorldExample publisher (or HelloWorldExample.exe publisher on windows).
In the second one: HelloWorldExample subscriber.


In this example, the subscriber side looks for an existing topic with the name provided, instead of creating
a new Topic. If the subscriber cannot found it during the established max_blocking_time the application ends.
Therefore if you launch the subscriber before launching the publisher, the application may end without creating
the datareader.


