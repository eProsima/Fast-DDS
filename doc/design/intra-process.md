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

Current Reader API will be used by local writers to deliver data. The local writer will call `processDataMsg` and
`processDataFragMsg` directly.

## Readers: management of local writers
When matching a writer, the reader should check if it is on the same process.

### StatefulReader
No output traffic to local writers should be performed.
* Changes on WriterProxy
    * `remote_locators_shrinked` should return an empty vector for local writers
    * Timed events should never be started for local writers

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

## Additional considerations

### Security
If we rely on the comparison of the GUID to identify endpoints on the same process, those belonging to a secured participant will not be taken into account, as in that case the GUID is recalculated using a hash of the whole participant GUID.

### Liveliness
There may be changes to the liveliness assertion mechanism, as local process endpoints may be automatically asserted without the need of exchanging messages. Even better, it may be possible to avoid adding local endpoints to the liveliness managers.

### Side effects

* This design isolates all message traffic from the wire, that is, tools like **wireshark™** will become useless. Given that most of our customers actually run intraprocess code, and report their issues using **wireshark™** traces, these changes will create a support nightmare. A workaround may be to supply a flag to inhibits the intraprocess behavior for debugging purposes, stating clearly, that it should be turn off during performance test.

* Once a shared memory transport is developed for interprocess purposes, an intraprocess mechanism would be pointless. That's because the trade off between speed gain and source complexity would be very expensive. From memory consumption point of view using an interprocess discovery database would yield more benefits that having per-process discovery databases.
