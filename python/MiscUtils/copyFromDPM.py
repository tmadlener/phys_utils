#!/usr/bin/env python

import subprocess as sp
import argparse
import sys
import os

def listSubDirs(directory):
    dpns_ls = sp.Popen(["dpns-ls {0}".format(directory)], stdout=sp.PIPE,
                       stderr=sp.PIPE, shell=True)
    (out, err) = dpns_ls.communicate()
    subDirs = out.split('\n')
    subDirs.remove('')

    return subDirs


def copySingleFile(fullPath, destPath):
    cp_cmd = 'xrdcp -P -N root://hephyse.oeaw.ac.at'
    xrdcp = sp.Popen(["{0}/{1} {2}".format(cp_cmd, fullPath, destPath)], shell=True)
    if args.verbosity > 0:
        print("Copying file \'{0}\' to \'{1}\'".format(fullPath, destPath))
    xrdcp.wait() # wait for process to finish


def copyFiles(fileList, destBase):
    """
    Copy all files from the fileList while figuring out wher exactly in the destBase
    they should be stored
    """
    print("Copying files")
    destDirList = os.listdir(destBase)
    destDirs = {} # make a map of job names to directories for easier lookup
    for d in destDirList:
        # remove crab_ prefix from dirname and store the 'full' path to the
        # results directory
        # print(d)
        destDirs["_".join(d.split("_")[1:])] = "/".join([destBase, d, "results"])

    for f in fileList:
        # the way the jobs are started, the last two directories are the date and the
        # result subdir (one for every 1000 jobs), making the 4th last the directory
        # with the same name as the crab job in the destBase
        jobName = f.split('/')[-4]
        copySingleFile(f, destDirs[jobName])


class DPMDirBuilder():
    """
    Directory builder on dpm, making an index of the requested task (subfolder)
    """
    def __init__(self, taskname, user="thmadlen"):
        self.taskname = taskname
        self.fileList = []
        self.basedir = "".join(["/dpm/oeaw.ac.at/home/cms/store/user/", user])
        self.currentDir = ''

        availDirs = listSubDirs(self.basedir)
        if taskname in availDirs:
            self.fileList.append(taskname)
            # self.fileList = availDirs
        else:
            print("{0} not found in {1}".format(taskname, self.basedir))
            print("Content:")
            for d in availDirs: print(d)
            sys.exit(1)

        print('Building index')
        self.buildIndex()


    def buildIndex(self):
        """Descend in every directory until the root files are found (then stop)"""
        # check if we already have root files, if so break out of recursion
        if self.fileList[0].endswith('.root'):
            return self.fileList

        fullDirList = []
        for d in self.fileList:
            if args.verbosity > 0:
                print("Getting subdirectories of \'{0}\'".format(d))
            fileList = listSubDirs("/".join([self.basedir, d]))
            for sd in fileList:
                fullDirList.append("/".join([d, sd]))

        self.fileList = fullDirList
        return self.buildIndex()


    def getFileList(self):
        """Return only list of root files"""
        return ("/".join([self.basedir, f]) for f in self.fileList if f.endswith('.root'))


"""
Arg parse
"""
parser = argparse.ArgumentParser(description='script for copying from dpm')
parser.add_argument('taskname', help='name of task (base folder on dpm)')
parser.add_argument('resbase', help='result base dir (where to store the stuff')
parser.add_argument('-v', '--verbose', help='verbosity of print outs',
                    dest='verbosity', action='store', type=int, default=0)
args = parser.parse_args()

dirBuilder = DPMDirBuilder(args.taskname)
filesToCopy = dirBuilder.getFileList()

copyFiles(filesToCopy, args.resbase)
