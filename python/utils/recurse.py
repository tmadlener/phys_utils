import re

def recurseOnFile(f, func, dirFunc = None):
    """
    Generic root file recursion function that traverses all TDirectories that can be found in a TFile.
    For every key that is found in the file it is checked if it inherits from TDirectory and if not the
    function passed in via the func paramter is executed.
    * f is the TFile or TDirectory to recurse on.
    * func is any function that takes a TObject as its single argument.
    * dirFunc is any function taking a TDirectory as its single argument that is executed on each directory but
      should be outside of the recursion (e.g. obtaining the current path)
    """
    for key in f.GetListOfKeys():
        obj = key.ReadObj()
        if not obj.InheritsFrom('TDirectory'):
            func(obj)
        else:
            dirFunc(obj)
            recurseOnFile(obj, func, dirFunc)


class ObjectCollector:
    """
    Functor for collecting all Objects that inherit from a specific class specified at
    construction where the name matches a passed regex.

    Creates a dictionary internally with the names as keys and the collected objects as
    values.
    """

    def __init__(self, rgx, className):
        """
        Create empty dict and compile passed regex
        """
        self.objects = {}
        self.regex = re.compile(rgx)
        self.clname = className


    def __call__(self, obj):
        """
        Interface required by the recursion implementation.
        Checks if object inherits from the class used in the constructor if its name
        matches the regex.

        NOTE: currently does not handle objects in subdirectories correctly!
        TODO: subdirectory handling by storing the "full name" of the objects.
        """
        if (obj.InheritsFrom(self.clname)) and self.regex.search(obj.GetName()):
            self.objects[obj.GetName()] = obj
