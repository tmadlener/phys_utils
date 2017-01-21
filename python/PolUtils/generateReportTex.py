#!/usr/bin/env python

import argparse
import os
import re

from utils.miscHelpers import getAllFilesFromDir, getRapPt, getRapPtLbl

def processLine(line, plots):
    """
    Process the line.
    """
    labels = ["sec:refmap", "sec:corrmap", "sec:dataresults", "sec:sanchecks"]
    section = [s for s in labels if s in line]
    if section:
        return getPlotTexOfSection(section[0], plots)

    return []


def getPlotTexOfSection(section, plots):
    """
    Generate the figure tex of the section
    """
    # hardcoded for the moment (defined mostly in createApplyCorrectionMap.sh)
    sectionFigures = {"sec:refmap": ["refmaps", "refmapTestHists", "refmapTestScatter"],
                      "sec:corrmap": ["corrmaps", "corrmapsData"],
                      "sec:dataresults": ["datamaps", "datamapsCorr", "covmaps"],
                      "sec:sanchecks": ["graphs"]}

    # regexes! (mainly to have beginning and end anchors)
    figurePlots = {"refmaps": "^cosThPhi_refMap",
                   "refmapTestHists": "_residuals.pdf$",
                   "refmapTestScatter": "_scatter.pdf$",
                   "corrmaps": "^correctionMap",
                   "corrmapsData": "^raw_corr_costhphi",
                   "datamaps": "^raw_data_costhphi",
                   "datamapsCorr": "^data_corr_costhphi",
                   "covmaps": "^covmap_data_corr_costhphi",
                   "graphs": "^san_check_l"}

    figureCaptions = {
        "refmaps": "Reference $(\\cos\\theta, \\phi)$ histograms for the different "
        "rapidity and $p_T$ bins created by numerically integrating "
        "$W(\\cos\\theta,\\phi|\\vec{\\lambda})$.",
        "refmapTestHists": "Residuals of injected $\\lambda$s and fitted $\\lambda$s "
        "in the reference maps.",
        "refmapTestScatter": "Scatter plots of the residuals vs the injected $\\lambda$s "
        "in the reference maps.",
        "corrmaps": "$(\\cos\\theta, \\phi)$ correction maps for the different rapidity "
        "and pt bins created by dividing data $(\\cos\\theta,\\phi)$ histograms by the "
        "reference maps.",
        "corrmapsData": "$(\\cos\\theta, \\phi)$ data histograms used in the correction "
        "map creation for the different rapidity and $p_T$ bins.",
        "datamaps": "$(\\cos\\theta, \\phi)$ data histograms before the application "
        "of the correction maps for the different rapidity and $p_T$ bins.",
        "datamapsCorr": "$(\\cos\\theta,\\phi)$ data histograms after the application "
        "of the correction maps for the different rapidity an $p_T$ bins. These are "
        "the maps that get fitted to obtain the result $\\lambda$ parameters.",
        "covmaps": "Comparison of coverage between correction map and data histograms "
        "to which the correction maps are applied. Color coding: $+1$ - Filled in the "
        "data histogram but not in the correction map, $-1$ - Filled in the correction "
        "map but not in the data histogram, $0$ - Filled in both, $-2$ - filled in "
        "neither.",
        "graphs": "Sanity check and results: Fitted $\\lambda$ values for different "
        "maps/histograms: reference - $\\lambda$ values injected into the reference map, "
        "sanity check - $\\lambda$ values after acceptance corrections, results - "
        "$\\lambda$ values of the data after correcting with the correction maps"
    }

    def sortRapPt(it): # sort according to rap and pt bin (rap takes precedence)
        return getRapPt(it)

    plotTex = []
    for figureBase in sectionFigures[section]:
        plotNames = getPlotsOfSection(figurePlots[figureBase], plots)
        if plotNames:
            # check if the plots are sortable by (rap, pt) and sort them if so
            if "rap" in plotNames[0] and "pt" in plotNames[0].lower():
                plotNames.sort(key=sortRapPt)
            plotTex.extend(makeFigTex(plotNames, getLabels(plotNames),
                                      figureCaptions[figureBase]))

    return plotTex


def getPlotsOfSection(sectionBase, plots):
    """
    Get all the plots matching the sectionBase from the plots and return a new list
    """
    rgx = re.compile(sectionBase)
    return [p for p in plots if rgx.search(p)]


def makeFigTex(plots, labels, caption = ""):
    """
    Produce a figure with subfloats (possibly split up into more than one figure)
    """
    tex = []
    tex.append("\\begin{figure}\n")
    nPlotted = 0
    nPlots = len(plots)
    for (plot, label) in zip(plots, labels):
        tex.append(getSubFloatStr(plot, label))
        nPlotted = nPlotted + 1
        if nPlotted % 2 == 0 and nPlotted < nPlots: # two plots per "line"
            tex.append("\n")
        if nPlotted % 6 == 0 and nPlotted < nPlots: # allow for a page break after 6 plots
            tex.append("\\end{figure}\n\n")
            tex.append("\\begin{figure}\n")

    if caption:
        tex.append("".join(["\\caption{", caption, "}\n"]))
    tex.append("\\end{figure}\n\n")

    return tex


def getSubFloatStr(plotPath, label):
    """Get Subfloat"""
    return "".join(["\\subfloat[", label, "]"
                    "{\\includegraphics[width=0.5\\linewidth]{", plotPath ,"}}\n"])


def getLabels(plots):
    """
    Get the labels to the plots
    """
    # hardcoded at the moment
    ptBinning = [10, 12, 14, 16, 18, 20, 22, 25, 30, 35, 40, 50, 70]
    rapBinning = [0, 1.2]

    lbls = []
    # if rapX_ptY is in plot the labels are the bin values
    if "rap" in plots[0] and "pt" in plots[0].lower():
        for p in plots:
            [rap, pt] = getRapPt(p)
            lbls.append(getRapPtLbl(rap, rapBinning, pt, ptBinning))

        return lbls

    # if one of the lambda abbrevitations is in the plot name, that is the label
    for p in plots:
        if "lth" in p: lbls.append("$\\lambda_{\\theta}$")
        if "lph" in p: lbls.append("$\\lambda_{\\phi}$")
        if "ltp" in p: lbls.append("$\\lambda_{\\theta\\phi}$")

    # here every case should be covered (to be checked)
    return lbls


"""
Argparse
"""
parser = argparse.ArgumentParser(description='Script for generating a valid tex file for reports.')
parser.add_argument("plotDir", help="path where the plots are located")
parser.add_argument("baseTexFile", help="basic tex file that has only to be filled with plots",
                    nargs="?",
                    default=os.path.join(os.environ["PHYS_UTILS_DIR"], "Latex/PolUtils/reportCorrMapBase.tex"))
parser.add_argument("--outputfile", "-o", help="name of the created output tex file",
                    dest="outfile", action="store", default="reportCorrMap.tex")


args = parser.parse_args()

"""
Script
"""

allPlots = getAllFilesFromDir(args.plotDir, ".pdf")

baseTexFile = open(args.baseTexFile, 'r')

# read base file in and add stuff where appropriate
tex = []
for line in baseTexFile:
    tex.append(line)
    for addLine in processLine(line, allPlots):
        tex.append(addLine)

baseTexFile.close()
# write to a new tex file
with open(args.outfile, 'w') as f:
    for l in tex: f.write(l)
