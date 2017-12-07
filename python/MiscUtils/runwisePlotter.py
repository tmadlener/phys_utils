#!/usr/bin/env python

import ROOT as ROOT
import numpy as np

from utils.dimuon_fitting import JpsiModel, PsiPrimeModel, UpsilonModel
from utils.miscHelpers import stringify

def get_trigger(filename):
    """
    Get the trigger from the filename
    Format is: Trigger_Parts_RunPart.root
    """
    return '_'.join(filename.split('_')[:-1])


def get_period(filename):
    """
    Get the period from the filename
    Format is Trigger_Parts_RunPart.root
    """
    return filename.split('_')[-1].split('.')[0]


def get_model(trigger):
    """Get the model according to the trigger"""
    print('========== Getting model')
    if 'Jpsi' in trigger:
        return JpsiModel('foo', None)
    if 'PsiPrime' in trigger:
        return PsiPrimeModel('foo', None)
    if 'Upsilon' in trigger:
        return UpsilonModel('foo', None)



def create_run_lumi_map(lumifile):
    """Create the map of run to lumi"""
    import csv
    print('========== Creating run - lumi map')

    rl_map = {}
    with open(lumifile, 'r') as csvf:
        reader = csv.reader(csvf, delimiter=',')

        for row in reader:
            # only use filled lines that are not commented
            if not row or row[0].startswith('#'):
                continue
            # run is in format run:fill
            run = int(row[0].split(':')[0])
            # recorded lumi is in last column
            lumi = float(row[-1])

            # rl_map[str(run)] = lumi # store them in a json, need strings as keys
            rl_map[run] = lumi # store them in a json, need strings as keys

    return rl_map


def get_snapshot_names(ws):
    """
    Get the names of all the snapshots from the workspace, by using
    the names of the fit results which can be retrieved without knowing
    the names (cannot be done for snapshots I think)
    """
    snapnames = []
    for obj in ws.allGenericObjects():
        if 'results' in obj.GetName():
            snapnames.append(obj.GetName().replace('results', 'snap'))

    return snapnames


def get_run_from_snap(snapname):
    """
    Get the run number from the snapshot name
    """
    return int(snapname.split('_')[-1])


def make_period_graph(p_results, rl_map, variable, norm_lumi):
    """
    Make the graph for one period
    """
    print('========== Making graph for {}'.format(variable))
    n_runs = len(p_results)
    runs = np.empty(n_runs, dtype='d')
    vals = np.empty(n_runs, dtype='d')
    errs = np.empty(n_runs, dtype='d')

    for (i,run) in enumerate(p_results):
        var_val, var_error = p_results[run][variable]
        if norm_lumi:
            lumi = rl_map[run]
            var_val, var_error = var_val / lumi, var_error / lumi

        runs[i] = run
        vals[i] = var_val
        errs[i] = var_error

    return ROOT.TGraphErrors(n_runs, runs, vals, np.zeros(n_runs, dtype='d'), errs)


def process_file(inputfile, variables, rl_map, plot=False):
    """Process one of the input files"""
    print('========== Processing file {}'.format(inputfile))
    trigger = get_trigger(inputfile)
    model = get_model(trigger)

    fin = ROOT.TFile.Open(inputfile)
    ws = fin.Get('ws_massfit')
    snapnames = get_snapshot_names(ws)

    run_yields = {}
    for snap in snapnames:
        if 'AND' in snap:
            continue # don't process the full sample run

        run = get_run_from_snap(snap)
        # load it here to not have to load it for every variable
        ws.loadSnapshot(snap)

        # TODO: plots

        run_vals = {}
        for var in variables:
            run_vals[var] = model.get_par_value(ws, var, '')

        run_yields[run] = run_vals

    period = get_period(inputfile)

    graphs = {}
    for var in variables:
        # TODO: handling to have variables and norm_lumi
        graphs[var] = make_period_graph(run_yields, rl_map, var, True)

    return period, graphs


def collect_graphs(graphs, variable):
    """Collect all graphs corresponding to a variable"""
    v_graphs = []
    for period in graphs:
        v_graphs.append(graphs[period][variable])

    return v_graphs


if __name__ == '__main__':
    import argparse

    ROOT.gROOT.SetBatch()

    parser = argparse.ArgumentParser(description='script for processing the files produced '
                                     'runwiseFitter script.')
    parser.add_argument('lumifile', help='file (csv) containing lumi info as obtained from brilcalc')
    parser.add_argument('fitfiles', nargs='+', help='fitfiles to process')
    parser.add_argument('-v', '--variables', nargs='+', help='variables to plot')
    parser.add_argument('-p', '--plot', help='create plots', default=False,
                        action='store_true')

    args = parser.parse_args()

    rl_map = create_run_lumi_map(args.lumifile)

    import pprint
    pprint.pprint(rl_map)

    all_res = {}
    for f in args.fitfiles:
        period, graphs = process_file(f, args.variables, rl_map, args.plot)
        all_res[period] = graphs


    ## TODO: fully automate plotting, the rest seems to work pretty fine
    gg = collect_graphs(all_res, args.variables[0])

    mkplot(gg, saveAs='test.pdf')
