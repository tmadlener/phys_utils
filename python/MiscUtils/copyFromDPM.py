#!/usr/bin/env python

import subprocess as sp
import argparse
import sys
import os
import multiprocessing

def listSubDirs(directory):
    dpns_ls = sp.Popen(["dpns-ls {0}".format(directory)], stdout=sp.PIPE,
                       stderr=sp.PIPE, shell=True)
    (out, err) = dpns_ls.communicate()
    subDirs = out.split('\n')
    subDirs.remove('')

    return subDirs


def isFilePresent(dpmFilePath, destPath, forceRemove):
    fileName = dpmFilePath.split('/')[-1]
    destFile = '/'.join([destPath, fileName])
    filePresent = os.path.isfile(destFile)
    if filePresent:
        if args.verbosity > 0 and not forceRemove:
            print("File \'{0}\' is already present in \'{1}\'".format(fileName, destPath))
        if forceRemove:
            os.remove(destFile)
            return False # file was present but now is removed

        return True # if file present and not removed

    return False # if not present return false



def copySingleFile((fullPath, destPath)):
    global cpCounter
    global presCounter
    if isFilePresent(fullPath, destPath, args.force):
        with presCounter.get_lock():
            presCounter.value += 1
        return

    cp_cmd = 'xrdcp -P -N root://hephyse.oeaw.ac.at'
    xrdcp = sp.Popen(["{0}/{1} {2}".format(cp_cmd, fullPath, destPath)], shell=True)
    if args.verbosity > 0:
        # It is very probable that printouts get mixed here for different threads
        # COULDDO: locking for printing
        curWorker = multiprocessing.current_process()
        print("Copying file \'{0}\' to \'{1}\' using worker {2}".format(fullPath, destPath,
                                                                        curWorker.name))
    with cpCounter.get_lock(): # increase copy-counter
        cpCounter.value += 1

    xrdcp.wait() # wait for process to finish


def initPool(counter, pCounter):
    global cpCounter
    cpCounter = counter
    global presCounter
    presCounter = pCounter


def copyFiles(fileListGen, destBase, nThreads=4, dryrun=False):
    """
    Copy all files from the fileList while figuring out wher exactly in the destBase
    they should be stored
    """
    from tqdm import tqdm

    print("Copying files, using {0} threads".format(nThreads))
    destDirList = os.listdir(destBase)
    destDirs = {} # make a map of job names to directories for easier lookup
    for d in destDirList:
        # remove crab_ prefix from dirname and store the 'full' path to the
        # results directory
        # print(d)
        destDirs["_".join(d.split("_")[1:])] = "/".join([destBase, d, "results"])

    # get the result directory from the file name
    # the way the jobs are started, the last two directories are the date and the
    # result subir (one for every 1000 jobs), making the 4th last the directory
    # with the same name as the crab job in the destBase
    getResDir = lambda x : destDirs[x.split('/')[-4]]

    # make a generator expression to get a list that contains the corresponding
    # destination for every file and zip the two lists together to pass them
    # to the worker ppol

    fileList = [f for f in fileListGen] # get list from generator (not resuable else)
    destForFile = (getResDir(f) for f in fileList)
    fileDestList = zip(fileList, destForFile)

    if dryrun:
        print('Running would copy {0} files'.format(len(fileList)))
        return

    counter = multiprocessing.Value('i', 0)
    pCounter = multiprocessing.Value('i', 0)

    copyPool = multiprocessing.Pool(nThreads, initializer = initPool, initargs = (counter,pCounter))
    with tqdm(total=len(fileList), desc='Copying', ncols=70) as pbar:
        for _ in copyPool.imap_unordered(copySingleFile, fileDestList):
            pbar.update()

    print('Copied {0} files. {1} were already present and not overwritten'.format(counter.value, pCounter.value))


