#!/usr/bin/env python

import subprocess
import math
import json
import argparse
import numpy as np
import os
# import sys

from utils.miscHelpers import strArgList

def createRefMap(lth, lph, ltp, rapBin, ptBin, outfilename, ctbins=64, phibins=16):
    """
    Create a reference costhetaphi map in the passed outfilename
    """
    if any(math.isnan(x) for x in [lth, lph, ltp]):
        print("Got nan for one of the lambdas: ({}, {}, {})."
              "Will not produce a reference map for (rap, pt)-bin ({},{})".format(lth, lph, ltp, rapBin, ptBin))
        return None

    argDict = {'lth': lth, 'ltp': ltp, 'lph': lph, 'pt': ptBin, 'rap': rapBin, 'filename': outfilename,
               'binsCt': ctbins, 'binsPhi': phibins}

    print("Producing reference map for (rap, pt)-bin {}, {}".format(rapBin, ptBin))
    subprocess.call([os.path.join(os.environ["PHYS_UTILS_DIR"],"PolUtils/bin/runCreateRefMap")]
                    + strArgList(argDict))


def fitReferenceMap(rapBin, ptBin, histfile):
    """
    Fit the reference map of the rapidity and pt bin and return the fit result
    """
    f = TF2("fcosthphi",
            "[0]*(1.+[1]*x[0]*x[0]+[2]*(1.-x[0]*x[0])*cos(2.*x[1]*0.0174532925)+[3]*2.*x[0]*sqrt(1.-x[0]*x[0])*cos(x[1]*0.0174532925))",
            -1.0, 1.0, -180.0, 180.0)
    f.SetParameters(1.0, 0.0, 0.0, 0.0)

    histname = "".join(["cosThPhi_refMap_rap", str(rapBin), "_pt", str(ptBin)])
    h = histfile.Get(histname)
    h.Fit(f)

    return {"lth": [f.GetParameter(1), f.GetParError(1)],
            "lph": [f.GetParameter(2), f.GetParError(2)],
            "ltp": [f.GetParameter(3), f.GetParError(3)]}


class TestTree:
    """
    A tree to store injected and fitted lambda values from reference map creation
    """
    def __init__(self):
        """
        create TTree and branches
        """
        self.tree = TTree("refMapTest", "injected and fitted reference lambda values")

        # create branches (can this be done nicer?)
        self.lthInj = np.zeros(1, dtype = float)
        self.tree.Branch("lthInj", self.lthInj, "lthInj/D")
        self.lthInjErr = np.zeros(1, dtype = float)
        self.tree.Branch("lthInjErr", self.lthInjErr, "lthInjErr/D")

        self.lphInj = np.zeros(1, dtype = float)
        self.tree.Branch("lphInj", self.lphInj, "lphInj/D")
        self.lphInjErr = np.zeros(1, dtype = float)
        self.tree.Branch("lphInjErr", self.lphInjErr, "lphInjErr/D")

        self.ltpInj = np.zeros(1, dtype = float)
        self.tree.Branch("ltpInj", self.ltpInj, "ltpInj/D")
        self.ltpInjErr = np.zeros(1, dtype = float)
        self.tree.Branch("ltpInjErr", self.ltpInjErr, "ltpInjErr/D")

        ## fitted values
        self.lthFit = np.zeros(1, dtype = float)
        self.tree.Branch("lthFit", self.lthFit, "lthFit/D")
        self.lthFitErr = np.zeros(1, dtype = float)
        self.tree.Branch("lthFitErr", self.lthFitErr, "lthFitErr/D")

        self.lphFit = np.zeros(1, dtype = float)
        self.tree.Branch("lphFit", self.lphFit, "lphFit/D")
        self.lphFitErr = np.zeros(1, dtype = float)
        self.tree.Branch("lphFitErr", self.lphFitErr, "lphFitErr/D")

        self.ltpFit = np.zeros(1, dtype = float)
        self.tree.Branch("ltpFit", self.ltpFit, "ltpFit/D")
        self.ltpFitErr = np.zeros(1, dtype = float)
        self.tree.Branch("ltpFitErr", self.ltpFitErr, "ltpFitErr/D")

        ## also store rap and pt bin
        self.rapBin = np.zeros(1, dtype = int)
        self.tree.Branch("rapBin", self.rapBin, "rapBin/I")
        self.ptBin = np.zeros(1, dtype = int)
        self.tree.Branch("ptBin", self.ptBin, "ptBin/I")


    def fill(self, inj, fit):
        """
        Fill the tree with the passed values
        """
        self.lthInj[0] = inj["lth"][0]
        self.lthInjErr[0] = inj["lth"][1]

        self.lphInj[0] = inj["lph"][0]
        self.lphInjErr[0] = inj["lph"][1]

        self.ltpInj[0] = inj["ltp"][0]
        self.ltpInjErr[0] = inj["ltp"][1]

        ## fitted values
        self.lthFit[0] = fit["lth"][0]
        self.lthFitErr[0] = fit["lth"][1]

        self.lphFit[0] = fit["lph"][0]
        self.lphFitErr[0] = fit["lph"][1]

        self.ltpFit[0] = fit["ltp"][0]
        self.ltpFitErr[0] = fit["ltp"][1]

        ## pt and rap bin
        self.ptBin[0] = inj["pt"]
        self.rapBin[0] = inj["rap"]

        self.tree.Fill()

    def Write(self):
        self.tree.Write()


"""
Argpares and json
"""
parser = argparse.ArgumentParser("Script for generating reference maps from a json file")
parser.add_argument("jsonFile", help="Path to the json file containing the lambdas to be used for the reference maps")
parser.add_argument("outputFile", help="File in which the reference maps should be stored")
parser.add_argument("--createmaps", "-c", dest="createmaps", help="Create the reference maps",
                    action="store_true")
parser.add_argument("--no-createmaps", "-nc", dest="createmaps",
                    help="Prevent (re)creation of reference maps", action="store_false")
parser.add_argument("--fitmaps", "-f", dest="fitmaps", help="Fit the created reference maps",
                    action="store_true")
parser.add_argument("--no-fitmaps", "-nf", dest="fitmaps", help="Do not fit the created reference maps",
                    action="store_false")

parser.set_defaults(createmaps=True, fitmaps=True)
args = parser.parse_args()

with open(args.jsonFile) as f:
    json = json.loads(f.read())


"""
Actual work
"""
from ROOT import TFile, TTree, TF2, gROOT # import root stuff here to not interfere with argparse
gROOT.SetBatch()

## create reference maps
if args.createmaps:
    for lam in json["lambdas"]:
        createRefMap(lam["lth"][0], lam["lph"][0], lam["ltp"][0], lam["rap"], lam["pt"], args.outputFile, 64, 16)


## fit reference maps and store fitted values together with the "reference and everything else"
if args.fitmaps:
    gROOT.SetBatch()
    histfile = TFile.Open(args.outputFile, 'update')
    tree = TestTree()

    for lam in json["lambdas"]:
        tree.fill(lam, fitReferenceMap(lam["rap"], lam["pt"], histfile))

    tree.Write()
    histfile.Write()
    histfile.Close()
