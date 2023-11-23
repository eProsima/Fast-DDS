set(XFAIL_DDS_PIM_TESTS
    Discovery.ParticipantLivelinessAssertion
    LivelinessQos/LivelinessQos.LongLiveliness_ManualByParticipant_BestEffort/Transport
    LivelinessQos/LivelinessQos.ShortLiveliness_ManualByParticipant_Automatic_BestEffort/Intraprocess
    LivelinessQos/LivelinessQos.ShortLiveliness_ManualByParticipant_BestEffort/Intraprocess
    LivelinessQos/LivelinessQos.ShortLiveliness_ManualByTopic_Automatic_BestEffort/Transport
    LivelinessQos/LivelinessQos.ShortLiveliness_ManualByTopic_Automatic_Reliable/Transport
    LivelinessQos/LivelinessQos.ThreeWriters_ThreeReaders/Intraprocess
    LivelinessQos/LivelinessQos.ThreeWriters_ThreeReaders/Transport
    LivelinessQos/LivelinessQos.TwoWriters_OneReader_ManualByParticipant/Intraprocess
    PersistenceLargeData/PersistenceLargeData.PubSubAsReliablePubPersistentWithFrag/Transport
    )

set_tests_properties(${XFAIL_DDS_PIM_TESTS}
    PROPERTIES LABELS xfail
    )
