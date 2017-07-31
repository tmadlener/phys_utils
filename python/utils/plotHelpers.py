"""
Global variables (for now, maybe I can hide them in some global instance at some point)
"""
_colors = [] # store index and TColor in list
_colorIndices = [] # just the indices of the above list (this one is needed for plotting)
 # in order to have a possibility to create legends on the fly and have them appear on plots
 # they have to persist longer than the function scope, so store them here
_drawLegends = []
# store all canvases that are not returned by mkplot here, to have a handle to get back to them
_drawCanvas = []

def _defaultColors():
    from ROOT import TColor

    _rgbcolors = [ # some rgb colors (taken from MATLABs default palette)
        [0, 0.4470, 0.7410],
        [0.8500, 0.3250, 0.0980],
        [0.9290, 0.6940, 0.1250],
        [0.4940, 0.1840, 0.5560],
        [0.4660, 0.6740, 0.1880],
        [0.3010, 0.7450, 0.9330],
        [0.6350, 0.0780, 0.1840]
    ]

    global _colors
    global _colorIndices

    if not _colors: # only define the global colors if they are not already defined
        _getNewIdx = TColor.GetFreeColorIndex # bind function call to local variable for less typing
        _colors = [(_getNewIdx(), TColor(_getNewIdx(), _rgbcolors[i][0], _rgbcolors[i][1], _rgbcolors[i][2]))
                   for i in range(len(_rgbcolors))]
        _colorIndices = [i[0] for i in _colors]

    return _colorIndices


def setColor(p, col):
    """Set color col to all attributes of the passed plottable p"""
    p.SetLineColor(col)
    p.SetMarkerColor(col)


def plotOnCanvas(can, plots, **kwargs):
    """
    Generic plotting function, that puts all the plots on plots onto the same canvas.
    Returns the canvas

    Options:
    - colors: list of colors to be used for plotting (otherwise default colors will be used)
    - drawOpt: string option that will be passed to the Draw() method
    - leg: put a legend on to the plot. This has to be an already present TLegend object!
    - legEntries: list of string (at least as long as the plot list) from which the
    keys for the legend are taken

    - Still evolving
    """
    can.cd()

    colors = kwargs.pop('colors', _defaultColors())
    getCol = lambda i: colors[ i % len(colors) ]

    drawOption = ''.join(['same', kwargs.pop('drawOpt', '')])
    legend = kwargs.pop('leg', None)
    legEntries = kwargs.pop('legEntries', [h.GetName() for h in plots])

    for i in range(len(plots)):
        setColor(plots[i], getCol(i))
        plots[i].Draw(drawOption)

        if legend is not None:
            legend.AddEntry(plots[i], legEntries[i], 'ple')

    if legend is not None:
        legend.Draw()

    can.Update()

    return can


def _getPositionOnCanvas(pos):
    """
    Get the position from parsing the string
    """
    from utils.miscHelpers import getPartialMatcher
    import re

    defaultPosition = [0.6, 0.1, 0.9, 0.3]
    if not pos: # no need to go through the whole machinery for default position
        return defaultPosition

    # possible positions (i.e. currently implemented)
    posStrings = ['bottom', 'top', 'left', 'right', 'center']
    # relative positions in canvas
    vertPositions = {'bottom': [0.1, 0.3], 'top': [0.7, 0.9]}
    horPositions = {'left': [0.1, 0.4], 'right': [0.6, 0.9], 'center': [0.35, 0.65]}

    getPos = lambda x,d : d[x] if x in d else None

    position = [None]*4

    for p in posStrings:
        posMatcher = getPartialMatcher(p, 3) # need at least three chars to be unambiguous
        if re.search(posMatcher, pos):
            vertPos = getPos(p, vertPositions)
            if vertPos:
                position[1] = vertPos[0]
                position[3] = vertPos[1]

            horPos = getPos(p, horPositions)
            if horPos:
                position[0] = horPos[0]
                position[2] = horPos[1]

    # very basic error handling
    try:
        if None in position:
            raise ValueError
    except ValueError:
        print('Could not get position from passed position str \'{0}\''.format(pos))
        position = defaultPosition

    return position


