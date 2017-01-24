#!/usr/bin/env python

import argparse
import os
import re

from utils.reportGeneration import makeFigTex, getLabels, sortRapPt

def addPlots(plots):
    """
    Simply put all plots into one figure
    NOTE: this breaks VERY easily
    """
    if "rap" in plots[0] or "pt" in plots[0].lower():
        plots.sort(key=sortRapPt)

    return makeFigTex(plots, getLabels(plots))


"""
Argparse
"""
parser = argparse.ArgumentParser(description="Script for generating a very basic report by "
                                 "by simply plotting all passed files into a .tex file")
parser.add_argument("plots", nargs="+", help="all of the plots that should be put into the report")
parser.add_argument("--outputfile", "-o", help="name of the created output tex file",
                    dest="outfile", action="store", default="quickReport.tex")

args = parser.parse_args()

"""
Script
"""
# hardcoded for the moment
baseTexFile = open(os.path.join(os.environ["PHYS_UTILS_DIR"], "Latex/reportQuickBase.tex"))

tex = []
for line in baseTexFile:
    tex.append(line)
    if "begin" in line:
        for plotLines in addPlots(args.plots):
            tex.append(plotLines)

baseTexFile.close()

with open(args.outfile, 'w') as f:
    for l in tex: f.write(l)
