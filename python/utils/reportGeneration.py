from miscHelpers import getRapPt, getRapPtLbl

def sortRapPt(it): # sort according to rap and pt bin (rap takes precedence)
    return getRapPt(it)


def makeFigTex(plots, labels, caption = ""):
    """
    Produce a figure with subfloats (possibly split up into more than one figure)
    """
    tex = []
    tex.append("\\begin{figure}\n")
    nPlotted = 0
    nPlots = len(plots)
    for (plot, label) in zip(plots, labels):
        tex.append(getSubFloatStr(plot, label))
        nPlotted = nPlotted + 1
        if nPlotted % 2 == 0 and nPlotted < nPlots: # two plots per "line"
            tex.append("\n")
        if nPlotted % 6 == 0 and nPlotted < nPlots: # allow for a page break after 6 plots
            tex.append("\\end{figure}\n\n")
            tex.append("\\begin{figure}\n")

    if caption:
        tex.append("".join(["\\caption{", caption, "}\n"]))
    tex.append("\\end{figure}\n\n")

    return tex


def getSubFloatStr(plotPath, label):
    """Get Subfloat"""
    return "".join(["\\subfloat[", label, "]"
                    "{\\includegraphics[width=0.5\\linewidth]{", plotPath ,"}}\n"])


def getLabels(plots):
    """
    Get the labels to the plots
    """
    # hardcoded at the moment
    ptBinning = [10, 12, 14, 16, 18, 20, 22, 25, 30, 35, 40, 50, 70]
    rapBinning = [0, 1.2]

    lbls = []
    # if rapX_ptY is in plot the labels are the bin values
    if "rap" in plots[0] and "pt" in plots[0].lower():
        for p in plots:
            [rap, pt] = getRapPt(p)
            lbls.append(getRapPtLbl(rap, rapBinning, pt, ptBinning))

        return lbls

    # if one of the lambda abbrevitations is in the plot name, that is the label
    for p in plots:
        if "lth" in p: lbls.append("$\\lambda_{\\theta}$")
        if "lph" in p: lbls.append("$\\lambda_{\\phi}$")
        if "ltp" in p: lbls.append("$\\lambda_{\\theta\\phi}$")

    # here every case should be covered (to be checked)
    return lbls
