#!/usr/bin/env python

import ROOT as ROOT
import numpy as np

from utils.dimuon_fitting import JpsiModel, PsiPrimeModel, UpsilonModel, PhiModel
from utils.miscHelpers import stringify, condMkDir
from utils.plotHelpers import mkplot

# expected periods
_all_periods = ['Run2017B', 'Run2017C', 'Run2017D', 'Run2017E', 'Run2017F']

# variables which should be plotted normalized against lumi
_norm_lumi_vars = ['n_signal', 'n_bkg', 'n_1S', 'n_2S', 'n_3S']

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
    if 'Phi' in trigger:
        return PhiModel('foo', None)


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

    # sort the values by run number
    run_idcs = runs.argsort()
    runs = runs[run_idcs]
    vals = vals[run_idcs]
    errs = errs[run_idcs]

    graph = ROOT.TGraphErrors(n_runs, runs, vals, np.zeros(n_runs, dtype='d'), errs)
    graph.SetMarkerStyle(20)
    return graph


def process_file(inputfile, variables, rl_map, plot=False, outdir=''):
    """Process one of the input files"""
    print('========== Processing file {}'.format(inputfile))
    trigger = get_trigger(inputfile)
    model = get_model(trigger)

    fin = ROOT.TFile.Open(inputfile)
    ws = fin.Get('ws_massfit')
    snapnames = get_snapshot_names(ws)

    period = get_period(inputfile)

    run_yields = {}
    for snap in snapnames:
        if 'AND' in snap:
            continue # don't process the full sample run

        run = get_run_from_snap(snap)
        # load it here to not have to load it for every variable
        ws.loadSnapshot(snap)

        if plot:
            run_cuts = stringify(snap.replace('snap_', ''),reverse=True)
            plot_name = '_'.join([trigger, period])
            model.plot(ws, run_cuts, '/'.join([outdir, plot_name]))

        run_vals = {}
        for var in variables:
            run_vals[var] = model.get_par_value(ws, var, '')

        run_yields[run] = run_vals

    graphs = {}
    for var in variables:
        graphs[var] = make_period_graph(run_yields, rl_map, var, var in _norm_lumi_vars)

    return period, graphs


def collect_graphs(graphs, variable):
    """Collect all graphs corresponding to a variable"""
    v_graphs = []
    for period in _all_periods:
        if period in graphs:
            v_graphs.append(graphs[period][variable])
        else:
            v_graphs.append(ROOT.TGraph()) # append empty graph to have same colors
            v_graphs[-1].SetMarkerStyle(20)

    return v_graphs


if __name__ == '__main__':
    import argparse

    ROOT.gROOT.SetBatch()

    parser = argparse.ArgumentParser(description='script for processing the files produced '
                                     'runwiseFitter script.')
    parser.add_argument('lumifile', help='file (csv) containing lumi info as obtained from brilcalc')
    parser.add_argument('fitfiles', nargs='+', help='fitfiles to process')
    parser.add_argument('-v', '--variables', nargs='+', help='variables to plot')
    parser.add_argument('-p', '--plot', help='create plots for each run', default=False,
                        action='store_true')
    parser.add_argument('-o', '--outdir', help='output directory for run fit plots',
                        default='./')
    parser.add_argument('-y', '--ylabel', help='ylabel',
                        default='yields / ub^{-1}')
    parser.add_argument('-s', '--savetofile', default=None, type=str,
                        help='outputfile name to which the generated graphs should be stored')
    parser.add_argument('-np', '--noplots', default=False, action='store_true',
                        help='Do not create plots, but only write graphs to file')

    args = parser.parse_args()

    if args.plot:
        condMkDir(args.outdir)

    rl_map = create_run_lumi_map(args.lumifile)

    all_res = {}
    for f in args.fitfiles:
        period, graphs = process_file(f, args.variables, rl_map, args.plot, args.outdir)
        all_res[period] = graphs

    if args.savetofile is not None:
        gsave_file = ROOT.TFile(args.savetofile, 'update')

        for period in all_res:
            for var in all_res[period]:
                all_res[period][var].SetName('_'.join([period, var]))
                all_res[period][var].Write()

        # gsave_file.Write()
        gsave_file.Close()

    if not args.noplots:
        for var in args.variables:
            var_graphs = collect_graphs(all_res, var)

            # simply assume all files are from the same trigger
            plotname = '_'.join([get_trigger(args.fitfiles[0]), var])

            mkplot(var_graphs, saveAs=plotname + '.pdf',
                   grid=True, xRange=[296500, 307000], drawOpt='PE',
                   yRange=[0,None], yLabel=args.ylabel, xLabel='run',
                   legPos='botleft', legEntries=_all_periods)
