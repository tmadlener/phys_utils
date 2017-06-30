def getROOTdefaultPalette(name='kBird'):
    """
    Create a matplotlib palette that is the same as ROOTs default
    palette (kBird) with a given name (defaults to 'kBird')
    """
    from matplotlib.colors import LinearSegmentedColormap

    _s = [0.0000, 0.1250, 0.2500, 0.3750, 0.5000, 0.6250, 0.7500, 0.8750, 1.0000]
    _r = [0.2082, 0.0592, 0.0780, 0.0232, 0.1802, 0.5301, 0.8186, 0.9956, 0.9764]
    _g = [0.1664, 0.3599, 0.5041, 0.6419, 0.7178, 0.7492, 0.7328, 0.7862, 0.9832]
    _b = [0.5293, 0.8684, 0.8385, 0.7914, 0.6425, 0.4662, 0.3499, 0.1968, 0.0539]

    _cdict = {'red': tuple((_s[i], _r[i], _r[i]) for i in range(0, 9)),
              'blue': tuple((_s[i], _b[i], _b[i]) for i in range(0, 9)),
              'green': tuple((_s[i], _g[i], _g[i]) for i in range(0, 9))
             }

    return LinearSegmentedColormap(name, _cdict)


def pairplot_hist2d(dataframe, **kwargs):
    """
    Plot all variables pairwise against each other using histograms in the diagonal
    and 2d (colorized) histograms in the offidagonal
    """
    from seaborn import PairGrid
    from matplotlib.pyplot import hist, hist2d

    def pg_hist2d(x, y, **kwargs):
        kwargs.pop('color') # not needed in our case
        cmap = getROOTdefaultPalette()
        hist2d(x, y, cmap=cmap, **kwargs)

    bins_diag = kwargs.pop('bins_diag', 25)
    bins_offdiag = kwargs.pop('bins_offdiag', 50)
    weights = kwargs.pop('weights', None)

    g = PairGrid(dataframe, **kwargs)
    g.map_diag(hist, bins=bins_diag, weights=weights)
    g.map_offdiag(pg_hist2d, bins=bins_offdiag, weights=weights)

    return g
