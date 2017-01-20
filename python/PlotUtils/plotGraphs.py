#!/usr/bin/env python

import argparse
import json
import math

from utils.recurse import recurseOnFile

class GraphCollector(object):
    """
    Functor that collects all TGraphs saved in a file and stores them in a dictionary.
    """

    def __init__(self):
        """
        Create new (and empty) dict and set the current path to empty string
        """
        self.graphDict = {}
        self.currentPath = ''

    def __call__(self, obj):
        """
        Check if the object inherits from TGraph and store it to the internal dictionary
        """
        if (obj.InheritsFrom('TGraph')):
            # The TDirectory::GetPath() method returns in the format /file/on/disk:/path/in/file
            splitPath = self.currentPath.split(':')
            path = ''
            if (len(splitPath) > 2):
                path = splitPath[1] + '/'

            graphName = ''.join([path, obj.GetName()])
            self.graphDict[graphName] = obj

    def setPath(self, directory):
        """
        Set the internal path variable to the current directory
        """
        self.currentPath = directory.GetPath()

    def getDict(self):
        """
        Get the dictionary
        """
        return self.graphDict


def setupCanvas(name, title, canJson):
    """
    Create and setup a canvas for plotting
    """
    can = r.TCanvas(name, title, canJson['size'][0], canJson['size'][1])
    can.SetFillColor(canJson["fillColor"])
    if canJson["gridX"]:
        can.SetGridx()
    if canJson["gridY"]:
        can.SetGridy()
    if canJson["logX"]:
        can.SetLogx()
    if canJson["logY"]:
        can.SetLogy()

    return can


def setupLegend(legJson):
    """
    Create legend from the passed json settings
    """
    pos = legJson["position"]
    leg = r.TLegend(pos[0], pos[1], pos[2], pos[3])
    leg.SetFillColor(legJson["fillColor"])
    leg.SetTextFont(legJson["textFont"])
    leg.SetTextSize(legJson["textSize"])
    leg.SetBorderSize(legJson["borderSize"])

    return leg


def createPlotHisto(canvas, axesJson):
    """
    Create the TH1F to plot the graphs on from the canvas and the axes settings
    """
    xRange = axesJson["xRange"]
    yRange = axesJson["yRange"]
    plotHist = canvas.DrawFrame(xRange[0], yRange[0], xRange[1], yRange[1])
    plotHist.SetXTitle(axesJson["xTitle"])
    plotHist.SetYTitle(axesJson["yTitle"])
    # plotHist.GetYaxis().SetTitleOffset(1.4)

    if "labSize" in axesJson:
        plotHist.SetLabelSize(axesJson["labSize"], "X")
        plotHist.SetLabelSize(axesJson["labSize"], "Y")

    if "titleSize" in axesJson:
        plotHist.SetTitleSize(axesJson["titleSize"], "X")
        plotHist.SetTitleSize(axesJson["titleSize"], "Y")

    return plotHist


def getColorAndStyle(i, markerJson):
    """
    get a color and a style integer for the i-th plot and the passed markerJson
    """
    cols = markerJson["color"]
    styls = markerJson["style"]
    return [ cols[ i % len(cols)], styls[ i % len(styls) ] ]


def getMinMax(graph):
    """
    Get the min and the max y-val (without errors) from the passed graph
    """
    ymin = float('inf')
    ymax = float('-inf')
    for i in range(graph.GetN()):
        x = r.Double(0)
        y = r.Double(0)
        graph.GetPoint(i, x, y)
        if y < ymin:
            ymin = y
        if y > ymax:
            ymax = y

        return [ymin, ymax]


def getMinMaxAll(graphDict):
    """
    Get the min and max value from all graphs stored in the graph dictionary
    (I.e. dict of dict)
    """
    ymin = float('inf')
    ymax = float('-inf')
    for foo, graphs in graphDict.iteritems():
        for bar, graph in graphs.iteritems():
            [gmin, gmax] = getMinMax(graph)
            if gmin < ymin:
                ymin = gmin
            if gmax > ymax:
                ymax = gmax

    return [ymin, ymax]


