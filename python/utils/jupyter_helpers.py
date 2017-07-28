def drawVarToHist(t, h, var, cut='', weight=None):
    """
    Fill the variable 'var' stored in the TTree 't' into the histogram 'h'.
    Passing a cut expression and a weight expression is possible as well and will be
    applied accordingly.
    """
    plotStr = " >> ".join([var, h.GetName()])
    if weight is not None:
        if cut:
            cut = " * ".join([weight, "(" + cut + ")"])
        else:
            cut = weight

    return t.Draw(plotStr, cut)


def combineCuts(cuts):
    """
    Concatenate a list of cuts in such a manner that the resulting
    expression will be the AND combination of all passed cuts.
    """
    safeCuts = ["".join(["(", c , ")"]) for c in cuts]
    return " && ".join(safeCuts)


def setHistOpts(h):
    """
    Set some default hist options for use in jupyter notebooks.
    """
    h.SetStats(0)
    h.Sumw2()