def _setupLegend(position):
    """
    Set up a TLegend.
    Currently just returns a legend with fixed position, will be more versatile in the future
    """
    from ROOT import TLegend
    global _drawLegends
    posVals = _getPositionOnCanvas(position)
    legend = TLegend(posVals[0], posVals[1], posVals[2], posVals[3])
    _drawLegends.append(legend) # store it in the global list

    return legend


def widenRange(minVal, maxVal, d=0.1):
    """
    Get a slightly wider range for plotting, so that the plot fits nicely and no data
    points appear on the very edges of the plot
    """
    dMin = minVal * 0.1
    dMax = maxVal * 0.1

    # depending on the sign of the values we either have to add or subtract to get a wider range
    minFact = 1 if minVal < 0 else -1
    maxFact = 1 if maxVal > 0 else -1

    return [minVal + minFact * dMin, maxVal + maxFact * dMax]


def _getMinMax(plots):
    """
    Get min/max value of all passed plots along x- and y-direction.
    Returns list of list (first is x-direction, second is y-direction)
    """
    from ROOT import Double
    x,y = Double(0), Double(0) # needed for TGraphs
    minsX, minsY, maxsX, maxsY = [], [], [], []
    try:
        for p in plots:
            if p.InheritsFrom('TH1'):
                minsX.append(p.GetBinLowEdge(1)) # 0 is underflow bin
                maxsX.append(p.GetBinLowEdge(p.GetNbinsX()) + p.GetBinWidth(p.GetNbinsX()))
                minsY.append(p.GetBinContent(p.GetMinimumBin()))
                maxsY.append(p.GetBinContent(p.GetMaximumBin()))
            elif p.InheritsFrom('TGraph'):
                for i in range(p.GetN()):
                    p.GetPoint(i, x, y)
                    minsX.append(x.real - p.GetErrorXlow(i))
                    maxsX.append(x.real + p.GetErrorXhigh(i))
                    minsY.append(y.real - p.GetErrorYlow(i))
                    maxsY.append(y.real + p.GetErrorYhigh(i))
            elif p.InheritsFrom('TF1'):
                minsX.append(p.GetXmin())
                maxsX.append(p.GetXmax())
                minsY.append(p.GetMinimum())
                maxsY.append(p.GetMaximum())
            else:
                raise ValueError
    except ValueError:
        print('Can\'t get min/max from object not inheriting from TH1, TF1 or TGraph')
        return None # don't handle this error, somewhere upstream this will fail

    return [widenRange(min(minsX), max(maxsX)),
            widenRange(min(minsY), max(maxsY))]


def _getAxisRange(axPlotRange, axRange):
    """
    Determine the plotting range along one axis.
    If the axRange is None or both entries are None the axPlotRange is returned.
    If one of the entries is None, the other will be used and the None entry will
    be replaced by the corresponding entry from axPlotRange.
    """
    if axRange is not None and len(axRange) < 2:
        print('Axis range needs to have at least two values but passed is: {}'.format(axRange))
        return axPlotRange

    if axRange is None  or (axRange[0] is None and axRange[1] is None):
        return axPlotRange

    axMin, axMax = axPlotRange # unpack (for a bit less typing)

    if axRange[0] is None:
        return [axMin, axRange[1]]
    if axRange[1] is None:
        return [axRange[0], axMax]

    return axRange


def _setupPlotHist(canvas, xRange, yRange, plots, xlab, ylab):
    """
    Set up a drawing frame for the canvas.
    If both ranges are specified, both are used, if only one is specified
    only that one is used and the other is set to the min/max values of the plots
    in that direction.
    Setting one of the entries in a range to None will result in the usage of the
    min/max value of the plots in that direction.
    """
    # If a 'global' x and or y label is set, the frame is needed for displaying
    # Similarly if we don't have a histogram the plot histo is needed for displaying
    histInPlots = any([p.InheritsFrom('TH1') for p in plots])
    if xRange is None and yRange is None and not ylab and not xlab and histInPlots:
        return None

    xPlotRange, yPlotRange = _getMinMax(plots)

    x = _getAxisRange(xPlotRange, xRange)
    y = _getAxisRange(yPlotRange, yRange)

    if canvas.GetLogx() == 1 and x[0] <= 0:
        print('min value for x axis is {0} but logscale is set, setting it to 0.1'.format(x[0]))
        x[0] = 0.1
    if canvas.GetLogy() == 1 and y[0] <= 0:
        print('min value for y axis is {0} but logscale is set, setting it to 0.1'.format(y[0]))
        y[0] = 0.1


    plotHist = canvas.DrawFrame(x[0], y[0], x[1], y[1])

    if xlab: plotHist.SetXTitle(xlab)
    if ylab: plotHist.SetYTitle(ylab)

    return plotHist


