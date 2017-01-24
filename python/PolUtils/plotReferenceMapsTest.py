#!/usr/bin/env python

import numpy as np
import matplotlib.pyplot as plt
import argparse

from utils.miscHelpers import sanitizeInputFileNames

# some global matplotlib settings
plt.rcParams['xtick.labelsize'] = 14
plt.rcParams['ytick.labelsize'] = 14
plt.rcParams['axes.labelsize'] = 16

# setup tex rendering
plt.rc("text",usetex=True)
plt.rc("font", family="serif")

beVerbose = False # turn progress output off by default

class FilesData:
    """helper class for holding all data from all files."""

    def __init__(self):
        """Create empty list for each parameter"""
        self.lthInj = []
        self.lphInj = []
        self.ltpInj = []

        self.lthInjErr = []
        self.lphInjErr = []
        self.ltpInjErr = []

        self.lthFit = []
        self.lphFit = []
        self.ltpFit = []

    def initTree(self, tree):
        """Setup branches in new list for TTree to read"""
        N = tree.GetEntries()
        self.lthInj.append(np.zeros(N, dtype=float))
        self.lphInj.append(np.zeros(N, dtype=float))
        self.ltpInj.append(np.zeros(N, dtype=float))

        self.lthInjErr.append(np.zeros(N, dtype=float))
        self.lphInjErr.append(np.zeros(N, dtype=float))
        self.ltpInjErr.append(np.zeros(N, dtype=float))

        self.lthFit.append(np.zeros(N, dtype=float))
        self.lphFit.append(np.zeros(N, dtype=float))
        self.ltpFit.append(np.zeros(N, dtype=float))


    def loopTree(self, tree):
        """Loop over all events in tree and read entries into np arrays"""
        i=0
        for entry in tree:
            self.lthInj[-1][i] = entry.lthInj
            self.lphInj[-1][i] = entry.lphInj
            self.ltpInj[-1][i] = entry.ltpInj

            self.lthInjErr[-1][i] = entry.lthInjErr
            self.lphInjErr[-1][i] = entry.lphInjErr
            self.ltpInjErr[-1][i] = entry.ltpInjErr

            self.lthFit[-1][i] = entry.lthFit
            self.lphFit[-1][i] = entry.lphFit
            self.ltpFit[-1][i] = entry.ltpFit

            i += 1


def collectData(infiles):
    """
    Collect the data from all passed files
    """
    allData = FilesData()

    for ifn in infiles:
        if beVerbose: print("Processing file \'{}\'".format(ifn[0]))

        f = TFile.Open(ifn[0])

        tree = f.Get("refMapTest") # hardcoded
        allData.initTree(tree)
        allData.loopTree(tree)

    return allData


def calcDifferences(a, b):
    """
    Calculate the differences between a and b, where a and b
    are both a list of numpy arrays with the same dimensions
    (i.e. length of list and length of numpy arrays)
    """
    return [f - i for (i, f) in zip(a, b)]


def calcRatios(a, b):
    """
    Calculate the ratio of a over b, where a and b are both a list of numpy arrays
    with the same dimensions (i.e. length of list and length of numpy arrays)
    """
    return [n / d for (n, d) in zip(a, b)]


def createHist(vals, infiles, filename, lsub):
    """
    Save histogram of the passed vals (numpy.array) under the passed filename
    """
    # we want symmetric plots
    if beVerbose: print("Creating plot \'{}\'".format(filename))

    maxval = np.max(np.abs(vals))

    colors = ['b', 'r', 'g', 'k', 'c', 'm', 'y'] # can't plot more than 7 in this way

    # for some reason matplotlib renders only singly quoted strings correctly
    xlabel = r''.join([r'\lambda_{', lsub, r'}^\text{fit} - \lambda_{', lsub, r'}^\text{inj}'])

    fig = plt.figure()
    ax = fig.add_subplot(111)
    for (v, f, c) in zip(vals, infiles, colors):
        ax.hist(v, 50, range=[-maxval*1.05, maxval*1.05], ec=c, lw=1, fc='None', label=f[1])
    ax.set_xlabel(xlabel)
    ax.grid(True)

    plt.legend(loc="upper right")
    fig.tight_layout()
    fig.savefig(filename)