def removeNans(graph):
    """
    Remove points where either x or y are NaN
    """
    # collect all points to remove first in ascending order and remove them in reverse order as
    # TGraph::RemovePoint(i) changes the index of the points following i
    remPoints = []
    for i in range(graph.GetN()):
        x = r.Double(0)
        y = r.Double(0)
        graph.GetPoint(i, x, y)
        if math.isnan(x) or math.isnan(y):
            remPoints.append(i)

    for i in reversed(remPoints):
        graph.RemovePoint(i)
        print('Removed point {} from graph {}, because x or y were nan'.format(i, graph.GetName()))


"""
Arg parser
"""
parser = argparse.ArgumentParser(description='script for plotting TGraphs')
parser.add_argument('jsonFile', help='json file containing the main configuration that can be overriden by other args')
args = parser.parse_args()


"""
ROOT setup
"""
import ROOT as r
r.gROOT.ProcessLine("gErrorIgnoreLevel = 1001")
r.gROOT.SetBatch()


"""
Json file read in
"""
with open(args.jsonFile) as f:
    json = json.loads(f.read())


"""
Collect Graphs from file
"""
# collect all the graphs, regardless of their later usage
graphList = dict()
for fn in json['inputfiles']:
    f = r.TFile.Open(fn[0]) # the 0 element contains the full path to the filename
    gColl = GraphCollector()
    recurseOnFile(f, gColl, lambda d: gColl.setPath(d))

    graphList[fn[1]] = gColl.getDict()
    f.Close()

# if a 'global' yTitle is set, use it for every plot, else take the yTitle from each plot
globYTitle = json["axes"]["yTitle"]

# if the yRange is empty, get the min and max values of all graphs and use this as yRange
if not json["axes"]["yRange"]:
        [ymin, ymax] = getMinMaxAll(graphList)
        json["axes"]["yRange"] = [ymin * 1.1, ymax * 1.1]

"""
Plot all desired graphs
"""
# plot all desired graphs
for plot in json["plots"]:
    canvas = setupCanvas('tmpCanvas', 'tmpCanvas', json["canvas"])
    if not globYTitle:
        json["axes"]["yTitle"] = plot["yTitle"]

    plotHist = createPlotHisto(canvas, json["axes"])
    legend = setupLegend(json["legend"])

    plotCounter = 0 # needed to have different markers for graphs from different files
    for legEntryBase, graphs in graphList.iteritems():
        for i in range(len(plot["graphs"]) / 2): # even elements are graph names, odd are legend entry additions
            if plot["graphs"][2*i] in graphs: # only plot if the graph has been collected from the file
                graph = graphs[plot["graphs"][2*i]]
                removeNans(graph)
                # color and style
                [col, style] = getColorAndStyle(plotCounter, json["markers"])
                graph.SetMarkerColor(col)
                graph.SetMarkerStyle(style)
                graph.SetMarkerSize(json["markers"]["size"])
                graph.SetLineColor(col)

                graph.Draw('P')
                legEntry = legEntryBase
                if plot["graphs"][2*i + 1]: # check if a legend entry has been specified before adding it
                    legEntry = ", ".join([legEntryBase, plot["graphs"][2*i + 1]])

                legend.AddEntry(graph, legEntry, "ple")
                plotCounter += 1

        if json["legend"]["draw"]:
            legend.Draw()
        canvas.Update()

        if plot["text"]:
            textJson = json["decoration"]["text"]
            text = r.TLatex(textJson["position"][0], textJson["position"][1], plot["text"])
            text.SetTextSize(textJson["textSize"])
            text.Draw("same")

        if plot["file"]: # if a filename is specified use it
            filename = "".join([json["outbase"], "_", plot["file"], ".pdf"])
        else: # else use the name of the first graph as filename
            filename = "".join([json["outbase"], "_", plot["graphs"][0], ".pdf"])
        canvas.SaveAs(filename)
