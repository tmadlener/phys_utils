#!/usr/bin/env python

import argparse
import re

from utils.recurse import collectHistograms, TH1DCollector
from utils.miscHelpers import filterDict, getRapPt
from utils.Fit_utils import createAndStoreGraphs

def fitWcosThetaPhi(hCosth, hPhi):
    """
    Fit the two 1D projections in two steps, by first fitting the costh projection to fix
    lambda_theta and then fit the phi projection to get lambda_phi as well
    """
    lthRes = fitWcosTheta(hCosth)
    lphRes = fitWPhi(hPhi, lthRes["lth"][0])

    return {"lth" : lthRes["lth"],
            "lph": lphRes["lph"]}


def fitWcosTheta(h):
    """Fit the phi integrated angular distribution function to the histogram"""
    W = TF1("Wcosth",
            "[0] * (1 + [1]*x[0]*x[0])", -1.0, 1.0)
    W.SetParameters(1.0, 0.0)

    fitRlt = h.Fit(W, "S")
    fitRlt.SetName("_".join([h.GetName(), "Wcosth_rlt"]))
    fitRlt.Write()

    return{"N":   [fitRlt.Parameter(0), fitRlt.Error(0)],
           "lth": [fitRlt.Parameter(1), fitRlt.Error(1)]}


def fitWPhi(h, lth):
    """Fit the phi integrated angular distribution function to the histogram"""
    W = TF1("Wphi",
            "[0]* (1 + [1] * cos(2*x[0]*0.0174532925))", # [1] is NOT lph!
            -180.0, 180.0)
    W.SetParameters(1.0, 0.0)

    fitRlt = h.Fit(W, "S")
    fitRlt.SetName("_".join([h.GetName(), "Wphi"]))
    fitRlt.Write()

    # store kappa in list, since lph and its error are both calculated
    # via factor 0.5 / (3 + lth) of kappa resp. its error
    kappa = [fitRlt.Parameter(1), fitRlt.Error(1)]
    lph = [0.5 * v / (3 + lth) for v in kappa]

    return {"N":   [fitRlt.Parameter(0), fitRlt.Error(0)],
            "lph": lph}


def collectLambdas(hists):
    """
    Fit all the projections ending with '_costh' and '_phi' with the appropriate
    angular distributed and collect the labmdas into a dictionary with the same keys
    as the histograms
    """
    lambdas = {}
    for (nameCosTh, hCosth) in filterDict(hists, "_costh(_|$)").iteritems():
        hPhi = [h for n, h in filterDict(hists, "_phi(_|$)").iteritems()
                if getRapPt(nameCosTh) == getRapPt(n)]
        if len(hPhi) == 1:
            lambdas[re.sub("_costh(_|$)", "", nameCosTh)] = fitWcosThetaPhi(hCosth, hPhi[0])
        else:
            print("Could not get (unambiguous) numerator histogram for {}. Got {} possible "
                  "candidates. Skipping this histogram.".format(nameCosth, len(numH)))

    return lambdas


"""
Argparse
"""
parser = argparse.ArgumentParser(description='Fit 1D Projections')
parser.add_argument('histFile', help="File containing the histograms to be fit")
parser.add_argument('-hr', '--histrgx', help="Regex the histograms have to match to be fitted",
                    dest="histRgx", action="store", default="")
parser.add_argument("--graphbase", "-g", help="Base name for the created graphs.",
                    dest="graphBase", action="store", default="proj_fit")
parser.add_argument("--ptBinning", help="pt binning that should be used",
                    dest="ptBinning", nargs="+", required=True)


args = parser.parse_args()

ptBinning = [float(v) for v in args.ptBinning]

"""
Script
"""
from ROOT import TFile, TH1D, TGraphAsymmErrors, TF1, gROOT
gROOT.SetBatch()

inputF = TFile.Open(args.histFile, "update")
projections = collectHistograms(inputF, args.histRgx, TH1DCollector)

createAndStoreGraphs(collectLambdas(projections), args.graphBase, ptBinning,
                     ["lth", "lph"], inputF)

inputF.Close()
