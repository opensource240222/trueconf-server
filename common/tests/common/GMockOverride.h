#pragma once

#include <gmock/gmock-generated-function-mockers.h>

#if defined(VS_GOOGLETEST)

#define MOCK_METHOD0_OVERRIDE(m, ...) GMOCK_METHOD0_(, , override, , m, __VA_ARGS__)
#define MOCK_METHOD1_OVERRIDE(m, ...) GMOCK_METHOD1_(, , override, , m, __VA_ARGS__)
#define MOCK_METHOD2_OVERRIDE(m, ...) GMOCK_METHOD2_(, , override, , m, __VA_ARGS__)
#define MOCK_METHOD3_OVERRIDE(m, ...) GMOCK_METHOD3_(, , override, , m, __VA_ARGS__)
#define MOCK_METHOD4_OVERRIDE(m, ...) GMOCK_METHOD4_(, , override, , m, __VA_ARGS__)
#define MOCK_METHOD5_OVERRIDE(m, ...) GMOCK_METHOD5_(, , override, , m, __VA_ARGS__)
#define MOCK_METHOD6_OVERRIDE(m, ...) GMOCK_METHOD6_(, , override, , m, __VA_ARGS__)
#define MOCK_METHOD7_OVERRIDE(m, ...) GMOCK_METHOD7_(, , override, , m, __VA_ARGS__)
#define MOCK_METHOD8_OVERRIDE(m, ...) GMOCK_METHOD8_(, , override, , m, __VA_ARGS__)
#define MOCK_METHOD9_OVERRIDE(m, ...) GMOCK_METHOD9_(, , override, , m, __VA_ARGS__)
#define MOCK_METHOD10_OVERRIDE(m, ...) GMOCK_METHOD10_(, , override, , m, __VA_ARGS__)

#define MOCK_CONST_METHOD0_OVERRIDE(m, ...) GMOCK_METHOD0_(, const, override, , m, __VA_ARGS__)
#define MOCK_CONST_METHOD1_OVERRIDE(m, ...) GMOCK_METHOD1_(, const, override, , m, __VA_ARGS__)
#define MOCK_CONST_METHOD2_OVERRIDE(m, ...) GMOCK_METHOD2_(, const, override, , m, __VA_ARGS__)
#define MOCK_CONST_METHOD3_OVERRIDE(m, ...) GMOCK_METHOD3_(, const, override, , m, __VA_ARGS__)
#define MOCK_CONST_METHOD4_OVERRIDE(m, ...) GMOCK_METHOD4_(, const, override, , m, __VA_ARGS__)
#define MOCK_CONST_METHOD5_OVERRIDE(m, ...) GMOCK_METHOD5_(, const, override, , m, __VA_ARGS__)
#define MOCK_CONST_METHOD6_OVERRIDE(m, ...) GMOCK_METHOD6_(, const, override, , m, __VA_ARGS__)
#define MOCK_CONST_METHOD7_OVERRIDE(m, ...) GMOCK_METHOD7_(, const, override, , m, __VA_ARGS__)
#define MOCK_CONST_METHOD8_OVERRIDE(m, ...) GMOCK_METHOD8_(, const, override, , m, __VA_ARGS__)
#define MOCK_CONST_METHOD9_OVERRIDE(m, ...) GMOCK_METHOD9_(, const, override, , m, __VA_ARGS__)
#define MOCK_CONST_METHOD10_OVERRIDE(m, ...) GMOCK_METHOD10_(, const, override, , m, __VA_ARGS__)

#define MOCK_METHOD0_T_OVERRIDE(m, ...) GMOCK_METHOD0_(typename, , override, , m, __VA_ARGS__)
#define MOCK_METHOD1_T_OVERRIDE(m, ...) GMOCK_METHOD1_(typename, , override, , m, __VA_ARGS__)
#define MOCK_METHOD2_T_OVERRIDE(m, ...) GMOCK_METHOD2_(typename, , override, , m, __VA_ARGS__)
#define MOCK_METHOD3_T_OVERRIDE(m, ...) GMOCK_METHOD3_(typename, , override, , m, __VA_ARGS__)
#define MOCK_METHOD4_T_OVERRIDE(m, ...) GMOCK_METHOD4_(typename, , override, , m, __VA_ARGS__)
#define MOCK_METHOD5_T_OVERRIDE(m, ...) GMOCK_METHOD5_(typename, , override, , m, __VA_ARGS__)
#define MOCK_METHOD6_T_OVERRIDE(m, ...) GMOCK_METHOD6_(typename, , override, , m, __VA_ARGS__)
#define MOCK_METHOD7_T_OVERRIDE(m, ...) GMOCK_METHOD7_(typename, , override, , m, __VA_ARGS__)
#define MOCK_METHOD8_T_OVERRIDE(m, ...) GMOCK_METHOD8_(typename, , override, , m, __VA_ARGS__)
#define MOCK_METHOD9_T_OVERRIDE(m, ...) GMOCK_METHOD9_(typename, , override, , m, __VA_ARGS__)
#define MOCK_METHOD10_T_OVERRIDE(m, ...) GMOCK_METHOD10_(typename, , override, , m, __VA_ARGS__)

