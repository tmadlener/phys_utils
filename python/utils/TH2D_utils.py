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



def printTH2D(h):
    """Print the TH2D as an NxM matrix, directly to the screen"""
    print("Contents of: {}, ({} x {})".format(h.GetName(), h.GetNbinsX(), h.GetNbinsY()))
    for i in range(0, h.GetNbinsX() + 2):
        row = []
        for j in range(0, h.GetNbinsY() + 2):
            row.append(h.GetBinContent(i,j))
        print(" ".join(str(v) for v in row))