def _setupCanvas(can, **kwargs):
    """
    Setup a TCanvas for drawing
    """
    from utils.miscHelpers import createRandomString
    from ROOT import TCanvas
    canName = kwargs.pop('name', createRandomString())

    size = kwargs.pop('size', [])
    if len(size) < 2:
        size = [800, 800]

    if can is None:
        can = TCanvas(canName, '', size[0], size[1])
    can.Clear()
    # Set logscales here, since information is needed for setting up the plot histogram
    can.SetLogy(kwargs.pop('logy', False))
    can.SetLogx(kwargs.pop('logx', False))

    if kwargs.pop('grid', False):
        can.SetGrid()

    return can


def mkplot(plots, **kwargs):
    """
    Small helper function to get a quick and dirty plot of a list of plottable root objects
    for jupyter / ipython

    Options:
    - ret: return the created TCanvas after plotting (default false)
    - name: create the TCanvas with the passed name instead of a random string
    - leg: draw a legend (using the names of the objects if no list of legend entries is
    passed). It is sufficient to pass only the legEntries if a legend is wanted. Also if
    the plots are passed as a dict the keys will automatically be used as legend entries
    - legEntries: list of keys to be used in the legend
    - legPos: position of legend in plot.
       Possible horizontal positions: 'right', 'left', 'center',
       Possible vertical positions: 'top', 'bottom'
       Strings can apear in any order and concatenated as well as abbreviated as long as
       three characters are used for each position argument. Default is 'bottom right'
    - {x,y}Range: define plotting range
    - {x,y}Label: axis labels
    - drawOpt: additional (ROOT) draw options to be passed to the Draw() method
    - useCan: use an existing canvas instead of creating a new one
    - colors: override default colors. List of VALID colors indices in ROOTs color index
    """
    from collections import Iterable

    # collect all options as a list of tuples, to pass them on to the the plotOnCanvas
    # function
    if not isinstance(plots, Iterable): plots = [plots] # allow to pass a single plot

    # if plots is a dictionary, automatically use the keys as legend entries, unless
    # they have already been specified by the corresponding argument
    legEntries = kwargs.pop('legEntries', [])
    if isinstance(plots, dict):
        if not legEntries: # first get keys, since plots won't be a dict after getting the values
            legEntries = plots.keys()
        plots = plots.values()

    plotOptions = []

    if legEntries or kwargs.pop('leg', False):
        legend = _setupLegend(kwargs.pop('legPos', ''))
        plotOptions.append(('leg', legend))
    if legEntries:
        plotOptions.append(('legEntries', legEntries))

    if 'drawOpt' in kwargs: plotOptions.append(('drawOpt', kwargs.pop('drawOpt')))
    if 'colors' in kwargs: plotOptions.append(('colors', kwargs.pop('colors')))

    existingCan = kwargs.pop('useCan', None)
    can = _setupCanvas(existingCan, **dict(kwargs))

    plotHist = _setupPlotHist(can, kwargs.pop('xRange', None), kwargs.pop('yRange', None),
                              plots, kwargs.pop('xLabel', ''), kwargs.pop('yLabel', ''))


    plotOnCanvas(can, plots, **dict(plotOptions)).Draw()

    saveName = kwargs.pop('saveAs', None)
    if saveName is not None and saveName:
        can.SaveAs(saveName)

    if kwargs.pop('ret', False) or existingCan is not None:
        return can
    else:
        global _drawCanvas
        _drawCanvas.append(can)
