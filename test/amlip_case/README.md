# AML-IP Use case issue

We encountered an issue developing AML-IP when reading data.
This test simulates our scenario to reproduce the error, as it depends on a complex series of events difficult to reproduce in a small scenario.

---

## Issue

The issue happens when a DataReader notifies that there is data available (both by `on_data_available` and by `wait_for_unread_message`) and calls `take_next_sample`.
This call can return NO_DATA (it happens very few times) even when there are data in the Reader ready to be read.

---

## Multiservice

In order to reproduce this issue in Fast DDS, we have simulated the AML-IP Multiservice communication paradigm.
This is a new protocol design to choose one and only one server available in a distributed network.
This MultiService paradigm in this test case works as follows:

* Client::send_task
  * Send a request with its own id and a unique task id
  * Client waits for a Server to reply to this message with client id, task id and the own server id
  * Client sends a message with these 3 same values indicating that that Server (the first answer received) is chosen

* Server::process_task
  * Waits for a Client to send a request
    * If request has already been targeted, skip it
  * With the request received send a message as available in `reply` with client and task id received and its own server id.
  * Wait for client to answer in `target` to choose the server
    * If this server is the one chosen, finish this call (it has been chosen)
    * Otherwise: start process again

CLIENT                  SERVER
send_task               process_task
 request    ------>
            <------     reply
 target     ------>

---

## Test

This test implements classes `ParticipantClient` and `ParticipantServer` that represents a MultiServiceClient and a MultiServiceServer.
The tests create N Clients and M Servers in N+M threads running in parallel.
Each Client sends X messages and each Server waits to be chosen N*X/M times.

Every read along the threads is done using `take_next_sample` and always after `wait_for_unread_message` returns that there are messages.
There are `ASSERT_EQ` after each `take_next_sample`, that should never happen (everything is intraprocess).

### Test failing

This tests does not usually fails, thus it must be executed several times in order to find the error:

* If using cmake command add the following flag
  * `./<path-to-fastdds-ws>/build/fastrtps/test/amlip_case/amlip_case_test --gtest_repeat=100`
* If using the colcon test command add the following flag:
  * `colcon test --packages-select fastrtps --ctest-args --timeout 20 -R amlip_case_test --repeat-until-fail=100`

This the failure message found:

```log
1: /home/paris/applications/eprosima/core/src/fastdds/test/amlip_case/amlip_case.cpp:228: Failure
1: Expected equality of these values:
1:   result
1:     Which is: 4-byte object <0B-00 00-00>
1:   ReturnCode_t::RETCODE_OK
1:     Which is: 0
1: in topic reply when reader has still data to read: 2
```

`<0B-00 00-00>` ReturnCode is 11 : `RETCODE_NO_DATA`

### on_data_available callback

There is no need to use this callback in this test. However it is used in the AML-IP and looks like it is a requirement to reproduce the issue.
This callback calls `get_unread_count` and `get_first_untaken_info`.
It must be tested if without these calls the issue is reproducible.

### LOG

Log Errors are used in this test for simplicity.
There is a macro that can be changed and recompiled in order to not show the logs.

### Scenario parameters

At the beginning of the test there are certain constants that can be modified to try different scenarios.
These set of values are known to reproduce the error. Some others may or may not reproduce it.

```cpp
constexpr const int TEST_DOMAIN = 42;
constexpr const bool USE_INTERPROCESS = true;

constexpr const char* REQUEST_TOPIC_NAME = "request";
constexpr const char* REPLY_TOPIC_NAME = "reply";
constexpr const char* TARGET_TOPIC_NAME = "target";

constexpr const int N_CLIENTS = 2;
constexpr const int N_SERVERS = 2;
constexpr const int N_TASKS = 4;
constexpr const int CLIENT_TIME_AFTER_TASK = 20;
constexpr const int SERVER_TIME_AFTER_TASK = 10;
```

---

## Investigation

### QoS

All this test (and AML-IP case) uses the following QoS:

* Transport:
  * Deactivate data_sharing
  * Uses only UDP and not Shared Memory
  * Uses intraprocess
* Endpoints
  * Every endpoint is RELIABLE, TRANSIENT_LOCAL and KEEP_ALL
* Dynamic Types
  * It deactivates everything possible of Dynamic Types. Otherwise it throws a seg fault.

### Clues

1. Before and after calling `take_next_sample`, `get_unread_count` gives a number higher than 0 (which makes no sense receiving a NO_DATA in take call). This is tested by adding a `get_unread_count` right after `assert` fails.
1. This only happens (or it looks like) if callback `on_data_available` is implemented.
1. If `take_next_sample` call is repeated right after returning NO_DATA, some of the times it returns the correct value (probably when there have been an addition at that specific time, so there are more data to retrieve).
1. Checking previous versions of Fast DDS the error is reproducible.
    1. Versions checked: `v2.9.0`, `v2.8.0`, `v2.7.0`, `v2.6.0`, `v2.5.0`.

### Fast DDS traces

There are some traces left in the code with comment `// (jparisu) AML-IP` in hot points where the error could be originated or at least traced.

---

## Attachments

There are 2 files attached to this test:

* `error_assert_unread` is a backtrace produces by adding an assert in `StatefulReader::change_read_by_user` (where the comment is)
* `error_out_of_range` other issue that happens from time to time (not sure it is related)