#define MOCK_CONST_METHOD0_T_OVERRIDE(m, ...) GMOCK_METHOD0_(typename, const, override, , m, __VA_ARGS__)
#define MOCK_CONST_METHOD1_T_OVERRIDE(m, ...) GMOCK_METHOD1_(typename, const, override, , m, __VA_ARGS__)
#define MOCK_CONST_METHOD2_T_OVERRIDE(m, ...) GMOCK_METHOD2_(typename, const, override, , m, __VA_ARGS__)
#define MOCK_CONST_METHOD3_T_OVERRIDE(m, ...) GMOCK_METHOD3_(typename, const, override, , m, __VA_ARGS__)
#define MOCK_CONST_METHOD4_T_OVERRIDE(m, ...) GMOCK_METHOD4_(typename, const, override, , m, __VA_ARGS__)
#define MOCK_CONST_METHOD5_T_OVERRIDE(m, ...) GMOCK_METHOD5_(typename, const, override, , m, __VA_ARGS__)
#define MOCK_CONST_METHOD6_T_OVERRIDE(m, ...) GMOCK_METHOD6_(typename, const, override, , m, __VA_ARGS__)
#define MOCK_CONST_METHOD7_T_OVERRIDE(m, ...) GMOCK_METHOD7_(typename, const, override, , m, __VA_ARGS__)
#define MOCK_CONST_METHOD8_T_OVERRIDE(m, ...) GMOCK_METHOD8_(typename, const, override, , m, __VA_ARGS__)
#define MOCK_CONST_METHOD9_T_OVERRIDE(m, ...) GMOCK_METHOD9_(typename, const, override, , m, __VA_ARGS__)
#define MOCK_CONST_METHOD10_T_OVERRIDE(m, ...) GMOCK_METHOD10_(typename, const, override, , m, __VA_ARGS__)

#else

#define MOCK_METHOD0_OVERRIDE MOCK_METHOD0
#define MOCK_METHOD1_OVERRIDE MOCK_METHOD1
#define MOCK_METHOD2_OVERRIDE MOCK_METHOD2
#define MOCK_METHOD3_OVERRIDE MOCK_METHOD3
#define MOCK_METHOD4_OVERRIDE MOCK_METHOD4
#define MOCK_METHOD5_OVERRIDE MOCK_METHOD5
#define MOCK_METHOD6_OVERRIDE MOCK_METHOD6
#define MOCK_METHOD7_OVERRIDE MOCK_METHOD7
#define MOCK_METHOD8_OVERRIDE MOCK_METHOD8
#define MOCK_METHOD9_OVERRIDE MOCK_METHOD9
#define MOCK_METHOD10_OVERRIDE MOCK_METHOD10

#define MOCK_CONST_METHOD0_OVERRIDE MOCK_CONST_METHOD0
#define MOCK_CONST_METHOD1_OVERRIDE MOCK_CONST_METHOD1
#define MOCK_CONST_METHOD2_OVERRIDE MOCK_CONST_METHOD2
#define MOCK_CONST_METHOD3_OVERRIDE MOCK_CONST_METHOD3
#define MOCK_CONST_METHOD4_OVERRIDE MOCK_CONST_METHOD4
#define MOCK_CONST_METHOD5_OVERRIDE MOCK_CONST_METHOD5
#define MOCK_CONST_METHOD6_OVERRIDE MOCK_CONST_METHOD6
#define MOCK_CONST_METHOD7_OVERRIDE MOCK_CONST_METHOD7
#define MOCK_CONST_METHOD8_OVERRIDE MOCK_CONST_METHOD8
#define MOCK_CONST_METHOD9_OVERRIDE MOCK_CONST_METHOD9
#define MOCK_CONST_METHOD10_OVERRIDE MOCK_CONST_METHOD10

#define MOCK_METHOD0_T_OVERRIDE MOCK_METHOD0_T
#define MOCK_METHOD1_T_OVERRIDE MOCK_METHOD1_T
#define MOCK_METHOD2_T_OVERRIDE MOCK_METHOD2_T
#define MOCK_METHOD3_T_OVERRIDE MOCK_METHOD3_T
#define MOCK_METHOD4_T_OVERRIDE MOCK_METHOD4_T
#define MOCK_METHOD5_T_OVERRIDE MOCK_METHOD5_T
#define MOCK_METHOD6_T_OVERRIDE MOCK_METHOD6_T
#define MOCK_METHOD7_T_OVERRIDE MOCK_METHOD7_T
#define MOCK_METHOD8_T_OVERRIDE MOCK_METHOD8_T
#define MOCK_METHOD9_T_OVERRIDE MOCK_METHOD9_T
#define MOCK_METHOD10_T_OVERRIDE MOCK_METHOD10_T

#define MOCK_CONST_METHOD0_T_OVERRIDE MOCK_CONST_METHOD0_T
#define MOCK_CONST_METHOD1_T_OVERRIDE MOCK_CONST_METHOD1_T
#define MOCK_CONST_METHOD2_T_OVERRIDE MOCK_CONST_METHOD2_T
#define MOCK_CONST_METHOD3_T_OVERRIDE MOCK_CONST_METHOD3_T
#define MOCK_CONST_METHOD4_T_OVERRIDE MOCK_CONST_METHOD4_T
#define MOCK_CONST_METHOD5_T_OVERRIDE MOCK_CONST_METHOD5_T
#define MOCK_CONST_METHOD6_T_OVERRIDE MOCK_CONST_METHOD6_T
#define MOCK_CONST_METHOD7_T_OVERRIDE MOCK_CONST_METHOD7_T
#define MOCK_CONST_METHOD8_T_OVERRIDE MOCK_CONST_METHOD8_T
#define MOCK_CONST_METHOD9_T_OVERRIDE MOCK_CONST_METHOD9_T
#define MOCK_CONST_METHOD10_T_OVERRIDE MOCK_CONST_METHOD10_T

#endif