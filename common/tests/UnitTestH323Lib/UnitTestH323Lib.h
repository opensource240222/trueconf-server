#pragma once
#include <gtest/gtest.h>

template<class T>
class H323LibBaseTest : public T {};

class H225MessageDecodeTest : public H323LibBaseTest< ::testing::Test> {};
class H225RasMessageDecodeTest : public H225MessageDecodeTest {};
class H245MessagDecodeTest : public H323LibBaseTest< ::testing::Test> {};

class H225MessageEncodeTest : public H323LibBaseTest< ::testing::Test> {};
class H225RasMessageEncodeTest : public H225MessageEncodeTest{};
class H245MessagEncodeTest : public H323LibBaseTest< ::testing::Test> {};

class H225RasRegressionTest : public H323LibBaseTest< ::testing::Test> {};