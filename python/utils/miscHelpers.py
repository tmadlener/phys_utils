import os

def condMkDir(path):
    """
    check if the folder with the given path exists and if not create it
    Taken from: http://stackoverflow.com/a/14364249/3604607
    """
    try:
        os.makedirs(path)
    except OSError as e:
        if not os.path.isdir(path):
            print('Caught error \'{}\' while trying to create directory \'{}\''.format(e.strerror, path))
            raise


def condMkDirFile(filename):
    """
    Check if the folder exists to store the passed file.
    The filename (i.e. everything after the last "/") is removed and then the directory is conditionally made
    """
    [path, name] = os.path.split(filename)
    condMkDir(path)


def flatten(l):
    """
    Flatten any past list into a 1D-list.
    Taken from here: http://stackoverflow.com/a/2158532/3604607
    """
    for elem in l:
        # if isinstance(elem, collections.Iterable) and not isinstance(elem, basestring):
        if hasattr(elem, "__iter__") and not isinstance(elem, basestring):
            for sub in flatten(elem):
                yield sub
        else:
            yield elem


def strArgList(d):
    """
    Create an argument list processable by subprocess.call() from the passed dict, where the keys are used as flags
    and the values as the arguments to the corresponding flag
    """
    argList = []
    for (k, v) in d.items():
        if hasattr(v, "__iter__") and not isinstance(v, basestring):
            argList.append(["--{}".format(k)] + [str(s) for s in v])
        else:
            argList.append(["--{}".format(k), str(v)])

    return list(flatten(argList))


def mergeDicts(*dicts):
    """
    Merge passed dicts together into a new dict by shallow copying.
    Duplicate keys get the values of that dict in which the key last appeared (Precedence goes to key value
    pairs in latter dicts).
    From: http://stackoverflow.com/a/26853961
    """
    merged = {}
    for dictionary in dicts:
        merged.update(dictionary)
    return merged


def filterDict(d, key):
    """
    Filter a dictionary to contain only keys matching key (string only currently)
    """
    return {k: v for (k, v) in d.iteritems() if key in k}


def getAllFilesFromDir(directory, ext):
    """
    Get all files with the specified extension from the directory
    """
    return [f for f in os.listdir(directory)
            if os.path.isfile(os.path.join(directory, f)) and f.endswith(ext)]


def getRapPtStr(fullName):
    """
    Get the rapX_ptY ending of the full name where, all characters are lower cased
    """
    strList = fullName.lower().split("_")
    return "_".join([strList[-2], strList[-1]])


def getRapPt(fullName):
    """
    Get the numerical values of the pt and rapidity bin (after removing anything after the first dot)
    NOTE: assumes that this follows the convention that rapX_ptY are always at the end of the full name
    """
    strList = fullName.partition(".")[0].lower().split("_")
    return[int(strList[-2][3:]), int(strList[-1][2:])]


def getRapPtLbl(rapBin, rapBinning, ptBin, ptBinning):
    """
    Get the bin information as tex string
    """
    rapMin = rapBinning[rapBin - 1]
    rapMax = rapBinning[rapBin]
    ptMin = ptBinning[ptBin - 1]
    ptMax = ptBinning[ptBin]

    return "".join(["$", str(rapMin), " < |y| < ", str(rapMax), ", ", 
                    str(ptMin), " < p_{T} < ", str(ptMax), "$"])
