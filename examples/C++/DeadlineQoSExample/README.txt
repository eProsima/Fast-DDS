-----------------------------
|   USER DEADLINE EXAMPLE   |
-----------------------------

-------------------
-     Purpose     -
-------------------

This example shows a way to implement a Deadline QoS feature on a FastRTPS Application,
taking an example project as a starting point.

Deadline is a subscriber-based QoS provides an alarm when the period in which data is received 
doesntmeet the configured requirements (i.e: data arrival is slower than expected).
When a topic has a key, each piece of data with different key is treated as a different
data source/sink and therefore a Deadline alarm is set off independently for each
key.
 
-------------------
-Working principle-
-------------------

The Deadline QoS is configured with a minimun data arrival frequency. In every period, a
check is performed to determine if new data has arrived for every key since the last update.
In the case a key does not have new data, the alarm for that key goes off.

This check is run with the same periodicity as the expected maximun period between samples.
Each key has a flag bit assigned to it, that becomes set to '1' every time a piece of data
with that key arrives. The Deadline check verifies the status of these flags and resets them
to '0' upon exit, meaning that, upon the next check, a '1' flag means "data received in this period"
and a '0' means "no new data". The alarm consists on a user-defined callback function that is called
when a '0' flag is found during the check.

--------------------
-  Implementation  -
--------------------

In this example, the Deadline QoS is implemented by the use of a helper class
DeadlineQoS. This class implements:
- A map of flag bits for each key (with mutex to ensure exclusive access).
- A method to be called from onNewDataMessage to set flags on the QoS map based on the keys.
- A predefined callback function that performs the QoS checks periodically.
- A function to activate the process.
- A deadline-missed callback function.

-------------------
-      USAGE      -
-------------------

To make use of the provided DeadlineQoS implementation...
- Create an instance of deadlineQoS inside your subscriber's listener.
- Activate the service by calling deadlineQoS::run()
- Use eadlineQoS::setFlag(key) to update the flag list on deadlineQoS on new data arrival.
- Specify the desired functionality on Deadline miss in deadlineQoS::callback()

It is important to configure the topic to distinguish between keys, otherwise the @key parameter
specified in the IDL will be ignored.
