set(XFAIL_SHMTRANSPORT_TESTS
    SHMTransportTests.buffer_recover
    )

set_tests_properties(${XFAIL_SHM_TRANSPORT_TESTS}
    PROPERTIES LABELS xfail
    )
