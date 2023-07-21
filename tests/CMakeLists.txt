# Unit tests

TestTarget(ut_mfcnt
    SOURCES
        ut_mfcnt.cpp
    LIBRARIES
        mfcnt
    DEPENDS
        testing
)

# Performance tests

TestTarget(perf_mfcnt
    SOURCES
        perf_mfcnt.cpp
    LIBRARIES
        mfcnt
    DEPENDS
        testing
)

