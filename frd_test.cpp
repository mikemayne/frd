// Copyright 2019 Michael Mayne

// Permission is hereby granted, free of charge, to any person obtaining a copy of this 
// software and associated documentation files (the "Software"), to deal in the Software without restriction, 
// including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
// subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
//  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "frd.h"

#include <sstream>
#include <fstream>

#include "gtest/gtest.h"
namespace {


    template <typename FloatType>
    bool equals(const frd::FRDValue<FloatType>& lhs, const frd::FRDValue<FloatType>& rhs)
    {
        return lhs.freqHz   == rhs.freqHz 
            && lhs.dBSPL    == rhs.dBSPL 
            && lhs.phaseDeg == rhs.phaseDeg;
    }


    TEST(FrdParseFileTest, TestParseFile) {

        std::ifstream fs("testfrd.txt");
        auto Frd = frd::parse<double>(fs);

        ASSERT_EQ(256, Frd.size());
        EXPECT_TRUE(equals(Frd[0], {10.00, 95.24, -37.18}));
    }

    TEST(FrdParseTest, TestEmpty) {

        std::stringstream ss;
        auto vec = frd::parse<double>(ss);

        EXPECT_EQ(0, vec.size());
    }

    TEST(FrdParseTest, TestSingleCommentLine) {

        std::stringstream ss;
        ss << "A comment";
        auto vec = frd::parse<double>(ss);

        EXPECT_EQ(0, vec.size());
    }

    TEST(FrdParseTest, TestSingleDataLine) {

        std::stringstream ss;
        ss << "A comment" << std::endl;
        ss << "10 1 1" << std::endl;
        auto vec = frd::parse<double>(ss);

        ASSERT_EQ(1, vec.size());
        EXPECT_EQ(vec[0].freqHz, 10.);
        EXPECT_EQ(vec[0].dBSPL, 1.);
        EXPECT_EQ(vec[0].phaseDeg, 1.);
    }

    TEST(FrdParseTest, TestSingleDataLineWithLotsOfDecimalPlaces) {

        std::stringstream ss;
        ss << "A comment" << std::endl;
        ss << "10 1 1.";

        const int num_dp = 20;
        for (int i=0; i<num_dp; ++i)
            ss << "1";

        ss << std::endl;

        auto vec = frd::parse<double>(ss);

        ASSERT_EQ(1, vec.size());
        EXPECT_EQ(vec[0].freqHz, 10.);
        EXPECT_EQ(vec[0].dBSPL, 1.);
        EXPECT_EQ(vec[0].phaseDeg, 1.11111111111111111111);
    }

    TEST(FrdParseTest, TestMultipleDataLinesWithMixedWhitespace) {

        std::stringstream ss;
        ss << "A comment" << std::endl;
        const int num_lines = 1000;
        for (int i=0; i<num_lines; ++i)
            ss << "10.0             1.0\t\t\t1.0" << std::endl;

        auto vec = frd::parse<double>(ss);

        ASSERT_EQ(num_lines, vec.size());
        EXPECT_EQ(vec[0].freqHz, 10.);
        EXPECT_EQ(vec[0].dBSPL, 1.);
        EXPECT_EQ(vec[0].phaseDeg, 1.);

        const int last_line = num_lines-1;
        EXPECT_EQ(vec[last_line].freqHz, 10.);
        EXPECT_EQ(vec[last_line].dBSPL, 1.);
        EXPECT_EQ(vec[last_line].phaseDeg, 1.);
    }

    TEST(FrdFindFreqTest, TestFindInEmptyVector) {

        std::vector<frd::FRDValue<double>> Frd;
        EXPECT_EQ(findFreq(Frd, 0.), Frd.end());
    }

    TEST(FrdFindFreqTest, TestFindInSingleElementVector) {

        std::vector<frd::FRDValue<double>> Frd;
        Frd.push_back({10., 10., 10.});
        ASSERT_NE(findFreq(Frd, 0.), Frd.end());
        EXPECT_EQ((*findFreq(Frd, 0.)).freqHz, 10.);
    }

    TEST(FrdFindFreqTest, TestFindInMultipleElementVector) {

        std::vector<frd::FRDValue<double>> Frd;

        for (int i=0; i<10; ++i) {
            Frd.push_back({i*10., i*10., 0.});
        }

        auto lessThanZero = findFreq(Frd, -10.);
        ASSERT_NE(lessThanZero, Frd.end());
        EXPECT_EQ((*lessThanZero).freqHz, 0.);

        auto justUnderMiddle = findFreq(Frd, 45.);
        ASSERT_NE(justUnderMiddle, Frd.end());
        EXPECT_EQ((*justUnderMiddle).freqHz, 50.);

        auto middle = findFreq(Frd, 50.);
        ASSERT_NE(middle, Frd.end());
        EXPECT_EQ((*middle).freqHz, 50.);

        auto justOverMiddle = findFreq(Frd, 55.);
        ASSERT_NE(justOverMiddle, Frd.end());
        EXPECT_EQ((*justOverMiddle).freqHz, 60.);

        auto last = findFreq(Frd, 90.);
        ASSERT_NE(last, Frd.end());
        EXPECT_EQ((*last).freqHz, 90.);

        auto pastEnd = findFreq(Frd, 100.);
        ASSERT_EQ(pastEnd, Frd.end());
    }


    TEST(FrdPolarDataTest, TestImportPolarData) {

        auto polar = frd::import_polardata<double>("./polar_test");
		ASSERT_GE(polar.size(), 3);
		ASSERT_GE(polar[0].size(), 1);
        EXPECT_EQ(polar[0][0].freqHz, 10);
        EXPECT_EQ(polar[0][0].dBSPL, 1);
        EXPECT_EQ(polar[0][0].phaseDeg, 1);

		ASSERT_GE(polar[1].size(), 1);
        EXPECT_EQ(polar[1][0].freqHz, 10);
        EXPECT_EQ(polar[1][0].dBSPL, 2);
        EXPECT_EQ(polar[1][0].phaseDeg, 2);

		ASSERT_GE(polar[2].size(), 1);
        EXPECT_EQ(polar[2][0].freqHz, 10);
        EXPECT_EQ(polar[2][0].dBSPL, 3);
        EXPECT_EQ(polar[2][0].phaseDeg, 3);
    }
}