#!/usr/bin/env python

import ROOT as ROOT
import ROOT.RooFit as RooFit
import os

from utils.miscHelpers import createRandomString, condMkDirFile

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
        return charm_trigger_selection,
    if 'PsiPrime' in trigger:
        return charm_trigger_selection,
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

    return outname, n_skim > 0


def stringify(run_selection):
    """Get a string representation of the run_selection"""
    str_rep = run_selection.replace(' ', '') # remove whitespace
    str_rep = str_rep.replace('>=', '_ge_').replace('<=', '_le_')
    str_rep = str_rep.replace('>', '_gt_').replace('<','_lt_')
    str_rep = str_rep.replace('==', '_eq_').replace('&&','__')

    return str_rep


class FitModel:
    def __init__(self, ws, intree):
        raise NotImplementedError('Initialization has to be defined by derived class')

    def _define_model(self):
        raise NotImplementedError('need to define \'_define_model\'')

    def fix(self, ws):
        """Fixes the "default" parameters after initial fit"""
        raise NotImplementedError('need to define \'fit\'')

    def fit(self, ws, run_selection):
        print('========== Fitting run_selection: {}'.format(run_selection))

        fit_data = ws.data('data').reduce(run_selection)
        fit_model = ws.pdf('fit_model')
        fit_model.fitTo(fit_data, RooFit.Minos(False), RooFit.NumCPU(6),
                        RooFit.Offset(False), RooFit.Extended(True))
        fit_res = fit_model.fitTo(fit_data, RooFit.Minos(True), RooFit.NumCPU(6),
                                  RooFit.Offset(False), RooFit.Save(True),
                                  RooFit.Extended(True))

        fit_res.SetName('results_' + stringify(run_selection))
        getattr(ws, 'import')(fit_res)

        ws.saveSnapshot('snap_' + stringify(run_selection), ws.allVars())

        self.cov_qual = fit_res.covQual()


    def _fix_params(self, ws, params):
        print('========== Fixing parameters')
        for param in params:
            print('========== Fixing param {} to {}'.format(param, ws.var(param).getVal()))
            ws.var(param).setConstant(True)


    def plot(self, ws, run_selection, outbase):
        print('========== Plotting {}'.format(run_selection))
        fit_data = ws.data('data').reduce(run_selection)
        mass = ws.var('oniaM')
        frame = mass.frame()
        fit_data.plotOn(frame)

        model = ws.pdf('fit_model')
        model.plotOn(frame)
        model.plotOn(frame, RooFit.Components('background'), RooFit.LineStyle(2),
                     RooFit.LineColor(ROOT.kRed + 1))
        model.plotOn(frame, RooFit.Components('signal'), RooFit.LineStyle(1),
                     RooFit.LineColor(ROOT.kGreen + 10))

        c = ROOT.TCanvas(createRandomString(8), 'c', 600, 600)
        c.cd()
        frame.Draw()
        c.Draw()

        c.SaveAs('_'.join([outbase, stringify(run_selection)]) + '.pdf')
        c.SaveAs('_'.join([outbase, stringify(run_selection)]) + '.svg')



    def get_par_value(self, ws, par, run_sel):
        """Get the result fit parameter from the workspace"""
        ws.loadSnapshot('snap_' + stringify(run_sel))
        var = ws.var(par)

        return [var.getVal(), var.getError()]


