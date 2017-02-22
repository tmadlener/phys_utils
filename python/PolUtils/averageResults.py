#!/usr/bin/env python

import argparse
from utils.TGraph_utils import averageGraphs
from utils.recurse import recurseOnFile, ObjectCollector


def getListOfGraphs(fn):
    """Get the list of the names of all graphs in the passed file"""
    f = TFile.Open(fn)
    graphCollector = ObjectCollector("", "TGraphAsymmErrors")
    recurseOnFile(f, graphCollector)
    graphNames = [n for n in graphCollector.objects]
    f.Close()
    return graphNames


def getGraphs(graphNames, files):
    """Get all graphs from all files and return a dict of lists"""
    graphs = {}
    for n in graphNames:
        graphs[n] = []

    for f in files:
        for n in graphNames:
            graphs[n].append(f.Get(n))

    return graphs


"""
Argparse
"""
parser = argparse.ArgumentParser(description="script for averaging the same graph from different files")
parser.add_argument("inputFiles", help="list of input files to process",
                    nargs="+")
parser.add_argument("-o", "--outputfile", help="File to which the averaged graph should be stored",
                    dest="outputFile", action="store", default="averageGraphs.root")

args = parser.parse_args()

"""
Script
"""
from ROOT import TFile, TGraphAsymmErrors

# get the list of all graphs from the first file in the list and
# then use that list to get them from the other files as well (without recursing)
graphNames = getListOfGraphs(args.inputFiles[0])

# open all the files here to be able to close them again easily
files = [TFile.Open(n) for n in args.inputFiles]
graphs = getGraphs(graphNames, files)


outputFile = TFile(args.outputFile, 'recreate')
for n in graphNames:
    avgGraph = averageGraphs(graphs[n])
    avgGraph.SetName(n)
    avgGraph.Write()

outputFile.Write()
outputFile.Close()

for f in files:
    f.Close()
