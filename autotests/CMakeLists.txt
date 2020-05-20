include(ECMAddTests)

ecm_add_tests(
 kplugininfotest.cpp

 LINK_LIBRARIES KF5KCMUtils Qt5::Test
)
