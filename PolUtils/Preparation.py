#!/usr/bin/env python

import argparse
import json
import subprocess
import os

from utils.miscHelpers import condMkDirFile, strArgList, mergeDicts

def getPtRapRange(ptRange, rapRange):
    """Return dict containing pt and rapidity range (in bin numbers)"""
    return {"ptMin": ptRange[0], "ptMax": ptRange[1], "rapMin": rapRange[0], "rapMax": rapRange[1]}


def getExeConfig(name, config):
    """
    Create the dictionary of cl arguments that are special to the executable with the passed name.
    This is the place where special care is (to be) taken for file names, so that the executable only gets full paths
    """
    outBase = config["output_basedir"]
    exeConfig = config["exe-config"][name]
    args = {}
    for con in exeConfig:
        (flag, arg), = con.items()
        if isinstance(arg, basestring): # string arguments are checked for the possibility of being a file
            # if we have a slash assume we have a file name, same goes for a ".", if we start with "/", assuming we already
            # have a full path and nothing needs to be done
            if ("/" in arg or "." in arg) and not arg.startswith("/"):
                arg = "".join([outBase, arg])

        args[flag] = arg

    return args


def runFromJsonConfig(name, config):
    """
    run binary with flags present from (global) json configuration.
    Creates the folders of all output files by checking all command line arguments that
    have 'output' in them. Builds the command line argument list, where 'local' args take
    precedence over 'global' ones. Calls the executable (via full path) with all arguments,
    so that the executable will pick the ones it needs and use them.
    """
    ptRapRange = getPtRapRange(config["ptBinRange"], config["rapBinRange"])
    globalArgs = config["flag-config"] # global flags/settings
    localArgs = getExeConfig(name, config) # local in the sense of executable specific

    # merging such that local args take precedence over global ones
    allArgs = mergeDicts(globalArgs, ptRapRange, localArgs)

    # check all 'output' flags/argument pairs if the appropriate folder structure exists already
    # This could get slow if the dictionary is large enough, but for now it should not be a bottleneck
    for key in allArgs.keys():
        if "output" in key:
            condMkDirFile(allArgs[key])

    executable = os.path.join(os.environ["PHYS_UTILS_DIR"], "PolUtils/bin/", name)
    subprocess.call([executable] + strArgList(allArgs))


"""
Setup arparse and read JSON
"""
parser = argparse.ArgumentParser(description="This script runs all the necessary Preparation steps.")
parser.add_argument("jsonFile", help="Path to the json file containing the run-configuration.")

args = parser.parse_args()

with open(args.jsonFile) as f:
    fullConfig = json.loads(f.read())


"""
Actual script
"""
if 'build' in fullConfig and fullConfig['build']:
    try:
        subprocess.check_call(["make",
                               "-C", os.path.join(os.environ["PHYS_UTILS_DIR"], "PolUtils"),
                               "-k", "-j4", "all"])
    except subprocess.CalledProcessError:
        print("Could not build the executables, check compile output to see what's wrong.")

# run all specified executables from the json configuration
for exe in fullConfig["run-config"]:
    (name, run), = exe.items()
    if run:
        runFromJsonConfig(name, fullConfig)
