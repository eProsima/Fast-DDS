set(XFAIL_DDS_PIM_TESTS
    BlackboxTests_DDS_PIM.Discovery.ParticipantLivelinessAssertion
    BlackboxTests_DDS_PIM.LivelinessQos/LivelinessQos.LongLiveliness_ManualByParticipant_BestEffort/Transport
    BlackboxTests_DDS_PIM.LivelinessQos/LivelinessQos.ShortLiveliness_ManualByParticipant_Automatic_BestEffort/Intraprocess
    BlackboxTests_DDS_PIM.LivelinessQos/LivelinessQos.ShortLiveliness_ManualByParticipant_BestEffort/Intraprocess
    BlackboxTests_DDS_PIM.LivelinessQos/LivelinessQos.ShortLiveliness_ManualByTopic_Automatic_BestEffort/Transport
    BlackboxTests_DDS_PIM.LivelinessQos/LivelinessQos.ShortLiveliness_ManualByTopic_Automatic_Reliable/Transport
    BlackboxTests_DDS_PIM.LivelinessQos/LivelinessQos.ThreeWriters_ThreeReaders/Intraprocess
    BlackboxTests_DDS_PIM.LivelinessQos/LivelinessQos.ThreeWriters_ThreeReaders/Transport
    BlackboxTests_DDS_PIM.LivelinessQos/LivelinessQos.TwoWriters_OneReader_ManualByParticipant/Intraprocess
    BlackboxTests_DDS_PIM.PersistenceLargeData/PersistenceLargeData.PubSubAsReliablePubPersistentWithFrag/Transport

    ##########################################################
    # These test are here because of TCP instabilities
    ##########################################################
    BlackboxTests_DDS_PIM.ChainingTransportTests.builtin_transports_api_large_data
    BlackboxTests_DDS_PIM.ChainingTransportTests.builtin_transports_api_large_datav6
    BlackboxTests_DDS_PIM.ChainingTransportTests.builtin_transports_env_large_data
    BlackboxTests_DDS_PIM.ChainingTransportTests.builtin_transports_env_large_datav6
    BlackboxTests_DDS_PIM.ChainingTransportTests.builtin_transports_xml_large_data
    BlackboxTests_DDS_PIM.ChainingTransportTests.builtin_transports_xml_large_datav6
    ##########################################################
    )

set(XFAIL_FASTRTPS_TESTS
    BlackboxTests_FastRTPS.Discovery.ParticipantLivelinessAssertion
    BlackboxTests_FastRTPS.LivelinessQos/LivelinessQos.LongLiveliness_ManualByParticipant_BestEffort/Transport
    BlackboxTests_FastRTPS.LivelinessQos/LivelinessQos.ShortLiveliness_ManualByParticipant_Automatic_BestEffort/Intraprocess
    BlackboxTests_FastRTPS.LivelinessQos/LivelinessQos.ShortLiveliness_ManualByParticipant_BestEffort/Intraprocess
    BlackboxTests_FastRTPS.LivelinessQos/LivelinessQos.ShortLiveliness_ManualByTopic_Automatic_BestEffort/Transport
    BlackboxTests_FastRTPS.LivelinessQos/LivelinessQos.ShortLiveliness_ManualByTopic_Automatic_Reliable/Transport
    BlackboxTests_FastRTPS.LivelinessQos/LivelinessQos.ThreeWriters_ThreeReaders/Intraprocess
    BlackboxTests_FastRTPS.LivelinessQos/LivelinessQos.ThreeWriters_ThreeReaders/Transport
    BlackboxTests_FastRTPS.LivelinessQos/LivelinessQos.TwoWriters_OneReader_ManualByParticipant/Intraprocess
    BlackboxTests_FastRTPS.PersistenceLargeData/PersistenceLargeData.PubSubAsReliablePubPersistentWithFrag/Transport

    ##########################################################
    # These test are here because of TCP instabilities
    ##########################################################
    BlackboxTests_FastRTPS.ChainingTransportTests.builtin_transports_api_large_data
    BlackboxTests_FastRTPS.ChainingTransportTests.builtin_transports_api_large_datav6
    BlackboxTests_FastRTPS.ChainingTransportTests.builtin_transports_env_large_data
    BlackboxTests_FastRTPS.ChainingTransportTests.builtin_transports_env_large_datav6
    BlackboxTests_FastRTPS.ChainingTransportTests.builtin_transports_xml_large_data
    BlackboxTests_FastRTPS.ChainingTransportTests.builtin_transports_xml_large_datav6
    ##########################################################
    )

set_tests_properties(${XFAIL_DDS_PIM_TESTS}
    PROPERTIES LABELS xfail
    )

set_tests_properties(${XFAIL_FASTRTPS_TESTS}
    PROPERTIES LABELS xfail
    )
