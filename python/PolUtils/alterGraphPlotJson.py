#!/usr/bin/env python

import argparse
import json
import os

def sanitizeInputFileNames(inputfiles):
    """
    Sanitize the input file name list (of lists) so that every file has a legend entry (empty)
    Merges all but the first entry in each list into one string that is then used as legend entry
    """
    for inf in inputfiles:
        n = len(inf)
        if n > 2:
            inf[1] = " ".join(inf[1:])
            del inf[-(n-2):]
        if n < 2:
            inf.append("")

    return inputfiles


"""
Argparse
"""
parser = argparse.ArgumentParser(description="script for auto-generating json files parseable by plotGraphs.py")
parser.add_argument("outputJsonFile", help="path to the generated json file")
parser.add_argument("baseJsonFile", help="basic json file to be adapted",
                    nargs="?", default=os.path.join(os.environ["PHYS_UTILS_DIR"], "python/PolUtils/basicPlots.json"))
parser.add_argument("--inputfile", "-i", help="add input file to be used (filename + leg entry)",
                    nargs="+", dest="inputfiles", action="append")
# parser.add_argument("--graphs", "-g", help="graph (names + additional leg entry) to be plotted (if present in input files)",
#                     nargs="+", dest="graphs", action="append")
parser.add_argument("--outbase", "-o", help="base name of the produced pdfs",
                    dest="outbase", action="store")

parser.set_defaults(outbase="", inputfiles=[])

args = parser.parse_args()


"""
Script
"""
with open(args.baseJsonFile) as f:
    baseJson = json.loads(f.read())

infiles = sanitizeInputFileNames(args.inputfiles)
baseJson["inputfiles"] = infiles

baseJson["outbase"] = args.outbase

with open(args.outputJsonFile, "w") as f:
    json.dump(baseJson, f)
