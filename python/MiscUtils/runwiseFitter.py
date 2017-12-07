#!/usr/bin/env python

import ROOT as ROOT
import os

from utils.miscHelpers import createRandomString, condMkDirFile
from utils.dimuon_fitting import JpsiModel, PsiPrimeModel, UpsilonModel

charm_trigger_selection = {
    'HLT_Dimuon10_PsiPrime_Barrel_Seagulls': 1,
    'HLT_Dimuon20_Jpsi_Barrel_Seagulls': 2,
    'HLT_Dimuon18_PsiPrime': 4,
    'HLT_Dimuon25_Jpsi': 8,
    'HLT_Dimuon18_PsiPrime_noCorrL1': 16,
    'HLT_Dimuon25_Jpsi_noCorrL1': 32
}

onia_trigger_selection = {
    'HLT_Dimuon10_Upsilon_Barrel_Seagulls': 1,
    'HLT_Dimuon14_Phi_Barrel_Seagulls': 2,
    'HLT_Dimuon12_Upsilon_eta1p5': 4,
    'HLT_Dimuon24_Upsilon_noCorrL1': 8,
    'HLT_Dimuon24_Phi_noCorrL1': 16,
    'HLT_Mu20_TkMu0_Phi': 32,
    'HLT_Mu25_TkMu0_Phi': 64,
    'HLT_Mu25_TkMu0_Onia': 128,
    'HLT_Mu30_TkMu0_Onia': 256
}


def get_trig_bits(trigger):
    """Get the map relating triggers to bits"""
    if 'Jpsi' in trigger:
        return charm_trigger_selection
    if 'PsiPrime' in trigger:
        return charm_trigger_selection
    if 'Upsilon' in trigger:
        return onia_trigger_selection


def create_trigger_cond(trigger, trigger_bit_map):
    """Create the condition to be checked for the trigger"""
    from utils.jupyter_helpers import combineCuts
    trig_bit = trigger_bit_map[trigger]

    bit_req = lambda t,b : ' & '.join([t, str(b)])
    return combineCuts([bit_req('trigger', trig_bit),
                        bit_req('matched', trig_bit)])


def create_trig_skim_file(infiles, trigger):
    """Create a temporary file containing only the events with the specified trigger"""
    tin = ROOT.TChain('rootuple')
    for fin in infiles:
        tin.AddFile(fin)

    tin.SetBranchStatus('*', 0)

    tin.SetBranchStatus('oniaM', 1)
    tin.SetBranchStatus('run', 1)
    tin.SetBranchStatus('trigger', 1)
    tin.SetBranchStatus('matched', 1)

    trig_bit_map = get_trig_bits(trigger)
    trig_req = create_trigger_cond(trigger, trig_bit_map)

    outname = createRandomString(16) + '.root'
    fout = ROOT.TFile(outname, 'recreate')
    tout = tin.CopyTree(trig_req)

    n_skim = tout.GetEntries()
    tout.Write()
    fout.Close()

    return outname, n_skim


def create_workspace():
    """Create a workspace and release it's ownership to avoid trouble with python GC"""
    ws = ROOT.RooWorkspace('ws_massfit')
    ROOT.SetOwnership(ws, False)
    return ws


def create_model(ws, infile, trigger):
    """Create the workspace containing the necessary data from the inputfile"""
    print('========== Creating workspace')
    tree = infile.Get('rootuple')

    if 'Jpsi' in trigger:
        return JpsiModel(ws, tree)
    if 'PsiPrime' in trigger:
        return PsiPrimeModel(ws, tree)
    if 'Upsilon' in trigger:
        return UpsilonModel(ws, tree)


def get_runs_in_data(infile):
    """Get the runs that are present in data"""
    from root_pandas import read_root
    df = read_root(infile, 'rootuple')

    return df.run.unique()


def do_runwise_fit(runs_data, model, ws, plot_base=None):
    """Fit every run separately"""
    # fix some of the parameters on the full sample
    model.fit(ws, 'run > 0 && run < 1000000')
    model.plot(ws, 'run > 0 && run < 1000000', plot_base) # just as overview
    model.fix(ws)

    for run in runs_data:
        run_sel = '=='.join(['run', str(run)])
        model.fit(ws, run_sel)


def run_period(period_files, trigger, period, plot_base):
    """Collect the data from all the files belonging to this period and run over it"""
    print('========== Running period {}'.format(period))
    skimfile, n_skim = create_trig_skim_file(period_files, trigger)

    # it is possible that there are no triggered events, only run the fits when there are events
    if n_skim:
        inputfile = ROOT.TFile.Open(skimfile)

        outbase = '/'.join([plot_base, '_'.join([trigger, period])])
        condMkDirFile(outbase)

        ws = create_workspace()
        fit_model = create_model(ws, inputfile, trigger)

        data_runs = get_runs_in_data(skimfile)
        print('========== Got {} runs with {} in total in this period'.format(len(data_runs), n_skim))
        do_runwise_fit(data_runs, fit_model, ws, outbase)

        ws.writeToFile(outbase + '.root')

    os.remove(skimfile)


def get_period_files(raw_periods):
    """Extract the files to run on and the period from the raw_input"""
    periods = {}
    for raw_p in raw_periods:
        period, raw_fs = raw_p.split(':')
        files = raw_fs.split(',')

        periods[period] = files

    return periods


if __name__ == '__main__':
    import argparse

    ROOT.gROOT.SetBatch()

    parser = argparse.ArgumentParser(description='script for doing the fits to get the '
                                     'yields/lumi for all runs in the passed data')
    parser.add_argument('trigger', help='which trigger path to use')
    parser.add_argument('-p', '--period', action='append', help='periods to run over in format: period:file1,file2,file3')
    parser.add_argument('-o', '--outdir', default='yield_fits',
                        help='output directory into which the plot, etc. should go')

    args = parser.parse_args()

    p_files = get_period_files(args.period)
    for period in p_files:
        run_period(p_files[period], args.trigger, period, args.outdir)
