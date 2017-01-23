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


def collectDataFile(fn):
    """
    Collect the data from one file
    """
    f = TFile.Open(fn)
    tree = f.Get("refMapTest") # hardcoded

    N = tree.GetEntries()
    lthInj = np.zeros(N, dtype=float)
    lphInj = np.zeros(N, dtype=float)
    ltpInj = np.zeros(N, dtype=float)

    lthFit = np.zeros(N, dtype=float)
    lphFit = np.zeros(N, dtype=float)
    ltpFit = np.zeros(N, dtype=float)

    # read entries into arrays
    i=0
    for entry in tree:
        lthInj[i] = entry.lthInj
        lphInj[i] = entry.lphInj
        ltpInj[i] = entry.ltpInj

        lthFit[i] = entry.lthFit
        lphFit[i] = entry.lphFit
        ltpFit[i] = entry.ltpFit

        i += 1

    f.Close()

    return [lthInj, lphInj, ltpInj, lthFit, lphFit, ltpFit]


def collectData(infiles):
    """
    Collect the data from all passed files
    """
    lthInj = []
    lphInj = []
    ltpInj = []
    lthFit = []
    lphFit = []
    ltpFit = []

    for f in infiles:
        fileData = collectDataFile(f[0])
        lthInj.append(fileData[0])
        lphInj.append(fileData[1])
        ltpInj.append(fileData[2])

        lthFit.append(fileData[3])
        lphFit.append(fileData[4])
        ltpFit.append(fileData[5])

    return [lthInj, lphInj, ltpInj, lthFit, lphFit, ltpFit]


def calcResiduals(inj, fit):
    """
    Calculate the residuals betwenn injected and fitted values
    for a list of numpy arrays (as returned from collectData)
    """
    res = []
    for (i, f) in zip(inj, fit):
        res.append(f - i)

    return res


def createHist(vals, infiles, filename, lsub):
    """
    Save histogram of the passed vals (numpy.array) under the passed filename
    """
    # we want symmetric plots
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


def createScat(res, ref, infiles, filename, lsub):
    """
    Create and save a scatter plot of residuals vs reference values
    """
    xlabel = r''.join([r'\lambda_{', lsub, r'}^\text{inj}'])
    ylabel = r''.join([r'(\lambda_{', lsub, r'}^\text{fit} - \lambda_{', lsub, r'}^\text{inj}) / \lambda_{',
                       lsub, r'}^\text{inj}'])

    fig = plt.figure()
    ax = fig.add_subplot(111)

    colors = ['b', 'r', 'g', 'k', 'c', 'm', 'y'] # can't plot more than 7 in this way
    for (rs, rf, i, c) in zip(res, ref, infiles, colors):
        ax.scatter(rf, rs/rf, color=c, label=i[1])

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

parser.set_defaults(outPath="", inputfiles=[])
args = parser.parse_args()


"""
Script
"""
# import ROOT as r # import ROOT after argparse setup
from ROOT import TFile, TTree, gROOT
gROOT.SetBatch()

infiles = sanitizeInputFileNames(args.inputfiles)

[lthInj, lphInj, ltpInj, lthFit, lphFit, ltpFit] = collectData(infiles)

# calculate residuals
lthRes = calcResiduals(lthInj, lthFit)
lphRes = calcResiduals(lphInj, lphFit)
ltpRes = calcResiduals(ltpInj, ltpFit)

createHist(lthRes, infiles, "".join([args.outPath, "lth_residuals.pdf"]), r"\theta")
createHist(lphRes, infiles, "".join([args.outPath, "lph_residuals.pdf"]), r"\phi")
createHist(ltpRes, infiles, "".join([args.outPath, "ltp_residuals.pdf"]), r"\theta\phi")

createScat(lthRes, lthInj, infiles, "".join([args.outPath, "lth_scatter.pdf"]), r"\theta")
createScat(lphRes, lphInj, infiles, "".join([args.outPath, "lph_scatter.pdf"]), r"\phi")
createScat(ltpRes, ltpInj, infiles, "".join([args.outPath, "ltp_scatter.pdf"]), r"\theta\phi")
