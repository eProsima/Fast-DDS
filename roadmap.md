# Fast RTPS Roadmap

This product is a FIWARE Generic Enabler.
If you would like to learn about the overall Roadmap of FIWARE, please check section "Roadmap" on the FIWARE Catalogue.

## Introduction

This section elaborates on proposed new features or tasks which are expected to be added to the product in the
foreseeable future.
There should be no assumption of a commitment to deliver these features on specific dates or in the order given.
The development team will be doing their best to follow the proposed dates and priorities, but please bear in mind
that plans to work on a given feature or task may be revised.
All information is provided as a general guidelines only, and this section may be revised to provide newer information
at any time.

Disclaimer:

* This section has been last updated in March 2022.
  Please take into account its content could be obsolete.

## Short term

The following list of features are planned to be addressed in the short term, and incorporated into the following
minor release of the product:

* Content filtered topic support on DataWriter
* Complete support for dynamic network interfaces
* ReadCondition implementation
* Secure Discovery Server
* DataReader, DataWriter and DomainParticipant DDS API implementation:
    - `DataReader::get_sample_lost_status`
    - `DataReader::lookup_instance`
    - `DataWriter::get_key_value`
    - `DataWriter::write_w_timestamp`
    - `DomainParticipant::find_topic`
* Android support

## Medium term

The following list of features are planned to be addressed in the medium term, typically within the subsequent
release(s) generated in the next **9 months** after next planned release:

* Service aware DataWriter
* DDS-XTypes 1.3

## Long term

The following list of features are proposals regarding the longer-term evolution of the product even though development
of these features has not yet been scheduled for a release in the near future.
Please feel free to contact us if you wish to get involved in the implementation or influence the roadmap.

* Full DDS API compliance
* Compliance with DDS-XML
* Low bandwidth transports
* ISO 26262 compliance
