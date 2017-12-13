import ROOT as r
import ROOT.Fit as rf
import ROOT.Math as rm

def _get_contour_TGraph(minimizer, error_level=1):
    """Construct a TGraph to store a contour"""
    from numpy import array, empty
    # only set to a new level if it has changed
    if minimizer.ErrorDef() != error_level:
        minimizer.SetErrorDef(error_level)

    n_points = 100
    np = array([n_points], dtype='I') # to have something that serves as unsigned int &
    cx = empty(n_points, dtype='d') # serves as double*
    cy = empty(n_points, dtype='d')

    # currently hardcoded: the parameter indices to get the contour
    if minimizer.Contour(1, 2, np, cx, cy):
        return r.TGraph(n_points, cx, cy)
    else:
        print('ERROR: Cannot get contour')
        return None


class CosthRatioFit():
    """Class handling the fitting of costh ratios."""
    def __init__(self, ratio_hist, ratio_func, **kwargs):
        """Setup everything"""
        self._setup_data(ratio_hist, kwargs.pop('use_center', False))
        self._setup_func(ratio_func)
        self._setup_fitter(kwargs.pop('fix_params', None))
        self._migrad = False
        self._minos = False
        self._hesse = False


    def fit(self):
        """Run the fit"""
        self._migrad = self.fitter.LeastSquareFit(self.data)
        self._print_results()
        self._recalc_uncer()


    def get_results(self, err_levels=[]):
        """Get fit results (possibly at different error levels)"""
        if not self._migrad:
            self.fit()
        if not self._minos or not self._hesse:
            print('Status of migrad/minos/hesse: {}/{}/{}.\n'
                  'Cannot return fit results'.format(self._migrad, self._minos, self._hesse))
            return None

        if not err_levels:
            return _get_one_result(1)
        else:
            results = []
            for err_level in err_levels:
                self._recalc_uncer(err_level)
                results.append(self._get_one_result(err_level))

            return results


    def _get_one_result(self, err_level):
        """Get the uncertainties, etc. belonging to one result"""
        res = {}
        fit_res = self.fitter.Result()
        res['chi2'] = fit_res.Chi2()
        res['ndf'] = fit_res.Ndf()
        res['errors'] = [v for v in fit_res.Errors()]
        res['params'] = [v for v in fit_res.Parameters()]
        # currently hardcoding the fact that we have three parameters here
        res['up_errors'] = [fit_res.UpperError(i) for i in range(3)]
        res['low_errors'] = [fit_res.LowerError(i) for i in range(3)]
        res['contour'] = _get_contour_TGraph(self.fitter.GetMinimizer(), err_level)
        res['err_level'] = err_level

        return res


    def _recalc_uncer(self, err_level=1):
        """Recalculate the uncertainties at a given error level"""
        self.fitter.GetMinimizer().SetErrorDef(err_level)
        self._hesse = self.fitter.CalculateHessErrors()
        self._print_results()
        self._minos = self.fitter.CalculateMinosErrors()
        self._print_results()


    def _print_results(self):
        """Print current results"""
        self.fitter.GetMinimizer().PrintResults()


    def _setup_data(self, hist_data, use_center):
        """Create the data in the format usable by the Fitter class"""
        opt = rf.DataOptions()
         # use the integral of the function in the bin, instead of central value if True
        opt.fIntegral = not use_center
        opt.fUseEmpty = False # ignore empty bins in data

        self.data = rf.BinData(opt)
        rf.FillData(self.data, hist_data)


    def _setup_func(self, func):
        """Setup the fitting function for the usage with the Fitter class"""
        self.func = rm.WrappedMultiTF1(func, func.GetNdim())


    def _setup_fitter(self, fix_params):
        """Setup the fitter"""
        self.fitter = rf.Fitter()
        self.fitter.SetFunction(self.func, False) # false to let the minimizer do gradient calcs

        minopt = rm.MinimizerOptions()
        minopt.SetMinimizerType('Minuit2')
        minopt.SetPrintLevel(1)
        self.fitter.Config().SetMinimizerOptions(minopt)

        if fix_params is not None:
            for (ipar, par_val) in fix_params:
                self.fitter.Config().ParSettings(ipar).SetValue(par_val)
                self.fitter.Config().ParSettings(ipar).Fix()
