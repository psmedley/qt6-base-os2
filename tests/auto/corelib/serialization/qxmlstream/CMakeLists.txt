# Generated from qxmlstream.pro.

#####################################################################
## tst_qxmlstream Test:
#####################################################################

# Collect test data
file(GLOB_RECURSE test_data
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        data/* XML-Test-Suite/*)

file(GLOB_RECURSE tokenError
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        tokenError/*)

qt_internal_add_test(tst_qxmlstream
    SOURCES
        tst_qxmlstream.cpp
    LIBRARIES
        Qt::Network
        Qt::GuiPrivate
    TESTDATA
        ${test_data}
        ${tokenError}
)
