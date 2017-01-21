#!/usr/bin/env python

## script for dividing all histograms with the same ending in two given files and storing
## the results in a new file.
## Naturally geared towards the use-cases of Polarization measurements

import math
import argparse
from functools import partial

from utils.recurse import collectHistograms
from utils.TH2D_utils import divide2D, compareCoverage
from utils.miscHelpers import getRapPtStr

def divideHistograms(numHists, denomHists, func = divide2D, algoName = ""):
    """
    Divide all numHists by the denomHists based on their name endings
    """
    ratios = {}
    for (denomN, denomH) in denomHists.iteritems():
        rapPt = getRapPtStr(denomN)
        numH = [h for n, h in numHists.items() if n.lower().endswith(rapPt)]
        if len(numH) == 1:
            ratioN = "_".join([algoName, numH[0].GetName().replace(rapPt, ""), "", denomN.replace(rapPt, ""), rapPt])
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

numHists = collectHistograms(numF, args.numeratorBase)
denomHists = collectHistograms(denomF, args.denominatorBase)

outputF = TFile(args.outputFile, "recreate")
if args.divideHists:
    divide = partial(divide2D, normalize=args.normHists, norm = 1)
    ratioHists = divideHistograms(numHists, denomHists, divide, "ratio")
    storeRatioHists(outputF, ratioHists, args.outputBase)

if args.createCovMap:
    covMaps = divideHistograms(numHists, denomHists, compareCoverage, "covmap")
    ofbase = "" # cannot simply pass args.outputBase, since that will overwrite possibly present ratio hists
    if args.outputBase:
        ofbase = "_".join(["covmap", args.outputBase])
    storeRatioHists(outputF, covMaps, ofbase)


outputF.Close()
numF.Close()
denomF.Close()