class JpsiModel(FitModel):
    def __init__(self, ws, intree=None):
        # if we don't plan to import data, we don't have to do anything here
        if intree is None:
            return

        print('========== Importing data')
        dimu_mass = ROOT.RooRealVar('oniaM', 'M^{#mu#mu} / GeV', 2.95, 3.3)
        run = ROOT.RooRealVar('run', 'run', 0, 4000000)
        data = ROOT.RooDataSet('data', 'mass fit data',
                               ROOT.RooArgSet(dimu_mass, run),
                               RooFit.Import(intree))
        getattr(ws, 'import')(data) # import is keyword in python!

        self._define_model(ws)


    def _define_model(self, ws):
        print('========== Defining fit model')

        mean = ROOT.RooRealVar('meanM', '#mu_{M} / GeV', 3.1, 2.95, 3.29)
        sigma = ROOT.RooRealVar('sigmaM', '#sigma_{M} / GeV', 0.0035, 0, 0.5)
        alpha = ROOT.RooRealVar('alphaM', '#alpha_{M}', 0.6, 0.2, 2.5)
        N = ROOT.RooRealVar('NM', 'N_{M}', 2.5, 1.8, 6)

        lamb = ROOT.RooRealVar('lambda', '#lambda_{bkg}', 0.0, -10, 10)
        n_signal = ROOT.RooRealVar('n_signal', 'N_{sig}', 1e4, 0, 1e7)
        n_bkg = ROOT.RooRealVar('n_bkg', 'N_{bkg}', 1e3, 0, 1e6)

        getattr(ws, 'import')(ROOT.RooArgSet(mean, sigma, alpha, N, lamb,
                                             n_signal, n_bkg))

        ws.factory('RooCBShape::signal(oniaM, meanM, sigmaM, alphaM, NM)')
        ws.factory('RooExponential::background(oniaM, lambda)')
        ws.factory('SUM::fit_model(n_signal * signal, n_bkg * background)')

    def fix(self, ws):
        self._fix_params(ws, ['NM', 'alphaM'])


class PsiPrimeModel(FitModel):
    def __init__(self, ws, intree=None):
        # if we don't plan to import data, we don't have to do anything here
        if intree is None:
            return

        print('========== Importing data')
        dimu_mass = ROOT.RooRealVar('oniaM', 'M^{#mu#mu} / GeV', 3.4, 4.0)
        run = ROOT.RooRealVar('run', 'run', 0, 4000000)
        data = ROOT.RooDataSet('data', 'mass fit data',
                               ROOT.RooArgSet(dimu_mass, run),
                               RooFit.Import(intree))
        getattr(ws, 'import')(data) # import is keyword in python!

        self._define_model(ws)

    def _define_model(self, ws):
        mean = ROOT.RooRealVar('meanM', '#mu_{M} / GeV', 3.7, 3.4, 4.0)
        sigma = ROOT.RooRealVar('sigmaM', '#sigma_{M} / GeV', 0.0035, 0, 0.5)
        alpha = ROOT.RooRealVar('alphaM', '#alpha_{M}', 0.6, 0.2, 2.5)
        N = ROOT.RooRealVar('NM', 'N_{M}', 2.5, 0, 6)

        lamb = ROOT.RooRealVar('lambda', '#lambda_{bkg}', 0.0, -10, 10)
        n_signal = ROOT.RooRealVar('n_signal', 'N_{sig}', 1e4, 0, 1e7)
        n_bkg = ROOT.RooRealVar('n_bkg', 'N_{bkg}', 1e3, 0, 1e6)

        getattr(ws, 'import')(ROOT.RooArgSet(mean, sigma, alpha, N, lamb,
                                             n_signal, n_bkg))

        ws.factory('RooCBShape::signal(oniaM, meanM, sigmaM, alphaM, NM)')
        ws.factory('RooExponential::background(oniaM, lambda)')
        ws.factory('SUM::fit_model(n_signal * signal, n_bkg * background)')

    def fix(self, ws):
        self._fix_params(ws, ['NM', 'alphaM'])


