#!/usr/bin/env python

import numpy as np
import matplotlib.pyplot as plt
import argparse

# some global matplotlib settings
plt.rcParams['xtick.labelsize'] = 14
plt.rcParams['ytick.labelsize'] = 14
plt.rcParams['axes.labelsize'] = 16

# setup tex rendering
plt.rc("text",usetex=True)
plt.rc("font", family="serif")


def createHist(vals, filename, lsub):
    """
    Save histogram of the passed vals (numpy.array) under the passed filename
    """
    # we want symmetric plots
    maxval = np.max(np.abs(vals))

    # for some reason matplotlib renders only singly quoted strings correctly
    xlabel = r''.join([r'\lambda_{', lsub, r'}^\text{fit} - \lambda_{', lsub, r'}^\text{inj}'])
    print(xlabel)
    fig = plt.figure()
    ax = fig.add_subplot(111)
    ax.hist(vals, 50, range=[-maxval*1.05, maxval*1.05])
    ax.set_xlabel(xlabel)
    ax.grid(True)
    fig.savefig(filename)

def createScat(res, ref, filename, lsub):
    """
    Create and save a scatter plot of residuals vs reference values
    """
    xlabel = r''.join([r'\lambda_{', lsub, r'}^\text{inj}'])
    ylabel = r''.join([r'\lambda_{', lsub, r'}^\text{fit} - \lambda_{', lsub, r'}^\text{inj}'])

    print(xlabel)
    print(ylabel)

    fig = plt.figure()
    ax = fig.add_subplot(111)
    ax.scatter(ref, res)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.grid(True)

    fig.savefig(filename)


"""
Argparse
"""
parser = argparse.ArgumentParser(description="Script for making quick overview test plots of the reference map creation",
                                 epilog="The \'createRefMaps.py\' script offers the possibility to fit "
                                 "the reference maps it creates. This script can be used to make some quick "
                                 "overview plots to see the differences between injected and fitted lambdas.")
parser.add_argument("refMapFile", help="Input file, as created by \'createRefMaps.py\' with the --fitmaps flag set.")
parser.add_argument("--output", "-o", help="Path to which the produced pdfs should be stored.",
                    action="store", dest="outPath")

parser.set_defaults(outPath="")
args = parser.parse_args()


"""
Script
"""
import ROOT as r # import ROOT after argparse setup
r.gROOT.SetBatch()

fn = args.refMapFile
tn = "refMapTest"

infile = r.TFile.Open(fn)
tree = infile.Get(tn)

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

infile.Close()

# calculate residuals
lthRes = lthFit - lthInj
lphRes = lphFit - lphInj
ltpRes = ltpFit - ltpInj

createHist(lthRes, "".join([args.outPath, "lth_residuals.pdf"]), r"\theta")
createHist(lphRes, "".join([args.outPath, "lph_residuals.pdf"]), r"\phi")
createHist(ltpRes, "".join([args.outPath, "ltp_residuals.pdf"]), r"\theta\phi")

createScat(lthRes, lthInj, "".join([args.outPath, "lth_scatter.pdf"]), r"\theta")
createScat(lphRes, lphInj, "".join([args.outPath, "lph_scatter.pdf"]), r"\phi")
createScat(ltpRes, ltpInj, "".join([args.outPath, "ltp_scatter.pdf"]), r"\theta\phi")
