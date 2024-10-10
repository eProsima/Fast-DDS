# Flow control example

The *eProsima Fast DDS flow control* example is a simple application intended to demonstrate the use
of Flow Controllers.

This example is part of the suite of examples designed by eProsima that aims to illustrate the features
and possible configurations of DDS deployments through *eProsima Fast DDS*.

* [Description of the example](#description-of-the-example)
* [Run the example](#run-the-example)

## Description of the example

eProsima Fast DDS provides a mechanism to control the data flow sent by a DataWriter.
The Flow Control is implemented through objects called Flow Controllers.
These controllers are registered on the creation of the DomainParticipant using a [Flow Controller Descriptor](https://fast-dds.docs.eprosima.com/en/latest/fastdds/api_reference/rtps/flowcontrol/FlowControllerDescriptor.html#flowcontrollerdescriptor), and then referenced on the creation of the DataWriter using Publish Mode Qos Policy.

A Flow Controller Descriptor is a simple struct that univocally defines a flow controller.
It includes the following [FlowControllersQos](https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/core/policy/eprosimaExtensions.html#flowcontrollersqos) settings and configurations:
* Name of the flow controller.
* Scheduler policy used by the flow controller.
* Maximum number of bytes to be sent to network per period.
* Period of time in milliseconds on which the flow controller is allowed to send the maximum number of bytes per period.
* Thread settings for the sender thread.

When using Flow Controllers, the DataWriter may need specific parameters related to the priority and the bandwith.
For more information, please refer to [Flow Controller Settings](https://fast-dds.docs.eprosima.com/en/latest/fastdds/property_policies/flow_control.html#flow-controller-settings).
* Property `fastdds.sfc.priority` is used to set the priority of the DataWriter for `HIGH_PRIORITY` and `PRIORITY_WITH_RESERVATION` flow controllers. Allowed values are from -10 (highest priority) to 10 (lowest priority). The default value is the lowest priority.
* Property `fastdds.sfc.bandwidth_reservation` is used to set the percentage of the bandwidth that the DataWriter is requesting for `PRIORITY_WITH_RESERVATION` flow controllers. Allowed values are from 0 to 100, and express a percentage of the total flow controller limit. By default, no bandwidth is reserved for the DataWriter.

Once instantiated, a flow controller will make sure there is a limit on the data it processes, so that no more than the specified size gets through it in the specified time.

In this example, the Fast DataWriter has no flow controller, while the Slow DataWriter has a flow controller limiting the maximum number of bytes to be sent per period, as well as the period of time on which the DataWriter is allowed to send.

The information regarding the kind of a DataWriter, whether it is a slow or a fast one, is communicated through the user data field during the discovery phase. This user data is embedded within the discovery protocol, allowing other participants in the network to shared information without requiring direct communication or configuration.
For more information, please refer to [UserDataQosPolicy](https://fast-dds.docs.eprosima.com/en/latest/fastdds/api_reference/dds_pim/core/policy/userdataqospolicy.html#userdataqospolicy).

## Run the example

To launch this example, two different terminals are required.
One of them will run the publisher example application, and the other will run the subscriber application.

### Flow Control publisher

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./flow_control publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

* Windows

    ```powershell
    example_path> flow_control.exe publisher
    Publisher running. Please press Ctrl+C to stop the Publisher at any time.
    ```

### Flow Control subscriber

* Ubuntu ( / MacOS )

    ```shell
    user@machine:example_path$ ./flow_control subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```

* Windows

    ```powershell
    example_path> flow_control.exe subscriber
    Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
    ```


### Expected output

Regardless of which application is run first, since the publisher will not start sending data until a subscriber is discovered, the expected output both for publishers and subscribers is a first displayed message acknowledging the match, followed by the amount of samples sent or received until Ctrl+C is pressed.
The samples are sent every 2s. The Slow DataWriter sends a sample before the Fast DataWriter, but the sample is always received later according with the FlowControllerQos settings in the Flow Controller.

### Flow Control publisher

```shell
Publisher running. Please press Ctrl+C to stop the Publisher at any time.
Publisher matched.
Message SENT from SLOW WRITER, count=1
Message SENT from FAST WRITER, count=1
Message SENT from SLOW WRITER, count=2
Message SENT from FAST WRITER, count=2
Message SENT from SLOW WRITER, count=3
Message SENT from FAST WRITER, count=3
...
```

### Flow Control subscriber

```shell
Subscriber running. Please press Ctrl+C to stop the Subscriber at any time.
Subscriber matched.
Sample RECEIVED from fast writer, count=1
Sample RECEIVED from slow writer, count=1
Sample RECEIVED from fast writer, count=2
Sample RECEIVED from slow writer, count=2
Sample RECEIVED from fast writer, count=3
Sample RECEIVED from slow writer, count=3
...
```

When Ctrl+C is pressed to stop one of the applications, the other one will show the unmatched status, displaying an informative message, and it will stop sending / receiving messages.