class UpsilonModel(FitModel):
    def __init__(self, ws, intree=None):
        # if we don't plan to import data, we don't have to do anything here
        if intree is None:
            return

        print('========== Importing data')
        dimu_mass = ROOT.RooRealVar('oniaM', 'M^{#mu#mu} / GeV', 8.5, 11.5)
        run = ROOT.RooRealVar('run', 'run', 0, 4000000)
        data = ROOT.RooDataSet('data', 'mass fit data',
                               ROOT.RooArgSet(dimu_mass, run),
                               RooFit.Import(intree))
        getattr(ws, 'import')(data) # import is keyword in python!

        self._define_model(ws)


    def _define_model(self, ws):
        mean1S = ROOT.RooRealVar('mean1S', '#mu_{M}^{1S} / GeV', 9.5, 9.0, 9.8)
        sigma1S = ROOT.RooRealVar('sigma1S', '#sigma_{M}^{1S} / GeV', 0.0035, 0, 0.5)
        alpha1S = ROOT.RooRealVar('alpha1S', '#alpha_{M}^{1S}', 0.6, 0, 2.5)
        N1S = ROOT.RooRealVar('N1S', 'N_{M}^{1S}', 2.5, 0, 6)

        mean2S = ROOT.RooRealVar('mean2S', '#mu_{M}^{2S} / GeV', 10.0, 9.8, 10.2)
        sigma2S = ROOT.RooRealVar('sigma2S', '#sigma_{M}^{2S} / GeV', 0.0035, 0, 0.5)
        alpha2S = ROOT.RooRealVar('alpha2S', '#alpha_{M}^{2S}', 0.6, 0, 2.5)
        N2S = ROOT.RooRealVar('N2S', 'N_{M}^{2S}', 2.5, 0, 6)

        mean3S = ROOT.RooRealVar('mean3S', '#mu_{M}^{3S} / GeV', 10.3, 10.2, 10.5)
        sigma3S = ROOT.RooRealVar('sigma3S', '#sigma_{M}^{3S} / GeV', 0.0035, 0, 0.5)
        alpha3S = ROOT.RooRealVar('alpha3S', '#alpha_{M}^{3S}', 0.6, 0, 2.5)
        N3S = ROOT.RooRealVar('N3S', 'N_{M}^{3S}', 2.5, 0, 6)

        lamb = ROOT.RooRealVar('lambda', '#lambda_{bkg}', 0.0, -10, 10)
        n_1S = ROOT.RooRealVar('n_1S', 'N_{sig}^{1S}', 1e4, 0, 1e7)
        n_2S = ROOT.RooRealVar('n_2S', 'N_{sig}^{2S}', 1e4, 0, 1e7)
        n_3S = ROOT.RooRealVar('n_3S', 'N_{sig}^{3S}', 1e4, 0, 1e7)
        n_bkg = ROOT.RooRealVar('n_bkg', 'N_{bkg}', 1e3, 0, 1e6)

        # n_signal = ROOT.RooFormulaVar('n_signal', '@0 + @1 + @2',
        #                               ROOT.RooArgSet(n_1S, n_2S, n_3S))

        getattr(ws, 'import')(ROOT.RooArgSet(mean1S, sigma1S, alpha1S, N1S,
                                             mean2S, sigma2S, alpha2S, N2S))
        getattr(ws, 'import')(ROOT.RooArgSet(mean3S, sigma3S, alpha3S, N3S,
                                             lamb, n_1S, n_2S, n_3S, n_bkg))
                                             # n_signal))

        ws.factory('RooCBShape::signal1S(oniaM, mean1S, sigma1S, alpha1S, N1S)')
        ws.factory('RooCBShape::signal2S(oniaM, mean2S, sigma2S, alpha2S, N2S)')
        ws.factory('RooCBShape::signal3S(oniaM, mean3S, sigma3S, alpha3S, N3S)')
        ws.factory('RooExponential::background(oniaM, lambda)')
        # ws.factory('SUM::signal(n_1S * signal1S, n_2S * signal2S, n_3S * signal3S)')
        ws.factory('SUM::fit_model(n_1S * signal1S, n_2S * signal2S, n_3S * signal3S, n_bkg * background)')


        def plot(self, ws, run_selection, outbase):
            print('========== Plotting {}'.format(run_selection))
            fit_data = ws.data('data').reduce(run_selection)
            mass = ws.var('oniaM')
            frame = mass.frame()
            fit_data.plotOn(frame)

            model = ws.pdf('fit_model')
            model.plotOn(frame)
            model.plotOn(frame, RooFit.Components('background'), RooFit.LineStyle(2),
                         RooFit.LineColor(ROOT.kRed + 1))
            model.plotOn(frame, RooFit.Components('signal1S'), RooFit.LineStyle(1),
                         RooFit.LineColor(ROOT.kGreen + 10))
            model.plotOn(frame, RooFit.Components('signal2S'), RooFit.LineStyle(1),
                         RooFit.LineColor(ROOT.kGreen + 10))
            model.plotOn(frame, RooFit.Components('signal3S'), RooFit.LineStyle(1),
                         RooFit.LineColor(ROOT.kGreen + 10))

            c = ROOT.TCanvas(createRandomString(8), 'c', 600, 600)
            c.cd()
            frame.Draw()
            c.Draw()

            c.SaveAs('_'.join([outbase, stringify(run_selection)]) + '.pdf')
            c.SaveAs('_'.join([outbase, stringify(run_selection)]) + '.svg')


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


