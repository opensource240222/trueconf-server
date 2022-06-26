#include "tools/SingleGatewayLib/VS_H264ResolutionCalc.h"

#include <gtest/gtest.h>

TEST(H264ResolutionCalcTest, bw27600)
{
	VS_H264ResolutionCalc calc(13);
	calc.SetMaxFs(1590);
	calc.SetMaxMbps(27600);
	EXPECT_EQ(calc.GetWidth(), 640);
	EXPECT_EQ(calc.GetHeight(), 360);
}

TEST(H264ResolutionCalcTest, bw108000)
{
	VS_H264ResolutionCalc calc(13);
	calc.SetMaxFs(3600);
	calc.SetMaxMbps(108000);
	EXPECT_EQ(calc.GetWidth(), 1280);
	EXPECT_EQ(calc.GetHeight(), 720);
}

TEST(H264ResolutionCalcTest, bw40500)
{
	VS_H264ResolutionCalc calc(13);
	calc.SetMaxFs(1344);
	calc.SetMaxMbps(40500);
	EXPECT_EQ(calc.GetWidth(), 768);
	EXPECT_EQ(calc.GetHeight(), 448);
}

TEST(H264ResolutionCalcTest, bw35000)
{
	VS_H264ResolutionCalc calc(22);
	calc.SetMaxFs(3600);
	calc.SetMaxMbps(35000);
	EXPECT_EQ(calc.GetWidth(), 640);
	EXPECT_EQ(calc.GetHeight(), 360);
}
