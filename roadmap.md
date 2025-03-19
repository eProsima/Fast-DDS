# Fast DDS Roadmap

This section elaborates on proposed new features or tasks which are expected to be added to the product in the foreseeable future.
There should be no assumption of a commitment to deliver these features on specific dates or in the order given.
The development team will be doing their best to follow the proposed dates and priorities, but please bear in mind that plans to work on a given feature or task may be revised.
All information is provided as a general guidelines only, and this section may be revised to provide newer information at any time.

Disclaimer:

* This section has been last updated in March 2025.
  Please take into account its content could be obsolete.

## Short term

The following list of features are planned to be addressed in the short term, and incorporated into the following release of the product (v3.3.0):

* Consolidation of some [properties](https://fast-dds.docs.eprosima.com/en/latest/fastdds/property_policies/property_policies.html) into QoS policies
* SHM permissions override
* Ignore local endpoints configured at endpoint level

## Medium term

The following list of features are planned to be addressed in the medium term, typically within the subsequent release(s) generated in the next **9 months** after next planned release:

* Support for custom interface selection for statistics traffic
* Full support for DDS X-TYPES assignability checks
* ChainingTransport configuration through XML
* Tier 1 support for QNX platforms
* Tier 1 support for Android platforms
* Full support for DESTINATION_ORDER QoS
* Support DURABILITY_SERVICE QoS
* Set default log level through XML

## Long term

The following list of features are proposals regarding the longer-term evolution of the product even though development of these features has not yet been scheduled for a release in the near future.
Please feel free to contact us if you wish to get involved in the implementation or influence the roadmap.

* Full DDS QOS compliance
    * Support for TIME_BASED_FILTER QoS
	* Support for TRANSPORT_PRIORITY QoS
	* Support for READER_DATA_LIFECYCLE QoS
	* Support for LATENCY_BUDGET QoS
	* Support for PRESENTATION QoS
* Full DDS API compliance
* Batching (allow small samples to be grouped in a single datagram)
* Allow user to inject system dependencies:
    * Getting current timestamp
    * Getting Host ID / process info / etc
    * Query network interfaces
    * Thread factory
    * Security plugin factory
* Allow user to configure when samples are acknowledged:
    * When available on reader
    * When processed (return_loan / take without loan)
    * On demand (new API on DataReader)
* New DEBUG log level
* Compliance with DDS-TSN
* Service aware DataWriter
* Compliance with DDS-XML
* Low bandwidth transports