class DPMDirBuilder():
    """
    Directory builder on dpm, making an index of the requested task (subfolder)
    """
    def __init__(self, taskname, user):
        self.taskname = taskname
        self.fileList = []
        self.basedir = "".join(["/dpm/oeaw.ac.at/home/cms/store/user/", user])
        self.currentDir = ''

        availDirs = listSubDirs(self.basedir)
        if not availDirs:
            print('{0} does not exist.'.format(self.basedir))
            sys.exit(1)

        if taskname in availDirs:
            self.fileList.append(taskname)
        else:
            print("{0} not found in {1}".format(taskname, self.basedir))
            print("Content:")
            for (i,d) in enumerate(availDirs):
                print('[{0}]: {1}'.format(i, d))

            while True:
                inp = raw_input('Choose from above or \'q\' to quit: ')
                if inp.upper() == 'Q':
                    sys.exit(1)
                else:
                    try:
                        dec = int(inp)
                        if dec >= len(availDirs): raise ValueError
                        break
                    except ValueError:
                        print('Enter number between 0 and {0} or \'q\' to quit'.format(len(availDirs) - 1))

            self.fileList.append(availDirs[dec])

        print('Building index')
        self.buildIndex()


    def buildIndex(self):
        """Descend in every directory until the root files are found (then stop)"""
        # check if we already have root files, if so break out of recursion
        if any(f.endswith('.root') for f in self.fileList):
            return self.fileList

        fullDirList = []
        for d in self.fileList:
            if args.verbosity > 1:
                print("Getting subdirectories of \'{0}\'".format(d))
            fileList = listSubDirs("/".join([self.basedir, d]))
            for sd in fileList:
                fullDirList.append("/".join([d, sd]))

        self.fileList = fullDirList
        return self.buildIndex()


    def getFileList(self):
        """Return only list of root files"""
        return ("/".join([self.basedir, f]) for f in self.fileList if f.endswith('.root'))



def getFilesToCopy(allFiles, subtasks, excltasks):
    """
    Get the list (generator) of files that should actually be copied from all the files in the index.
    """
    if subtasks is None and excltasks is None:
        return allFiles

    # check if any of the specified subtasks is actually in the filename
    inSubTaskList = lambda x, l: any(t for t in l if t in x)

    if subtasks is None:
        return (f for f in allFiles if not inSubTaskList(f, excltasks))
    if excltasks is None:
        return (f for f in allFiles if inSubTaskList(f, subtasks))

    return (f for f in allFiles if inSubTaskList(f, subtasks) and not inSubTaskList(f, excltasks))



if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='script for copying from dpm')
    parser.add_argument('taskname', help='name of task (base folder on dpm)')
    parser.add_argument('resbase', help='result base dir (where to store the stuff')
    parser.add_argument('-v', '--verbose', help='verbosity of print outs',
                        dest='verbosity', action='store', type=int, default=0)
    parser.add_argument('-t', '--subtasks', help='Get only one of all possible sub-tasks of the whole task',
                        dest='subtasks', action='store', nargs='+')
    parser.add_argument('-n', '--nThreads', action='store', type=int, default=4, dest='nThreads',
                        help='number of threads to be used for copying')
    parser.add_argument('-u', '--user', action='store', dest='user', default='thmadlen',
                        help='user name on dpm')
    parser.add_argument('-f', '--force', help='Force copying even if file is already present',
                        action='store_true', default=False)
    parser.add_argument('--dryrun', dest='dryrun', default=False, action='store_true',
                        help='Only get the number of files to copy, but don\'t actually copy them.')
    parser.add_argument('-x', '--exclude-subtasks', action='store', nargs='+', dest='excltasks',
                        help='Do not retrieve the sub-tasks listed here even if listed in the subtasks list')

    args = parser.parse_args()

    dirBuilder = DPMDirBuilder(args.taskname, args.user)
    allFiles = dirBuilder.getFileList()

    filesToCopy = getFilesToCopy(allFiles, args.subtasks, args.excltasks)

    copyFiles(filesToCopy, args.resbase, args.nThreads, args.dryrun)
