#!/usr/bin/env python

import argparse
import json
import ROOT

"""
Setup the argument parser
"""
parser = argparse.ArgumentParser(description="This script can be used to apply a cut to a TTree in a ROOT file.")
parser.add_argument("filenameInput", help="Path to the input filename")
parser.add_argument("filenameOutput", help="Path to the output filename")
parser.add_argument("jsonFile", help="Path to the json file declaring all additional information")

args = parser.parse_args()

"""
Read JSON file
"""
with open(args.jsonFile, 'r') as f:
    json = json.loads(f.read())


"""
Get TTree, apply cuts and store tree under the same name in the output file
"""
inputTree = ROOT.TChain(json["tree"])
inputTree.AddFile(args.filenameInput)

outFile = ROOT.TFile.Open(args.filenameOutput, 'recreate')

outputTree = inputTree.CopyTree(json["cut"])

outputTree.Write()
outFile.Write()
outFile.Close()