def createScat(res, ref, infiles, filename, lsub, pulls=False):
    """
    Create and save a scatter plot of residuals vs reference values (or pulls vs. reference values)
    """
    if beVerbose: print("Creating plot \'{}\'".format(filename))

    xlabel = r''.join([r'\lambda_{', lsub, r'}^\text{inj}'])
    if pulls:
        ylabel = r''.join([r'(\lambda_{', lsub, r'}^\text{fit} - \lambda_{', lsub,
                           r'}^\text{inj}) / \sigma_{\lambda_{', lsub, r'}^\text{inj}}'])
    else:
        ylabel = r''.join([r'(\lambda_{', lsub, r'}^\text{fit} - \lambda_{', lsub,
                           r'}^\text{inj}) / \lambda_{', lsub, r'}^\text{inj}'])

    fig = plt.figure()
    ax = fig.add_subplot(111)

    colors = ['b', 'r', 'g', 'k', 'c', 'm', 'y'] # can't plot more than 7 in this way
    for (rs, rf, i, c) in zip(res, ref, infiles, colors):
        ax.scatter(rf, rs, color=c, label=i[1])

    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.grid(True)

    plt.legend(loc="upper right")
    fig.tight_layout()
    fig.savefig(filename)


"""
Argparse
"""
parser = argparse.ArgumentParser(description="Script for making quick overview test plots of the reference map creation",
                                 epilog="The \'createRefMaps.py\' script offers the possibility to fit "
                                 "the reference maps it creates. This script can be used to make some quick "
                                 "overview plots to see the differences between injected and fitted lambdas.")
parser.add_argument("--inputfile", "-i", nargs="+", dest="inputfiles", action="append",
                    help="add input file to be used (filename + leg entry (can be empty and can contain whitespace))")
parser.add_argument("--output", "-o", help="Path to which the produced pdfs should be stored.",
                    action="store", dest="outPath")
parser.add_argument("--verbose", "-v", help="Print some progress messages",
                    action="store_true", dest="verbose")

parser.set_defaults(outPath="", inputfiles=[], verbose=False)
args = parser.parse_args()

beVerbose = args.verbose

"""
Script
"""
# import ROOT as r # import ROOT after argparse setup
from ROOT import TFile, TTree, gROOT
gROOT.SetBatch()

infiles = sanitizeInputFileNames(args.inputfiles)

data = collectData(infiles)

# calculate residuals
lthRes = calcDifferences(data.lthInj, data.lthFit)
lphRes = calcDifferences(data.lphInj, data.lphFit)
ltpRes = calcDifferences(data.ltpInj, data.ltpFit)

createHist(lthRes, infiles, "".join([args.outPath, "lth_residuals.pdf"]), r"\theta")
createHist(lphRes, infiles, "".join([args.outPath, "lph_residuals.pdf"]), r"\phi")
createHist(ltpRes, infiles, "".join([args.outPath, "ltp_residuals.pdf"]), r"\theta\phi")

lthRelRes = calcRatios(lthRes, data.lthInj)
lphRelRes = calcRatios(lphRes, data.lphInj)
ltpRelRes = calcRatios(ltpRes, data.ltpInj)

createScat(lthRelRes, data.lthInj, infiles, "".join([args.outPath, "lth_scatter.pdf"]), r"\theta")
createScat(lphRelRes, data.lphInj, infiles, "".join([args.outPath, "lph_scatter.pdf"]), r"\phi")
createScat(ltpRelRes, data.ltpInj, infiles, "".join([args.outPath, "ltp_scatter.pdf"]), r"\theta\phi")

lthPulls = calcRatios(lthRes, data.lthInjErr)
lphPulls = calcRatios(lphRes, data.lphInjErr)
ltpPulls = calcRatios(ltpRes, data.ltpInjErr)

createScat(lthPulls, data.lthInj, infiles, "".join([args.outPath, "lth_pulls_scatter.pdf"]), r"\theta", True)
createScat(lphPulls, data.lphInj, infiles, "".join([args.outPath, "lph_pulls_scatter.pdf"]), r"\phi", True)
createScat(ltpPulls, data.ltpInj, infiles, "".join([args.outPath, "ltp_pulls_scatter.pdf"]), r"\theta\phi", True)
