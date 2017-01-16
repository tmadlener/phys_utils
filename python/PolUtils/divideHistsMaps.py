#!/usr/bin/env python

## script for dividing all histograms with the same ending in two given files and storing
## the results in a new file.
## Naturally geared towards the use-cases of Polarization measurements

import math
import argparse

from utils.recurse import recurseOnFile, ObjectCollector
from utils.TH2D_utils import divide2D

class TH2DCollector(ObjectCollector):
    """
    Collector object to collect all TH2Ds from a given file into a dict
    """

    def __init__(self, rgx):
        ObjectCollector.__init__(self, rgx, "TH2D")


def getRapPtStr(fullName):
    """
    Get the rapX_ptY ending of the full name where, all characters are lower cased
    """
    strList = fullName.lower().split("_")
    return "_".join([strList[-2], strList[-1]])


def getPtRap(fullName):
    """
    Get the numerical values of the pt and rapidity bin
    NOTE: assumes that this follows the convention that rapX_ptY are always at the end of the full name
    """
    strList = fullName.lower().split("_")
    return[int(strList[-1][2:]), int(strList[-2][3:])]


def divideHistograms(numHists, denomHists):
    """
    Divide all numHists by the denomHists based on their name endings
    """
    ratios = {}
    for (denomN, denomH) in denomHists.iteritems():
        rapPt = getRapPtStr(denomN)
        numH = [h for n, h in numHists.items() if n.lower().endswith(rapPt)]
        if len(numH) == 1:
            ratioN = "_".join([numH[0].GetName().replace(rapPt, ""), "", denomN.replace(rapPt, ""), rapPt])
            print(ratioN)
            if ratioN in ratios:
                print("WARNING: {} already present in ratios. Replacing it.".format(ratioN))
            ratios[ratioN] = divide2D(numH[0], denomH, ratioN)
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


def collectHistograms(f, basename):
    """
    Collect all TH2Ds from TFile f, whose name matches basename and return them in a dict
    with the names as the keys and the TH2Ds as value
    """
    hColl = TH2DCollector(basename)
    recurseOnFile(f, hColl)
    return hColl.objects


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
parser.add_argument("--numerator-base", dest="numeratorBase", help="The base name (without the matching ending) "
                    "of the numeratr histograms. Leave empty to just match according to the ending.",
                    action="store")
parser.add_argument("--denominator-base", dest="denominatorBase", help="The base name (without the matching ending) "
                    "of the numeratr histograms. Leave empty to just match according to the ending.",
                    action="store")
parser.add_argument("--output-base", dest="outputBase", help="The base name for the output histograms. "
                    "If left empty it will be created automatically from the numerator and denominator bases.",
                    action="store")

parser.set_defaults(numeratorBase="", denominatorBase="", outputBase="")
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
ratioHists = divideHistograms(numHists, denomHists)

outputF = TFile(args.outputFile, "recreate")
storeRatioHists(outputF, ratioHists, args.outputBase)

outputF.Close()
numF.Close()
denomF.Close()