def create_lumi_run_map(lumifile):
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

            rl_map[str(run)] = lumi # store them in a json, need strings as keys

    return rl_map



def get_runs_in_data(infile):
    """Get the runs that are present in data"""
    from root_pandas import read_root
    df = read_root(infile, 'rootuple')

    return df.run.unique()


def do_runwise_fit(runs_data, model, ws, plot_base=None):
    """Fit every run separately"""
    # fix some of the parameters on the full sample
    model.fit(ws, 'run > 0 && run < 1000000')
    model.plot(ws, 'run > 0 && run < 1000000', plot_base if plot_base is not None else 'full_sample')
    model.fix(ws)

    n_signal = {}

    for run in runs_data:
        run_sel = '=='.join(['run', str(run)])
        model.fit(ws, run_sel)
        if plot_base is not None:
            model.plot(ws, run_sel, plot_base)

        # need strings as keys to store them in a json
        n_signal[str(run)] = [ws.var('n_signal').getVal(), ws.var('n_signal').getError(),
                         model.cov_qual]

    return n_signal


def run_period(period_files, trigger, period, plot_base):
    """Collect the data from all the files belonging to this period and run over it"""
    print('========== Running period {}'.format(period))
    skimfile, valid = create_trig_skim_file(period_files, trigger)
    n_signal = {}
    # it is possible that there are no triggered events, only run the fits when there are events
    if valid:
        inputfile = ROOT.TFile.Open(skimfile)

        outbase = '/'.join([plot_base, '_'.join([trigger, period])])
        condMkDirFile(outbase)

        ws = create_workspace()
        fit_model = create_model(ws, inputfile, trigger)

        data_runs = get_runs_in_data(skimfile)
        print('========== Got {} runs in this period'.format(len(data_runs)))
        n_signal = do_runwise_fit(data_runs, fit_model, ws,
                                  outbase)

        ws.writeToFile(outbase + '.root')

    os.remove(skimfile)

    return n_signal


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

    parser = argparse.ArgumentParser(description='script for getting the yields/lumi for all runs in the passed data')
    parser.add_argument('lumifile', help='file (csv) containing lumi info as obtained from brilcalc')
    parser.add_argument('trigger', help='which trigger path to use')
    parser.add_argument('-p', '--period', action='append', help='periods to run over in format: period:file1,file2,file3')
    parser.add_argument('-o', '--outdir', default='yield_fits',
                        help='output directory into which the plot, etc. should go')


    args = parser.parse_args()

    p_files = get_period_files(args.period)


    p_sig_yields = {}
    for period in p_files:
        p_yields = run_period(p_files[period], args.trigger, period, args.outdir)
        p_sig_yields[period] = p_yields

    import json
    with open(args.outdir + '/signal_yields.json', 'w') as f:
        json.dump(p_sig_yields, f, indent=2, sort_keys=True)

    rl_map = create_lumi_run_map(args.lumifile)
    with open(args.outdir + '/run_lumi_map.json', 'w') as f:
        json.dump(rl_map, f, indent=2, sort_keys=True)
