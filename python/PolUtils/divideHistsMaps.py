#!/usr/bin/env python

## script for dividing all histograms with the same ending in two given files and storing
## the results in a new file.
## Naturally geared towards the use-cases of Polarization measurements

import math
import argparse
from functools import partial

from utils.recurse import collectHistograms, TH2DCollector, TH1DCollector
from utils.TH_utils import divide2D, compareCoverage, divide1D
from utils.miscHelpers import getRapPtStr, getRapPt, removeRapPt, filterDict, filterDictNot

def divideHistograms(numHists, denomHists, func = divide2D, algoName = ""):
    """
    Divide all numHists by the denomHists based on their (rap, pt) info
    """
    ratios = {}
    for (denomN, denomH) in denomHists.iteritems():
        rapPt = getRapPt(denomN)
        # get the list of all numerator candidates the have the same rap and pt bin
        # this should be enough for disambiguation, otherwise you have to filter before
        numH = [h for n, h in numHists.items() if rapPt == getRapPt(n)]
        if len(numH) == 1:
            ratioN = "_".join([algoName, removeRapPt(numH[0].GetName()), "",
                               removeRapPt(denomN), getRapPtStr(denomN)])
            if ratioN in ratios:
                print("WARNING: {} already present in ratios. Replacing it.".format(ratioN))
            ratios[ratioN] = func(numH[0], denomH, ratioN)
        else:
            print("Could not get (unambiguous) numerator histogram for {}. Got {} possible"
                  "candidates. Skipping this histogram.".format(denomN, len(numH)))

    return ratios


def storeRatioHists(f, hists, baseName = ""):
    """
    Store passed histograms to passed file with the passed baseName
    """
    f.cd()
    for (name, h) in hists.iteritems():
        if baseName:
            h.SetName("_".join([baseName, getRapPtStr(name)]))
        h.Write()


"""
Argparse
"""
parser = argparse.ArgumentParser(description="Script for dividing TH2Ds in different root "
                                 "files and storing the resulting histograms.",
                                 epilog="The script divides all histograms found in the numerator file "
                                 "by the histograms found in the denominator file, if the histograms names "
                                 "both end in \'_rapX_ptY\', where \'rap\' and \'pt\' are matched case insensitive.")
parser.add_argument("numeratorFile", help="Path to the root file containing the numerator hists")
parser.add_argument("denominatorFile", help="Path to the root file containing the denominator hists")
parser.add_argument("outputFile", help="File in which the ratio histograms should be stored.")
parser.add_argument("--numerator-base", "-n", dest="numeratorBase", help="The base name (without the matching ending) "
                    "of the numeratr histograms. Leave empty to just match according to the ending.",
                    action="store")
parser.add_argument("--denominator-base", "-d", dest="denominatorBase", help="The base name (without the matching ending) "
                    "of the numeratr histograms. Leave empty to just match according to the ending.",
                    action="store")
parser.add_argument("--output-base", "-o", dest="outputBase", help="The base name for the output histograms. "
                    "If left empty it will be created automatically from the numerator and denominator bases.",
                    action="store")
parser.add_argument("--create-covmap", dest="createCovMap", help="Create a coverage map, for checking which "
                    "if bins had to be set to 0 in the division. These maps are stored in the output file.",
                    action="store_true")
parser.add_argument("--no-ratios", dest="divideHists", help="Do not create the ratio histograms",
                    action="store_false")
parser.add_argument("--normalize", dest="normHists", help="Normalize histograms to 1 before dividing them",
                    action="store_true")
parser.add_argument("--relerr-cut", "-rc", help="Specify a (lower) cut on the relative error of the "
                    "ratio. If the relative error is above this value, the bin will be set to zero.",
                    dest="relErrCut", default=None, action="store", type=float)

parser.set_defaults(numeratorBase="", denominatorBase="", outputBase="",
                    createCovMap=False, divideHists=True, normHists=False)
args = parser.parse_args()


"""
Script
"""
# import ROOT stuff here, so that it doesn't mess up argparse
from ROOT import TFile, TH2D, gROOT
gROOT.SetBatch()

numF = TFile.Open(args.numeratorFile)
denomF = TFile.Open(args.denominatorFile)

# first process all the TH2Ds in the file
numHists = collectHistograms(numF, args.numeratorBase, TH2DCollector)
denomHists = collectHistograms(denomF, args.denominatorBase, TH2DCollector)

outputF = TFile(args.outputFile, "recreate")
if args.divideHists:
    divide = partial(divide2D, normalize=args.normHists, norm = 1, relErrCut = args.relErrCut)
    ratioHists = divideHistograms(numHists, denomHists, divide, "ratio")
    storeRatioHists(outputF, ratioHists, args.outputBase)

if args.createCovMap:
    covMaps = divideHistograms(numHists, denomHists, compareCoverage, "covmap")
    ofbase = "" # cannot simply pass args.outputBase, since that will overwrite possibly present ratio hists
    if args.outputBase:
        ofbase = "_".join(["covmap", args.outputBase])
    storeRatioHists(outputF, covMaps, ofbase)


# now do the TH1Ds
numHists = collectHistograms(numF, args.numeratorBase, TH1DCollector)
denomHists = collectHistograms(denomF, args.denominatorBase, TH1DCollector)

# TODO:
# * geared towards costh and phi projection usage -> make this a bit more versatile

# process the _phi_ and _costh_ histograms explicitly here
for proj in ["costh", "phi"]:
    projRgx = r"_" + proj + r"(_|$)"
    ratioHists = divideHistograms(filterDict(numHists, projRgx),
                                  filterDict(denomHists, projRgx),
                                  divide1D, "ratio")
    storeRatioHists(outputF, ratioHists,
                    "_".join([args.outputBase, proj]))

costhphiRgx = r"_(costh|phi)(_|$)"
# Filter out all previously stored histograms and process them now
ratioHists = divideHistograms(filterDictNot(numHists, costhphiRgx),
                              filterDictNot(denomHists, costhphiRgx),
                                            divide1D, "ratio")
storeRatioHists(outputF, ratioHists, args.outputBase)


outputF.Close()
numF.Close()
denomF.Close()
