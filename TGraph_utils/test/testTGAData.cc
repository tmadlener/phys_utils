#include "gtest/gtest.h"

#include "TGAData.h"
#include "Point.h"

#include "TGraphAsymmErrors.h"
#include <vector>

// compile with
// g++ -isystem /afs/hephy.at/user/t/tmadlener/googletest/googletest/include -pthread test_TGAData.cc $(root-config --libs --cflags) -Wall /afs/hephy.at/user/t/tmadlener/googletest/libgtest.a -o testTGAData

// define some vector<double> to construct some TGraphAsymmErrors. (values do not actually matter!)
const std::vector<double> xCoords1 = { 1.2, 2.3, 3.4, 4.5, 5.6 };
const std::vector<double> yCoords1 = { 9.8, 7.8, 6.8, 5.8, 5.2 };
const std::vector<double> errors1 = { 0.1, 0.2, 0.3, 0.356, 0.47 }; // error values do not really matter!

const std::vector<double> xCoords2 = { 2.5, 4.5, 6.3, 4.2, 5.6 };
const std::vector<double> yCoords2 = { 4.25, 2.636, 45.25, 4.25, 6.2 };
const std::vector<double> errors2 = { 0.5, 0.6, 0.4, 4.5, 6.2 };

class TGADataTest : public ::testing::Test, public TGAData {
public:
  /** constructor. sets up some TGraphAsymmErrors for testing. */
  TGADataTest() {
    m_testGraphs.push_back( new TGraphAsymmErrors(xCoords1.size(), xCoords1.data(), yCoords1.data(), errors1.data(), errors1.data(),
                                                  errors1.data(), errors1.data()) );
    m_testGraphs.push_back( new TGraphAsymmErrors(xCoords2.size(), xCoords2.data(), yCoords2.data(), errors2.data(), errors2.data(),
                                                  errors2.data(), errors2.data()) );
  }

  virtual ~TGADataTest() {
    for (auto * graph : m_testGraphs) delete graph;
  }

protected:
  std::vector<TGraphAsymmErrors*> m_testGraphs{};
};

/** Check if the added graph is indeed contained in the internal storage of the TGAData object. */
TEST_F(TGADataTest, addGraphTest)
{
  // add the graph the TGAData we are deriving of to have access to its protected m_points member
  this->addGraph(m_testGraphs[0]);
  EXPECT_EQ(m_testGraphs[0]->GetN(), m_points.size());
  this->addGraph(m_testGraphs[1]);
  EXPECT_EQ(xCoords1.size() + xCoords2.size(), m_points.size());

  // check if the x- & y-coordinates are set correctly (Assuming that the rest is ok if these two are correct.)
  for (size_t i = 0; i < xCoords1.size(); ++i) {
    double x, y;
    EXPECT_EQ(i, m_testGraphs[0]->GetPoint(i, x, y));
    EXPECT_DOUBLE_EQ(x, m_points[i].x);
    EXPECT_DOUBLE_EQ(y, m_points[i].y);
    EXPECT_DOUBLE_EQ(xCoords1[i], m_points[i].x);
    EXPECT_DOUBLE_EQ(yCoords1[i], m_points[i].y);
  }

  for (size_t i = xCoords1.size(); i < xCoords1.size() + xCoords2.size(); ++i) {
    int j = i - xCoords1.size();
    double x,y;
    EXPECT_EQ(j, m_testGraphs[1]->GetPoint(j, x, y));
    EXPECT_DOUBLE_EQ(x, m_points[i].x);
    EXPECT_DOUBLE_EQ(y, m_points[i].y);
    EXPECT_DOUBLE_EQ(xCoords2[j], m_points[i].x);
    EXPECT_DOUBLE_EQ(yCoords2[j], m_points[i].y);
  }
}

/** Check if the getValues function returns the correct vector. */
TEST_F(TGADataTest, getValuesTest)
{
  this->addGraph(m_testGraphs[0]);
  this->addGraph(m_testGraphs[1]);

  auto getXCoord = [] (const Point& p) { return p.x; };
  std::vector<double> xCoords = this->getValues(getXCoord);
  for (size_t i = 0; i < xCoords.size(); ++i) {
    if (i < xCoords1.size()) {
      EXPECT_DOUBLE_EQ(xCoords1[i], xCoords[i]);
    } else {
      EXPECT_DOUBLE_EQ(xCoords2[i - xCoords1.size()], xCoords[i]);
    }
  }
}


/** Check if the returned TGraphAsymmErrors is the one that is stored internally. */
TEST_F(TGADataTest, getTGATest)
{
  this->addGraph(m_testGraphs[0]);
  TGraphAsymmErrors* graph = this->getTGA();

  for (int i = 0; i < m_testGraphs[0]->GetN(); ++i) {
    double xe, ye; // the expected values
    double xa, ya; // the actual values

    EXPECT_EQ(i, m_testGraphs[0]->GetPoint(i, xe, ye));
    EXPECT_EQ(i, graph->GetPoint(i, xa, ya));

    EXPECT_DOUBLE_EQ(xe, xa);
    EXPECT_DOUBLE_EQ(ye, ya);

    // all errors are the same in this test cases so checking one should be enough
    EXPECT_DOUBLE_EQ(m_testGraphs[0]->GetErrorXlow(i), graph->GetErrorXlow(i));
  }
  delete graph; // cleanup

  // TODO: check if sorting works appropriately
}

/** this is here as a reminder to implement it, once a decision has been made on to which function is more performant. */
TEST_F(TGADataTest, getTGAVectorsTest)
{
  // TODO
}


int main (int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
