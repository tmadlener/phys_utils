#!/usr/bin/env python

import argparse

from utils.recurse import deepCollectHistograms
from utils.plotHelpers import mkplot
from utils.miscHelpers import condMkDirFile


"""
Argparse
"""
parser = argparse.ArgumentParser(description='Script that extracts the plots from a DQM'
                                 ' file and stores them in the same subdirectory structure'
                                 ' on disk as they are in the file. If given more than one'
                                 ' file it will create comparison plots.')
parser.add_argument('files', help='input files', nargs='+' )
parser.add_argument('-o', '--outdir', help='output base directory', default='./',
                    dest='outdir')
parser.add_argument('-v', '--verbose', help='verbose printings', action='store_true',
                    default=False, dest='verbose')
parser.add_argument('-d', '--startdir', help='The TDirectory to start at in the TFile if '
                    'only a subpart of the file is needed', default='', dest='startdir')
parser.add_argument('-r', '--regex', help='The regex expression the plot name has to match '
                    ' in order to be saved (including the full path to the plot)',
                    dest='rgx', default='')
parser.add_argument('-k', '--keys', help='comma separated list of keys to use for the '
                    'legends in the plots (One per input file).', default='', dest='keys')
parser.add_argument('-x', '--extension', help='File extension to be used for saving the '
                    ' plots (the \'.\' is needed!). Default = \'.svg\'', dest='extension',
                    default='.svg')

args = parser.parse_args()


"""
Script
"""
from ROOT import TFile, TDirectory, gROOT
gROOT.SetBatch()
if not args.verbose:
    gROOT.ProcessLine('gErrorIgnoreLevel = 1001')

# Use the first input file to collect all plots from there and use the dict keys from it
# to later get the same plots from the other files.  This assumes that the same plots are
# available in all passed input files!  If a plot is only available in the first file,
# this is no problem. However,  if a plot is not collected from the first file, but is
# available in any other file it will not be printed. For DQM files that have been
# produced with the same modules this should not be a real problem, since all the files
# have the same plots.

files = [TFile.Open(fn) for fn in args.files]

# prepare keys
if args.keys:
    keys = args.keys.split(',')
else:
    keys = []

if keys and len(keys) != len(files):
    print('WARNING: Got {} files and {} keys. Make sure the numbers match!'
          ' Continuing now but script might crash'. format(len(files), len(keys)))

# check where we start and set the directory for the recursion start accordingly
fDirPath = args.startdir
if fDirPath:
    fDir = files[0].Get(fDirPath)
else:
    fDir = files[0]

# get histograms from first file
hists = deepCollectHistograms(fDir, args.rgx)

# loop over all hists and check the other files if the plots are also present there. If
# yes, plot all of the plots onto one canvas
for n in hists:
    # collect the current histogram again (together with all the others)
    # not checking if all the plots are actually present currently, simply assuming
    # they are!
    allhists = [f.Get(n) for f in files]

    # create plot file name
    n = n.replace(fDirPath, '') # common part can be removed
    n = n.replace('//', '/') # cleanup some of the spurious slashes
    n += args.extension

    plname = '/'.join([args.outdir, n])
    condMkDirFile(plname)

    # colz has no effect on 1D hists for drawOpt
    mkplot(allhists, saveAs=plname, drawOpt='colz', legEntries=keys)
