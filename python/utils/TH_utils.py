from math import sqrt

def normalize2D(h, N = 1):
    """
    Normalize histogram to N (defaults to 1)
    Return the passed histogram
    NOTE: This alters the passed histogram
    """
    # using TH2D.Scale() sets the errors to sqrt(content) it they have not been set manually before!
    # Thus, defining own normalization, where the errors are scaled by the same factor as the bin contents
    norm = N / h.Integral()
    for i in range(0, h.GetNbinsX() + 2): # also scale the over- and underflow bins
        for j in range(0, h.GetNbinsY() + 2):
            cont = h.GetBinContent(i,j)
            h.SetBinContent(i,j, cont * norm)
            err = h.GetBinError(i,j)
            h.SetBinError(i,j, err * norm)

    return h


def divide2D(h, g, name = "", normalize = False, norm = 1, relErrCut = None):
    """
    Divide histogram (TH2D) h by g doing an error propagation for each bin by calculating
    relative errors of the bins in each histogram and adding them in quadrature to get the relative
    error of the ratio.

    optional arguments:
    * name: name of returned histogram
    * normalize: normalize histograms before dividing
    * norm: normalization to which the histograms get normalized if desired

    Passed histograms are not changed by a call to this function, since they get cloned before
    anything happens and a new histogram is returned.
    """
    n = h.Clone()
    d = g.Clone()

    if n.GetNbinsX() != d.GetNbinsX() or n.GetNbinsY() != d.GetNbinsY():
        raise NotDivisable(n, d)

    if normalize:
        n = normalize2D(n, norm)
        d = normalize2D(d, norm)

    # divide histograms bin by bin
    for i in range(0, n.GetNbinsX() + 2):
        for j in range(0, n.GetNbinsY() + 2):
            nBin = Bin(n.GetBinContent(i,j), n.GetBinError(i,j))
            dBin = Bin(d.GetBinContent(i,j), d.GetBinError(i,j))
            nBin.divide(dBin, relErrCut)
            nBin.setBin(n, i, j)

    if name:
        n.SetName(name)

    return n


def divide1D(h, g, name = ""):
    """
    Divide TH1D h by TH1D g.
    """
    n = h.Clone()
    d = g.Clone()

    if n.GetNbinsX() != d.GetNbinsX():
        raise NotDivisable(h, g)

    for i in range(0, n.GetNbinsX() + 2):
        nBin = Bin(n.GetBinContent(i), n.GetBinError(i))
        dBin = Bin(d.GetBinContent(i), d.GetBinError(i))
        nBin.divide(dBin)
        nBin.setBin1D(n, i)

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

    def __repr__(self):
        return "{} +/- {}, relErr2 = {}".format(self.cont, self.err, self.relErr2)


    def divide(self, otherBin, relErrCut = None):
        """
        Divide this bin by the otherBin with proper error propagation.
        If the denominator bins content is zero, the ratio is set to 0 with 0 error
        to avoid division by 0
        !"""
        if otherBin.cont == 0:
            if self.cont != 0:
                print("Bin set to zero to avoid division by 0. "
                      "Numerator was {}".format(self.cont))
            self.cont = 0
            self.relErr2 = 0
        else:
            self.cont = self.cont / otherBin.cont
            self.relErr2 = self.relErr2 + otherBin.relErr2

            if relErrCut and self.relErr2 > relErrCut**2:
                print("Bin set to zero, due to resulting rel. err = {} (> {}). "
                      "Ratio was {}".format(sqrt(self.relErr2), relErrCut, self.cont))
                self.cont = 0
                self.relErr = 0

        self.err = sqrt(self.relErr2) * self.cont

        return self


    def setBin(self, h, i, j):
        """Set bin to the passed histogram (TH2D)"""
        h.SetBinContent(i,j, self.cont)
        h.SetBinError(i,j, self.err)

    def setBin1D(self, h, i):
        """Set bin to the passed histogram (TH1D)"""
        h.SetBinContent(i, self.cont)
        h.SetBinError(i, self.err)


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

    Bins that are filled in h but not in g, get assigned +1, respectively bins that are filled in g
    but not in h get assigned +2. Bins that are filled in both are assigned 3, bins that are filed
    in neither are assigned 0.
    """
    hCont = h.GetBinContent(i,j) # need no errors for this
    gCont = g.GetBinContent(i,j)

    if hCont > 0 and gCont > 0: return 3
    if hCont == 0 and gCont == 0: return 0
    if hCont > 0 and gCont == 0: return 1
    if hCont == 0 and gCont > 0: return 2

    print("getCoverage fell through all cases it can handle with values {} and {}. This should not happen."
          "Maybe something is wrong with the inputs?".format(hCont, gCont))
    return -1000


def printTH2D(h):
    """Print the TH2D as an NxM matrix, directly to the screen"""
    print("Contents of: {}, ({} x {})".format(h.GetName(), h.GetNbinsX(), h.GetNbinsY()))
    for i in range(0, h.GetNbinsY() + 2):
        row = []
        for j in range(0, h.GetNbinsX() + 2):
            row.append(h.GetBinContent(j,i))
        print(" ".join(str(v) for v in row))


def printTH1D(h):
    """Print the TH1D as a list, directly to the screen"""
    print("Contents of: {}, ({})".format(h.GetName(), h.GetNbinsX()))
    for i in range(0, h.GetNbinsX() + 2):
        print("{} +/- {}".format(h.GetBinContent(i), h.GetBinError(i)))


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


def drawTH2DErrColMap(h, c):
    """
    Draw the relative errors of a TH2D as color map
    NOTE: this modifies the passed histogram
    """
    eh = h.Clone()
    for i in range(0, eh.GetNbinsX() + 2):
        for j in range(0, eh.GetNbinsY() + 2):
            con = eh.GetBinContent(i, j)
            if con > 0:
                h.SetBinContent(i, j, eh.GetBinError(i, j) / con)
            else:
                h.SetBinContent(i, j, 0)

    h.SetName("_".join([h.GetName(), "relErr"]))
    drawTH2DColMap(h, c)


def rescaleHistToMax(h):
    """
    Rescale the histogram such that the maximum bin has content 1
    """
    maxBinCont = h.GetBinContent(h.GetMaximumBin())
    if maxBinCont > 0:
        h.Scale(1 / maxBinCont)
    return h


def rescaleHistToNorm(h, N = 1.0):
    """
    Rescale the histogram such that its integral is N
    """
    h.Scale(N / h.Integral())
    return h

def createTGAE(h):
    """
    Create a TGraphAsymmErrors from the passed histogram
    """
    from utils.TGraph_utils import createGraph
    x = []
    y = []
    ex = []
    ey = []
    for i in range(1, h.GetNbinsX() + 1):
        x.append(h.GetBinCenter(i))
        y.append(h.GetBinContent(i))
        ey.append(h.GetBinError(i))
        ex.append(x[-1] - h.GetBinLowEdge(i))

    return createGraph(x, y, ex, ex, ey, ey)
