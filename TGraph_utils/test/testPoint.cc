#include "gtest/gtest.h"

#include "Point.h"

#include "TGraphAsymmErrors.h"

// compile with
// g++ -isystem /afs/hephy.at/user/t/tmadlener/googletest/googletest/include -pthread test_Point.cc $(root-config --libs --cflags) -Wall /afs/hephy.at/user/t/tmadlener/googletest/libgtest.a -o testPoint

// define some arbitrary data for constructing a TGraphAsymmErrors
// specifically made not all vectors have some sort of order for checking the comparison operator
const std::vector<double> xCoords = { 10.23, 2.3, 2.4, 3.5, 4.6 };
const std::vector<double> yCoords = { 2.3, 4.5, 3.4, 5.6, 6.7 };
const std::vector<double> errXHigh = { 0.2, 0.3, 0.4, 0.5, 0.6 };
const std::vector<double> errYHigh = { 0.23, 0.34, 0.45, 0.56, 0.67 };
const std::vector<double> errXLow = { 0.9, 0.8, 0.7, 0.6, 0.5 };
const std::vector<double> errYLow = { 0.98, 0.87, 0.76, 0.65, 0.54 };

class PointTest : public ::testing::Test {
public:
  PointTest()
  {
    testGraph = new TGraphAsymmErrors(xCoords.size(), xCoords.data(), yCoords.data(),
                                      errXLow.data(), errXHigh.data(), errYLow.data(), errYHigh.data());
  }

  virtual ~PointTest() { if(testGraph) delete testGraph; }

protected:
  TGraphAsymmErrors* testGraph;
};

/** check if the constructor copies all necessary data to the correct fields. */
TEST_F(PointTest, constructorTest)
{
  for (size_t i = 0; i < xCoords.size(); ++i) {
    Point p(testGraph, i);
    EXPECT_DOUBLE_EQ(xCoords[i], p.x);
    EXPECT_DOUBLE_EQ(yCoords[i], p.y);
    EXPECT_DOUBLE_EQ(errXHigh[i], p.exh);
    EXPECT_DOUBLE_EQ(errXLow[i], p.exl);
    EXPECT_DOUBLE_EQ(errYHigh[i], p.eyh);
    EXPECT_DOUBLE_EQ(errYLow[i], p.eyl);
  }
}

/** check if the operator< actually orders by the x-coordinate of a point. */
TEST_F(PointTest, operatorSmallerTest)
{
  Point p1(testGraph, 1); // x = 2.3, y = 4.5
  Point p2(testGraph, 2); // x = 2.4, y = 3.4
  EXPECT_TRUE(p1 < p2);

  Point p3(testGraph, 0); // x = 10.23
  EXPECT_FALSE(p3 < p1);
  EXPECT_TRUE(p2 < p3);
}

/** check if the addTo method actually adds a point to the passed TGraphAsymmErrors. */
TEST_F(PointTest, addToTest)
{
  TGraphAsymmErrors* graph = new TGraphAsymmErrors();
  EXPECT_EQ(0, graph->GetN());
  Point p(testGraph, 0);
  p.addTo(graph);

  EXPECT_EQ(1, graph->GetN());
  double x, y;
  EXPECT_EQ(0, graph->GetPoint(0, x, y));
  EXPECT_DOUBLE_EQ(p.x, x);
  EXPECT_DOUBLE_EQ(p.y, y);
  EXPECT_DOUBLE_EQ(p.exh, graph->GetErrorXhigh(0));
  EXPECT_DOUBLE_EQ(p.exl, graph->GetErrorXlow(0));
  EXPECT_DOUBLE_EQ(p.eyh, graph->GetErrorYhigh(0));
  EXPECT_DOUBLE_EQ(p.eyl, graph->GetErrorYlow(0));

  Point p2(testGraph, 2);
  p2.addTo(graph);
  EXPECT_EQ(2, graph->GetN());
  EXPECT_EQ(1, graph->GetPoint(1, x, y));
  EXPECT_DOUBLE_EQ(p2.x, x);
  EXPECT_DOUBLE_EQ(p2.y, y);
  EXPECT_DOUBLE_EQ(p2.exh, graph->GetErrorXhigh(1));
  EXPECT_DOUBLE_EQ(p2.exl, graph->GetErrorXlow(1));
  EXPECT_DOUBLE_EQ(p2.eyh, graph->GetErrorYhigh(1));
  EXPECT_DOUBLE_EQ(p2.eyl, graph->GetErrorYlow(1));
}

int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
