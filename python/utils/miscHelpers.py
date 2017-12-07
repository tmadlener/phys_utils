import os
import re
import numpy as np

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
    Filter a dictionary to contain only keys matching key
    """
    return {k: v for (k, v) in d.iteritems() if re.search(key, k)}


def filterDictNot(d, key):
    """
    Filter a dictionary to contain only keys that do not match key
    """
    return {k: v for (k, v) in d.iteritems() if not re.search(key, k)}


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
    [rap, pt] = getRapPt(fullName)
    return "_".join(["rap" + str(rap), "pt" + str(pt)])


def getBinIdx(fullName, binVarRgx):
    """
    Get the bin index from the full name using a regular expression of the binned variable
    """
    rgx = r"".join([binVarRgx, "([0-9]+)"])
    m = re.search(rgx, fullName)
    if m:
        return int(m.group(1))

    print("Couldn't match regex to find index of binned variable: \'{}\' in \'{}\'".format(binVarRgx, fullName))
    return -1


def getRapPt(fullName):
    """
    Get the numerical values of the pt and rapidity bin by matching a regex against the
    passed full name.
    """
    rapIdx = getBinIdx(fullName, r"[Rr][Aa][Pp]_?")
    ptIdx = getBinIdx(fullName, r"[Pp][Tt]_?")

    if rapIdx < 0 or ptIdx < 0:
        print("Couldn't match regex to find rap and pt bin in \'{}\'".format(fullName))

    return[rapIdx, ptIdx]



def removeRapPt(fullName):
    """
    Remove the rapX_ptY info from the fullName and return a new string
    """
    rapPtBinRgx = r"[Rr][Aa][Pp]_?[0-9]+_[Pp][Tt]_?[0-9+]"
    return re.sub(rapPtBinRgx, "", fullName)


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


def getBinEdgeVals(fullName):
    """
    Get the numerical values of the bin, denoted in it's name that has the form:
    UpVtoXpY
    """
    rgx = r'([0-9]+(p[0-9]+)?)to([0-9]+(p[0-9]+)?)'
    m = re.search(rgx, fullName)
    if m:
        return [float(m.group(1).replace('p', '.')), float(m.group(3).replace('p', '.'))]

    print("Couldn't match regex to find bin edges of binning in name: \'{}\'".format(fullName))
    return None

def sanitizeInputFileNames(inputfiles):
    """
    Sanitize the input file name list (of lists) so that every file has a legend entry (defaults to empty)
    Merges all but the first entry in each list into one string that is then used as legend entry
    """
    for inf in inputfiles:
        n = len(inf)
        if n > 2:
            inf[1] = " ".join(inf[1:])
            del inf[-(n-2):]
        if n < 2:
            inf.append("")

    return inputfiles


def getAnyMatchRgx(rgxs):
    """
    Get a regex that matches all of the passed regexes without any special ordering using
    positive lookahead
    """
    rgxList = flatten(['(?=.*', r, ')'] for r in rgxs if r) # get the lookahead for every rgx
    return r''.join(rgxList)


def parseVarBinning(binningStr):
    """
    Parse the variable binning from the command line argument (passed to this function)
    Currently supported:
    1) list of floating point numbers as strings
    2) string in format 'X:Y:Z', the MATLAB format for getting a linearly spaced array of numbers
    3) string in forma 'X:Z,N', a linearly spaced array including X and Z with N entries in total (MATLAB linspace)
    Exits with value 1 if no parsing can be done
    """
    if len(binningStr) > 1:
        return [float(v) for v in binningStr]

    if re.search(r"(-?\d+(\.\d+):?){3}", binningStr[0]):
        [minVal, step, maxVal] = (float(v) for v in binningStr[0].split(':'))
        return np.arange(minVal, maxVal, step)

    if re.search(r"(-?\d+(\.\d+)?:?){2},\d+", binningStr[0]):
        tmp = binningStr[0].split(':')
        tmp2 = tmp[1].split(',')
        return np.linspace(float(tmp[0]), float(tmp2[0]), int(tmp2[1]))

    try:
        return [float(binningStr[0])]
    except ValueError:
        pass

    print("Could not convert \'{}\' into a proper binning".format(binningStr))


def createRandomString(n=32):
    """
    Create a random string containing upper and lower case letters as well as digits.
    NOTE: not cryptographically safe (see https://stackoverflow.com/questions/2257441/
    for more details)
    """
    from random import choice
    from string import ascii_letters, digits
    return ''.join(choice(ascii_letters + digits) for _ in range(n))


def getPartialMatcher(word, minchars, wrap=False):
    """
    Get a regex expression to partially match word (requiring at least minchars)
    characters to match at the beginning (see https://stackoverflow.com/questions/13405223/)
    If wrap, the begin of string and end of string will be prepended (reps. appended)
    """
    rgx = '|'.join([word[:i] for i in range(len(word), minchars - 1, -1)])
    rgx = ''.join(['(', rgx, ')']) # wrap in a group to have access to it from outside

    if wrap:
        rgx = r''.join(['^', rgx, '$'])

    return rgx


def tail(command):
    """
    Run the passed command (list of strings) via subprocess.Popen, capture the output
    and return it, similar to 'tail -f command'.

    https://stackoverflow.com/a/26006921
    """
    import subprocess
    popen = subprocess.Popen(command, stdout=subprocess.PIPE)
    for line in iter(popen.stdout.readline, ""):
        yield line,


def stringify(selection, reverse=False):
    """
    Get a string representation of a selection (as it is used e.g. in
    TTree::Draw) or in RooDataSet::reduce.  If reverse is set to True,
    the original representation can be obtained (with stripped
    whitespace)
    """
    repl_pairs = (
        ('>=', '_ge_'),
        ('<=', '_le_'),
        ('<', '_lt_'),
        ('>', '_gt_'),
        ('==', '_eq_'),
        ('!=', '_ne_'),
        ('&&', '_AND_'),
        ('||', '_OR_')
    )

    ret_str = selection.replace(' ', '')
    if reverse:
        for rep, sym in repl_pairs:
            ret_str = ret_str.replace(sym, rep)
    else:
        for sym, rep in repl_pairs:
            ret_str = ret_str.replace(sym, rep)

    return ret_str
