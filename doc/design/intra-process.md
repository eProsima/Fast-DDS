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

## Intra-process reception thread
In order to separate the processing of new data on the subscribers from the user calls to write on the publisher, and simulate reception from the transport layer, a local reception thread should be added. It should work in a similar way to the AsyncWriterThread: readers will register their interest when they are informed of new local data. When the thread is awakened, it calls a `notify_new_changes` method on all the interested readers.

### RTPSReader
* A `ResourceLimitedVector<CacheChange_t*> unnotified_changes` should be added to RTPSReader to hold the list of changes pending notification.
* Method `notify_new_changes` will perform notification of reception for all pending changes.
* Method `receive_local_writer_data` that receives a change from a local writer
* Method `receive_local_writer_gap` that receives a gap from a local writer

Additional consideration: It may be interesting to also use this mechanism for transport received data. This will imply that all data reception callbacks will be made on the same thread, regardless of being received from the local process or a remote one. The tradeoff here is the number of context switches will increase.

## Readers: management of local writers
When matching a writer, the reader should check if it is on the same process.

### StatelessReader
* Keep an `is_on_same_process` flag on RemoteWriterInfo_t
* Make `acceptMsgFrom` return false for local writers. This is necessary in case we receive from a local writer by multicast.

### StatefulReader
No output traffic to local writers should be performed.
* Changes on WriterProxy
    * `remote_locators_shrinked` should return an empty vector for local writers
    * Timed events should never be started for local writers
* Changes on StatefulReader
    * Make `processDataMsg` and `processDataFragMsg` ignore local writers. This is necessary in case we receive from a local writer by multicast.

## Writers: management of local readers
When matching a reader, the writer should check if it is on the same process.
* No output traffic to local readers should be performed.
* Samples are sent to local readers using `receive_local_writer_data`.
* When a local reader is matched, and transient_local durabitlity is used, the reader automatically receives data with `receive_local_writer_data`.

### StatelessWriter
* On ReaderLocator::start, no locators should be added when the reader is local

### StatefulWriter
* Samples sent to local readers are automatically acknowledged
* Gaps sent to local readers should use `receive_local_writer_gap`
* Timed events should never be started for local readers

## Discovery process

### Participant discovery (PDP)
We will leave participant discovery to the standard mechanism, but builtin endpoints will use the new
mechanism to match with the ones on the local process.

### Endpoint discovery (EDP)
Current mechanism performs matching on the local participant when an endpoint is created. 
This should be extended to inform all participants of the same domain in the current process.
We should ensure that local readers match the local writers before the local writers match the local readers.

So the process would be:
```
// When creating a writer
foreach reader in join(local process readers, discovered remote readers)
    if valid_matching(reader, writer)
        reader.matched_writer_add(writer)
        writer.matched_reader_add(reader)

// When creating a reader
foreach writer in join(local process writers, discovered remote writers)
    if valid_matching(reader, writer)
        reader.matched_writer_add(writer)
        writer.matched_reader_add(reader)
```

## New design side-effects

* This design isolates all message traffic from the wire, that is, tools like **wireshark™** will become useless. Given that most of our customers actually run intraprocess code, and report their issues using **wireshark™** traces, these changes will create a support nightmare. A workaround may be to supply a flag to inhibits the intraprocess behavior for debugging purposes, stating clearly, that it should be turn off during performance test.

* Once a shared memory transport is developed for interprocess purposes, an intraprocess mechanism would be pointless. That's because the trade off between speed gain and source complexity would be very expensive. From memory consumption point of view using an interprocess discovery database would yield more benefits that having per-process discovery databases.