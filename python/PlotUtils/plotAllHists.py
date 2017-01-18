#!/usr/bin/env python

import argparse
import re
from utils.recurse import recurseOnFile
from utils.TH2D_utils import drawTH2DColMap

class HistSaverIfMatch(object):
    """Functor for saving all (toplevel) histograms in a TFile."""

    def __init__(self, regex, extension = ".pdf", outpath="./"):
        self.rgx = re.compile(regex)
        self.ext = extension
        self.path = outpath
        self.can = TCanvas("saveCanvas", "saveCanvas", 1000, 1000)

    def __call__(self, obj):
        """
        Provide required interface for recurseOnFile call.
        Save all objects inheriting from 'TH1' and use their name as filename with the
        extension specified in the constructor as file-ending.
        """
        if obj.InheritsFrom("TH1") and self.rgx.search(obj.GetName()):
            self.can.Clear()
            self.can.cd()
            obj.SetStats(False)
            if obj.InheritsFrom("TH2"):
                drawTH2DColMap(obj, self.can)
            else:
                obj.Draw()

            self.can.SaveAs("".join([self.path, obj.GetName(), self.ext]))


"""
Argparse
"""
parser = argparse.ArgumentParser(description="Script for plotting and saving all histograms in a file.")
parser.add_argument("inputFile", help="File from which all histograms should be saved")
parser.add_argument("--histrgx", help="Regex a histogram name has to match in order to be saved.",
                    dest="histRgx", action="store")
parser.add_argument("--output-path", "-o", help="Path to which the created files should be saved.",
                    dest="outPath", action="store")
parser.add_argument("--extension", "-e", help="Extension to be used for storing plots (default = .pdf)",
                    dest="extension", action="store")

parser.set_defaults(histRgx="", outPath="./", extension=".pdf")
args = parser.parse_args()


"""
Script
"""
from ROOT import TFile, TH1D, TH2D, TCanvas, gROOT
gROOT.SetBatch()
gROOT.ProcessLine("gErrorIgnoreLevel = 1001") # don't litter stdout

f = TFile.Open(args.inputFile)
histSaver = HistSaverIfMatch(args.histRgx, args.extension, args.outPath)
recurseOnFile(f, histSaver)

f.Close()
