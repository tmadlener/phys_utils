#!/usr/bin/env python

import argparse
import re

from utils.recurse import collectHistograms, TH2DCollector
from utils.miscHelpers import getAnyMatchRgx, parseVarBinning
from utils.Fit_utils import createAndStoreGraphs, fitAngularDistribution


def collectLambdas(hists):
    """
    Fit all hists in the passed dict and collect the lambdas into a dictionary with
    the same keys as the histograms
    """
    lambdas = {}
    for (name, h) in hists.iteritems():
        lambdas[name] = fitAngularDistribution(h)

    return lambdas


"""
Argparse
"""
parser = argparse.ArgumentParser(description="Fit cosTheta-Phi histograms with angular distribution"
                                 "and store results as TGraphAsymmErrors in the input file")
parser.add_argument("histFile", help="File containing the histograms to be fit")
parser.add_argument("--histrgx", help="Regex the histogram names have to match to be fitted.",
                    dest="histRgx", action="store")
parser.add_argument("--graphbase", help="Base name for the created graphs.",
                    dest="graphBase", action="store")
parser.add_argument("--varBinning", help="binning of the variable to be used",
                    dest="varBinning", nargs="+", required=True)
parser.add_argument("--binVariable", help="regex of the binned variable", dest="binVarRgx",
                    action="store")
parser.add_argument("--fixedVariable", help="regex of the fixed variable (i.e. each different index of this variable gets a new graph)",
                    dest="fixVarRgx", action="store")

parser.set_defaults(histRgx="", graphBase="fitted", binVarRgx="", fixVarRgx="")


args = parser.parse_args()
varBinning = parseVarBinning(args.varBinning)

"""
Script
"""
from ROOT import TFile, TH2D, TGraphAsymmErrors, gROOT
gROOT.SetBatch()

# don't collect and fit histograms that are not needed
fullMatchRgx = getAnyMatchRgx([args.histRgx, args.fixVarRgx, args.binVarRgx])

inputF = TFile.Open(args.histFile, "update")
histsToFit = collectHistograms(inputF, fullMatchRgx, TH2DCollector)
lambdas = collectLambdas(histsToFit)

createAndStoreGraphs(lambdas, args.graphBase, varBinning, args.binVarRgx, args.fixVarRgx,
                     ["lth", "lph", "ltp"], inputF)

inputF.Close()
