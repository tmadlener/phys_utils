import ROOT as ROOT
import ROOT.RooFit as RooFit

from utils.miscHelpers import createRandomString, stringify

class FitModel:
    def __init__(self, ws, intree):
        raise NotImplementedError('Initialization has to be defined by derived class')

    def _define_model(self):
        raise NotImplementedError('need to define \'_define_model\'')

    def fix(self, ws):
        """Fixes the "default" parameters after initial fit"""
        raise NotImplementedError('need to define \'fix\'')

    def fit(self, ws, run_selection):
        print('========== Fitting run_selection: {}'.format(run_selection))

        fit_data = ws.data('data').reduce(run_selection)
        fit_model = ws.pdf('fit_model')
        fit_res = fit_model.fitTo(fit_data, RooFit.Minos(False), RooFit.NumCPU(6),
                                  RooFit.Offset(False), RooFit.Extended(True),
                                  RooFit.Save(True))
        # fit_res = fit_model.fitTo(fit_data, RooFit.Minos(True), RooFit.NumCPU(6),
        #                           RooFit.Offset(False), RooFit.Save(True),
        #                           RooFit.Extended(True))

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



    def get_par_value(self, ws, par, snap=''):
        """Get the result fit parameter from the workspace"""
        if snap:
            ws.loadSnapshot(snap)
        var = ws.var(par)

        return [var.getVal(), var.getError()]


class JpsiModel(FitModel):
    def __init__(self, ws, intree=None):
        # if we don't plan to import data, we don't have to do anything here
        print('========== Constructing JpsiModel')
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
        print('========== Constructing PsiPrimeModel')
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
        print('========== Constructing UpsilonModel')
        if intree is None:
            return

        print('========== Importing data')
        dimu_mass = ROOT.RooRealVar('oniaM', 'M^{#mu#mu} / GeV', 8.65, 11.3)
        run = ROOT.RooRealVar('run', 'run', 0, 4000000)
        data = ROOT.RooDataSet('data', 'mass fit data',
                               ROOT.RooArgSet(dimu_mass, run),
                               RooFit.Import(intree))
        getattr(ws, 'import')(data) # import is keyword in python!

        self._define_model(ws)


    def _define_model(self, ws):
        mean1S = ROOT.RooRealVar('mean1S', '#mu_{M}^{1S} / GeV', 9.5, 9.0, 9.8)
        sigma1S = ROOT.RooRealVar('sigma1S', '#sigma_{M}^{1S} / GeV', 0.0035, 0, 0.25)
        alpha1S = ROOT.RooRealVar('alpha1S', '#alpha_{M}^{1S}', 0.6, 0, 2.5)
        N1S = ROOT.RooRealVar('N1S', 'N_{M}^{1S}', 5)

        # mean2S = ROOT.RooRealVar('mean2S', '#mu_{M}^{2S} / GeV', 10.05, 9.8, 10.1)
        mean2S = ROOT.RooFormulaVar('mean2S', '#mu_{M}^{2S}',
                                    '@0 * {} / {}'.format(10023.26, 9460.30),
                                    ROOT.RooArgList(mean1S))
        sigma2S = ROOT.RooRealVar('sigma2S', '#sigma_{M}^{2S} / GeV', 0.0035, 0, 0.25)
        alpha2S = ROOT.RooRealVar('alpha2S', '#alpha_{M}^{2S}', 0.6, 0, 2.5)
        N2S = ROOT.RooRealVar('N2S', 'N_{M}^{2S}', 5)

        # mean3S = ROOT.RooRealVar('mean3S', '#mu_{M}^{3S} / GeV', 10.5, 10.25, 10.5)
        mean3S = ROOT.RooFormulaVar('mean3S', '#mu_{M}^{1S}',
                                    '@0 * {} / {}'.format(10355.2, 9460.30),
                                    ROOT.RooArgList(mean1S))
        sigma3S = ROOT.RooRealVar('sigma3S', '#sigma_{M}^{3S} / GeV', 0.0035, 0, 0.5)
        alpha3S = ROOT.RooRealVar('alpha3S', '#alpha_{M}^{3S}', 0.6, 0, 2.5)
        N3S = ROOT.RooRealVar('N3S', 'N_{M}^{3S}', 5)

        lamb = ROOT.RooRealVar('lambda', '#lambda_{bkg}', 0.0, -10, 10)
        n_1S = ROOT.RooRealVar('n_1S', 'N_{sig}^{1S}', 1e4, 0, 1e7)
        n_2S = ROOT.RooRealVar('n_2S', 'N_{sig}^{2S}', 1e4, 0, 1e7)
        n_3S = ROOT.RooRealVar('n_3S', 'N_{sig}^{3S}', 1e4, 0, 1e7)
        n_bkg = ROOT.RooRealVar('n_bkg', 'N_{bkg}', 1e3, 0, 1e6)

        # have to split the import, since overloading only works up to ~10 arguments
        getattr(ws, 'import')(ROOT.RooArgSet(mean1S, sigma1S, alpha1S, N1S,
                                             mean2S, sigma2S, alpha2S, N2S))
        getattr(ws, 'import')(ROOT.RooArgSet(mean3S, sigma3S, alpha3S, N3S,
                                             lamb, n_1S, n_2S, n_3S, n_bkg))

        ws.factory('RooCBShape::signal1S(oniaM, mean1S, sigma1S, alpha1S, N1S)')
        ws.factory('RooCBShape::signal2S(oniaM, mean2S, sigma2S, alpha2S, N2S)')
        ws.factory('RooCBShape::signal3S(oniaM, mean3S, sigma3S, alpha3S, N3S)')
        ws.factory('RooExponential::background(oniaM, lambda)')
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


    def fix(self, ws):
        self._fix_params(ws, ['N1S', 'N2S', 'N3S',
                              'alpha1S', 'alpha2S', 'alpha3S'])
