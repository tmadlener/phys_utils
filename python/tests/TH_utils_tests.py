#!/usr/bin/env python

import unittest
from math import sqrt

from utils.TH_utils import *
from ROOT import TH2D, TH1D

"""
Bin
"""
class TestBinTestCase(unittest.TestCase):
    def setUp(self):
       self.b = Bin(1, 1)
       self.c = Bin(0, 0.5)
       self.d = Bin(2.5, 1.6)
       self.f = Bin(1.2, 0.9)


class TestBinConstructor(TestBinTestCase):
    def runTest(self):
        self.assertEqual(self.b.cont, 1)
        self.assertEqual(self.b.err, 1)
        self.assertEqual(self.b.relErr2, 1)

        self.assertEqual(self.c.cont, 0)
        self.assertEqual(self.c.err, 0.5)
        self.assertEqual(self.c.relErr2, 0)

        self.assertAlmostEqual(self.d.relErr2, 1.6**2 / 2.5**2)


class TestBinDivide(TestBinTestCase):
    def runTest(self):
        self.f.divide(self.d)
        self.assertAlmostEqual(self.f.cont, 0.48) # 1.2 / 2.5 = 0.48
        self.assertAlmostEqual(self.f.relErr2, 0.9721) # (0.9 / 1.2) ^ 2 + (1.6 / 2.5) ^ 2 = 0.9721
        self.assertAlmostEqual(self.f.err, sqrt(0.9721) * 0.48)


class TestBinDivideByZero(TestBinTestCase):
    def runTest(self):
        self.b.divide(self.c)
        self.assertEqual(self.b.cont, 0)
        self.assertEqual(self.b.err, 0)


class TestBinSetBin(TestBinTestCase):
    def runTest(self):
        h = TH2D("", "", 2, 0, 3, 1, 0, 3)
        self.b.setBin(h, 1, 1)
        self.assertEqual(h.GetBinContent(1, 1), self.b.cont)
        self.assertEqual(h.GetBinError(1, 1), self.b.err)

        self.f.setBin(h, 2, 1)
        self.assertEqual(h.GetBinContent(2, 1), self.f.cont)
        self.assertEqual(h.GetBinError(2, 1), self.f.err)


"""
divide2D
"""
class TestDivide2DTestCase(unittest.TestCase):
    def setUp(self):
        self.h1 = TH2D("h1", "h1", 2, 0, 10, 2, 0, 10)
        Bin(2, 1).setBin(self.h1, 1, 1)
        Bin(3, 0.5).setBin(self.h1, 1, 2)
        Bin(3, 2).setBin(self.h1, 2, 1)
        Bin(4, 1).setBin(self.h1, 2, 2)

        self.g = TH2D("g", "g", 2, 0, 10, 2, 0, 10)
        Bin(8, 0).setBin(self.g, 1, 1)
        Bin(7, 0).setBin(self.g, 1, 2)
        Bin(3, 0).setBin(self.g, 2,1)
        Bin(6, 0).setBin(self.g, 2, 2)

        self.h2 = TH2D("h2", "h2", 2, 0, 10, 2, 0, 10)
        Bin(8, 0.5).setBin(self.h2, 1, 1)
        Bin(7, 0.75).setBin(self.h2, 1, 2)
        Bin(3, 4.5).setBin(self.h2, 2, 1)
        Bin(6, 6.2).setBin(self.h2, 2, 2)


class TestDivide2DSetup(TestDivide2DTestCase):
    def runTest(self):
        self.assertEqual(self.h1.GetBinContent(1, 1), 2)
        self.assertEqual(self.h1.GetBinError(1, 1), 1)
        self.assertEqual(self.h1.GetBinContent(1, 2), 3)
        self.assertEqual(self.h1.GetBinError(1, 2), 0.5)
        self.assertEqual(self.h1.GetBinContent(2, 1), 3)
        self.assertEqual(self.h1.GetBinError(2, 1), 2)
        self.assertEqual(self.h1.GetBinContent(2, 2), 4)
        self.assertEqual(self.h1.GetBinError(2, 2), 1)

        self.assertEqual(self.g.GetBinContent(1, 1), 8)
        self.assertEqual(self.g.GetBinContent(1, 2), 7)
        self.assertEqual(self.g.GetBinContent(2, 1), 3)
        self.assertEqual(self.g.GetBinContent(2, 2), 6)
        for i in range(1, 3):
            for j in range(1, 3):
                self.assertEqual(self.g.GetBinError(i,j), 0)


