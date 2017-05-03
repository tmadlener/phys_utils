#!/usr/bin/env python

import argparse

from utils.TGraph_utils import divide

parser = argparse.ArgumentParser(description="script for dividing TGraphAsymmErrors")
parser.add_argument('-d', '--denominator', dest='denominator',
                    help='name of the denominator graph')
parser.add_argument('-n', '--numerator', dest='numerator',
                    help='name of the numerator graph')
parser.add_argument('-o', '--outputfile', dest='outfile', default='ratio.root',
                    help="name of the output file")
parser.add_argument('numfile', help='root file containing the num graphs')
parser.add_argument('denomfile', help='root file containing the denom graph')

args = parser.parse_args()

from ROOT import TFile, TGraphAsymmErrors

nf = TFile.Open(args.numfile)
df = TFile.Open(args.denomfile)

ng = nf.Get(args.numerator)
dg = df.Get(args.denominator)

of = TFile(args.outfile, 'recreate')

r = divide(ng, dg, '_'.join(['r', ng.GetName(), dg.GetName()]))
r.Write()
of.Close()
