# Intra-process delivery

Several requirements, each with a section.

## Identifying local endpoints
Endpoints on the same process have the same content on the first 8 octets of their GUID prefix. 

### GUID_t refactor
* Provide methods `is_builtin()`, `is_on_same_process_as(other_guid)` and `is_on_same_host_as(other_guid)`
* Consider other improvements
    * Split Guid.h into three headers (guid, prefix and entity_id)
    * Converting EntityId_t to uint32_t
    * Use a union on GuidPrefix_t to efficiently compare parts of it

### Getting pointers to local endpoints
* Add methods on RTPSDomain to return a pointer to reader/writer given its guid
* Should be aware of builtin endpoints

## Intra-process reception
The processing of new data on the subscribers is performed by the thread writing the data.
This thread is in charge of copying the `CacheChange_t` into the `ReaderHistory` and calling `NotifyChanges()`.
This decision was taken to maintain consistence with how currently operate the reception threads coming from transport
layer.

User's documentation should make clear any user's listener registered to be informed about new changes implies having
blocked both the transport's reception threads and threads which write data.
Also should explain there is another mechanism to read samples, using the function `wait_for_unread_samples()` from the
user's thread and then taking/reading them.

Current Reader API will be used by local writers to deliver data. The local writer will call `processDataMsg` directly.

## Readers: management of local writers
When matching a writer, the reader should check if it is on the same process.

**Considerations at destruction time**

Until now a reader was safe to be destructed because we are sure that no thread will access it. This is possible because:

* All events which use the reader were destructed first. Then the event thread will never access the reader.
* The reader was deregistered from the `ReceiverResource` objects. Then any reception thread will never access the
reader.

Now a reader can be accessed by a writer from the local process. But it was accessed if there was a match with the
writer using discovery. Therefore, we should make sure there will be an unmatch with the writer using discovery before
the reader is destructed. And this unmatch will be instantly because the EDP builtin endpoints will use intraprocess
mechanism.

### StatefulReader
No output traffic to local writers should be performed.
* Changes on WriterProxy
    * `remote_locators_shrinked` should return an empty vector for local writers
    * Timed event `heartbeat response` should never be started for local writers
    * Timed event `initial acknack` should directly call `process_acknack` on the local writer

## Writers: management of local readers
When matching a reader, the writer should check if it is on the same process.
* No output traffic to local readers should be performed.
* Samples are sent to local readers using `processDataMsg`.
* When a local reader is matched, and transient_local durability is used, the reader should automatically receive data
with `processDataMsg`.

### StatelessWriter
* On ReaderLocator::start, no locators should be added when the reader is local

### StatefulWriter
* Samples sent to local readers are automatically acknowledged
* Gaps sent to local readers should directly call `processGapMsg`
* Heartbeats sent to local readers should directly call `processHeartbeatMsg`
* Timed events should never be started for local readers, except for an `initial heartbeat`

## Discovery process

### Participant discovery (PDP)
In order to keep the sepparation of domains while on the same process, we will leave participant discovery to the standard mechanism.
No intraprocess delivery will be used for builtin PDP readers and writers.

### Endpoint discovery (EDP)
We will leave endpoint discovery to the standard mechanism, but builtin endpoints will use the new mechanism to
send/receive `WriterProxyData` and `ReaderProxyData` to/from the ones on the local process.

## Additional considerations

### Security
If we rely on the comparison of the GUID to identify endpoints on the same process, those belonging to a secured participant will not be taken into account, as in that case the GUID is recalculated using a hash of the whole participant GUID.

### Liveliness
No changes are necessary for the liveliness assertion mechanism, except that manual_by_topic assertions should directly call `processHeartbeatMsg` on local readers.

### Side effects

* This design isolates all message traffic from the wire, that is, tools like **wireshark™** will become useless. Given that most of our customers actually run intraprocess code, and report their issues using **wireshark™** traces, these changes will create a support nightmare. A workaround may be to supply a flag to inhibits the intraprocess behavior for debugging purposes, stating clearly, that it should be turn off during performance test.

* Once a shared memory transport is developed for interprocess purposes, an intraprocess mechanism would be pointless. That's because the trade off between speed gain and source complexity would be very expensive. From memory consumption point of view using an interprocess discovery database would yield more benefits that having per-process discovery databases.
