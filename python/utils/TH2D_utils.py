from math import sqrt

def divide2D(h, g, name = ""):
    """
    Divide histogram (TH2D) h by g doing an error propagation for each bin by calculating
    relative errors of the bins in each histogram and adding them in quadrature to get the relative
    error of the ratio.

    Passed histograms are not changed by a call to this function, since they get cloned before
    anything happens and a new histogram is returned.
    """
    n = h.Clone()
    d = g.Clone()

    if n.GetNbinsX() != d.GetNbinsX() or n.GetNbinsY() != d.GetNbinsY():
        raise NotDivisable(n, d)

    # divide histograms bin by bin
    for i in range(0, n.GetNbinsX() + 2):
        for j in range(0, n.GetNbinsY() + 2):
            nBin = Bin(n.GetBinContent(i,j), n.GetBinError(i,j))
            dBin = Bin(d.GetBinContent(i,j), d.GetBinError(i,j))
            if dBin.cont <= 0:
                if nBin > 0:
                    print("bin ({},{}) set to zero to avoid division by 0. "
                          "Numerator was {}".format(i, j, nBin.cont))
                nBin = Bin(0, 0) # if denominator has no entries, also set the numerator to 0
            else:
                nBin.divide(dBin)

            nBin.setBin(n, i, j)

    if name:
        n.SetName(name)

    return n

class NotDivisable(Exception):
    """Exception raised when two histograms are not divisable"""
    def __init__(self, h, g):
        self.hBins = [h.GetNbinsX(), h.GetNbinsY()]
        self.gBins = [g.GetNbinsX(), g.GetNbinsY()]
    def __str__(self):
        return "Histograms are not divisable. Binnings: {} vs. {}".format(self.hBins, self.gBins)


class Bin:
    """Helper class that does the main calculations"""
    def __init__(self, cont, err):
        """Construct and set relative bin error if content is not 0"""
        self.cont = cont
        self.err = err
        self.relErr2 = 0
        if self.cont > 0:
            self.relErr2 = self.err**2 / self.cont**2

    def divide(self, otherBin):
        """Divide this bin by the otherBin with proper error propagation without handling division by 0!"""
        self.cont = self.cont / otherBin.cont
        self.relErr2 = self.relErr2 + otherBin.relErr2
        self.err = sqrt(self.relErr2) * self.cont

    def setBin(self, h, i, j):
        """Set bin to the histogram passed histogram"""
        h.SetBinContent(i,j, self.cont)
        h.SetBinError(i,j, self.err)


def compareCoverage(h, g, name = ""):
    """
    Check the coverage of the two TH2Ds.

    The meaning of the different values that are assigned to the new histogram can be found at the
    definition of getCoverage()
    """
    c = h.Clone()
    for i in range(0, c.GetNbinsX() + 2):
        for j in range(0, c.GetNbinsY() + 2):
            coverage = getCoverage(c, g, i, j)
            c.SetBinContent(i, j, coverage)

    if name:
        c.SetName(name)
    return c


def getCoverage(h, g, i, j):
    """
    Check if bin (i,j) in histograms h and g are filled

    Bins that are filled in h but not in g, get assigned +1, respectively bins that are filled in h
    but not in g get assigned -1. Bins that are filled in both are assigned 0, bins that are filed
    in neither are assigned -2.
    """
    hCont = h.GetBinContent(i,j) # need no errors for this
    gCont = g.GetBinContent(i,j)

    if hCont > 0 and gCont > 0: return 0
    if hCont == 0 and gCont == 0: return -2
    if hCont > 0 and gCont == 0: return 1
    if hCont == 0 and gCont > 0: return -1

    print("getCoverage fell through all cases it can handle with values {} and {}. This should not happen."
          "Maybe something is wrong with the inputs?".format(hCont, gCont))
    return -1000


def printTH2D(h):
    """Print the TH2D as an NxM matrix, directly to the screen"""
    print("Contents of: {}, ({} x {})".format(h.GetName(), h.GetNbinsX(), h.GetNbinsY()))
    for i in range(0, h.GetNbinsX() + 2):
        row = []
        for j in range(0, h.GetNbinsY() + 2):
            row.append(h.GetBinContent(i,j))
        print(" ".join(str(v) for v in row))


def drawTH2DColMap(h, c):
    """
    Drawing a TH2D as color map requires some tweaking for a (more or less) nice result
    Most importantly the color axis has to be moved to the left for readable axis labels
    """
    from ROOT import gPad, TPaletteAxis
    c.SetRightMargin(0.175) # make some space for color axis
    c.cd()
    h.Draw("colz")
    c.Update() # draw first, since the TPaletteAxis won't be available else
    p = h.GetListOfFunctions().FindObject("palette")
    p.SetX1NDC(0.84) # move to the right
    p.SetX2NDC(0.89)
    gPad.Modified()
    gPad.Update()
