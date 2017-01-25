#!/usr/bin/env python

import sys
import argparse


from utils.recurse import collectHistograms, TH2DCollector

def projectorX(h, ending):
    return h.ProjectionX(ending)

def projectorY(h, ending):
    return h.ProjectionY(ending)


def getProjections(hists, projector, ending):
    """
    """
    return [projector(h, "_".join([n, ending])) for (n,h) in hists.iteritems()]


def storeInFile(f, hs):
    f.cd()
    for h in hs: h.Write()


"""
Argparse
"""
parser = argparse.ArgumentParser(description="Script for creating 1D projections along the X "
                                 "and Y axis of all TH2Ds in a given root file")
parser.add_argument('inputFile', help="Input file")
parser.add_argument('-r', '--histrgx', help="Regex the histogram has to match in order to be used",
                    dest="histRgx", action="store", default="")
parser.add_argument('-n', '--create-new', help="Create a new file instead of updating the input file",
                    dest="updateFile", action="store_false", default=True)
parser.add_argument("outputFile", help="Output file name (only used when --create-new is used)",
                    nargs="?", default="")

args = parser.parse_args()


"""
Script
"""
from ROOT import TFile, TH2D, gROOT
gROOT.SetBatch()

if not args.updateFile:
    f = TFile.Open(args.inputFile)
    if not args.outputFile:
        print("Need an output file name if \'--create-new\' is set")
        sys.exit(1)
    of = TFile(args.outputFile, "recreate")

else:
    f = TFile.Open(args.inputFile, "update")
    of = f

if f and of:
    hists = collectHistograms(f, args.histRgx, TH2DCollector)
    projX = getProjections(hists, projectorX, "costh")
    projY = getProjections(hists, projectorY, "phi")

    storeInFile(of, projX)
    storeInFile(of, projY)

    f.Close()
    if of:
        of.Close()