class TestDivide2DSetName(TestDivide2DTestCase):
    def runTest(self):
        self.assertEqual(divide2D(self.h1, self.g, "foo").GetName(), "foo")


class TestDivide2DExactDenom(TestDivide2DTestCase):
    def runTest(self):
        r = divide2D(self.h1, self.g)
        self.assertAlmostEqual(r.GetBinContent(1,1), 2/float(8))
        self.assertAlmostEqual(r.GetBinContent(1,2), 3/float(7))
        self.assertAlmostEqual(r.GetBinContent(2,1), 3/float(3))
        self.assertAlmostEqual(r.GetBinContent(2,2), 4/float(6))

        self.assertAlmostEqual(r.GetBinError(1,1), 1/float(8))
        self.assertAlmostEqual(r.GetBinError(1,2), 0.5/float(7))
        self.assertAlmostEqual(r.GetBinError(2,1), 2/float(3))
        self.assertAlmostEqual(r.GetBinError(2,2), 1/float(6))


class TestDivide2DErrorPropagation(TestDivide2DTestCase):
    def runTest(self):
        # since the bin contents are the same as for the ExactDenom test case
        # only checking the errors here
        r = divide2D(self.h1, self.h2)
        self.assertAlmostEqual(r.GetBinError(1,1), 2/float(8) * sqrt((0.5/8)**2 + (1/float(2)**2)))
        self.assertAlmostEqual(r.GetBinError(1,2), 3/float(7) * sqrt((0.75/7)**2 + (0.5/3)**2))
        self.assertAlmostEqual(r.GetBinError(2,1), 3/float(3) * sqrt((4.5/3)**2 + (2/float(3))**2))
        self.assertAlmostEqual(r.GetBinError(2,2), 4/float(6) * sqrt((6.2/6)**2 + (1/float(4))**2))


class TestDivide2DHoleIsZero(TestDivide2DTestCase):
    def runTest(self):
        self.h2.SetBinContent(1, 1, 0) # set bin to 0, but leave error filled in, since this has to be handled correctly
        r = divide2D(self.h1, self.h2)
        self.assertEqual(r.GetBinContent(1,1), 0)
        self.assertEqual(r.GetBinError(1,1), 0)


class TestDivide2DNotDivisable(TestDivide2DTestCase):
    def runTest(self):
        h = TH2D("","", 1,0,1,2,0,1)
        with self.assertRaises(NotDivisable):
            divide2D(self.g, h)

"""
divide1D
"""
class TestDivide1DTestCase(unittest.TestCase):
    def setUp(self):
        self.h = TH1D("h", "h", 3, 0, 10)
        Bin(1, 0.5).setBin1D(self.h, 1)
        Bin(2, 0.25).setBin1D(self.h, 2)
        Bin(3, 1.0/9.0).setBin1D(self.h, 3)

        self.g = TH1D("g", "g", 3, 0, 10)
        Bin(3, 0.3).setBin1D(self.g, 1)
        Bin(0, 0.2).setBin1D(self.g, 2)
        Bin(2, 0.4).setBin1D(self.g, 3)


class TestDivide1DSetName(TestDivide1DTestCase):
    def runTest(self):
        self.assertEqual(divide1D(self.h, self.g, "foo").GetName(), "foo")


class TestDivide1DErrorPropagation(TestDivide1DTestCase):
    def runTest(self):
        r = divide1D(self.h, self.g)
        self.assertAlmostEqual(r.GetBinError(1), 1.0/3.0 * sqrt(0.5**2 + (0.3/3)**2))
        self.assertAlmostEqual(r.GetBinError(3), 3.0/2.0 * sqrt(1/27.0**2 + (0.2)**2))
        # second bin is covered by other test!


class TestDivide1DRatios(TestDivide1DTestCase):
    def runTest(self):
        r = divide1D(self.h, self.g)
        self.assertAlmostEqual(r.GetBinContent(1), 1.0/3.0)
        self.assertEqual(r.GetBinContent(3), 3.0/2.0)
        # second bin covered by other test


class TestDivide1DHoleIsZero(TestDivide1DTestCase):
    def runTest(self):
        r = divide1D(self.h, self.g)
        self.assertEqual(r.GetBinContent(2), 0)
        self.assertEqual(r.GetBinError(2), 0)


class TestDivide1DNotDivisable(TestDivide1DTestCase):
    def runTest(self):
        f = TH1D("", "", 1, 0, 1)
        with self.assertRaises(NotDivisable):
            divide1D(self.h, f)


if __name__ == "__main__":
    unittest.main()
