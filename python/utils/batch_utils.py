import json

def filter_batch_step(statuses, excodes):
    """
    If a batch job has exited, a .batch step is added. This function gets the
    information from them and replaces the information from the original with it,
    the .batch step has been run, otherwise the original will be retained.
    """
    from miscHelpers import flatten

    # collect a list of all job ids that have to be removed / 'handled'
    alljobs = list(flatten(statuses.values()))
    rmjobs = []
    for job in alljobs:
        # if '.batch' in job and job.replace('.batch', '') in alljobs:
        if '.'.join([job, 'batch']) in alljobs:
            rmjobs.append(job)

    # in the statuses we only have to remove the jobs from rmjobs from the appropriate list
    # afterwards we simply remove keys with an empty list
    for job in rmjobs:
        batch_job = '.'.join([job, 'batch'])
        for _, status_jobs in statuses.iteritems():
            if job in status_jobs:
                if batch_job in status_jobs:
                    status_jobs.remove(batch_job)
                else:
                    status_jobs.remove(job)
            else:
                if batch_job in status_jobs:
                    status_jobs.remove(batch_job)
                    status_jobs.append(job)

    rmstats = []
    for status in statuses:
        if len(statuses[status]) == 0:
            rmstats.append(status)

    for stat in rmstats:
        del statuses[stat]

    # from the exit codes we simply have to 'move' the .batch values to the 'normal ones'
    for job in rmjobs:
        batch_job = '.'.join([job, 'batch'])
        excodes[job] = excodes[batch_job]
        del excodes[batch_job]


def parse_sacct_output(output):
    """
    Parse the output of sacct -b -n -P -j JobID
    and return a dict containing information about different exit statuses
    """
    def cond_add_to_dict(d, key, value):
        """
        Add value to dict d under key by creating a list or adding to it
        """
        if key in d:
            d[key].append(value)
        else:
            d[key] = [value]

    status_dict = {}
    exit_code_dict = {}
    for output_line in output:
        [job_id, status, exit_code] = output_line[0].strip().split('|')

        cond_add_to_dict(status_dict, status, job_id)
        exit_code_dict[job_id] = exit_code

    filter_batch_step(status_dict, exit_code_dict)

    return [status_dict, exit_code_dict]


def check_batch_job(job_id):
    """
    Check one batch job, with all the necessary info
    """
    from utils.miscHelpers import tail

    sacct_cmd = ['sacct', '-b', '-n', '-P', '-j', job_id]
    [status, excode] = parse_sacct_output(tail(sacct_cmd))

    def get_exit_codes(stat_dict, excodes, status):
        """
        Get all exit codes for a given status
        """
        if status in stat_dict:
            return [(j, excodes[j]) for j in stat_dict[status]]
        else:
            return None

    completed = get_exit_codes(status, excode, 'COMPLETED')
    if len(status) == 1 and completed is not None:
        exitZero = [e == '0:0' for (_, e) in completed]
        if not all(exitZero):
            for (i,(j, e)) in enumerate(completed):
                if not exitZero[i]:
                    print('{} has COMPLETED but exit code is {}'.format(j, e))
        else: # if all jobs have exited with zero, we don't need to print info (at least too much of it)
            # print('For {} all tasks exited with 0'.format(job_id))
            return True

    # if we go past here, print info, but don't indicate success
    failed = get_exit_codes(status, excode, 'FAILED')
    if failed is not None:
        for (j, e) in failed:
            print('Job {} FAILED with exit code {}'.format(j, e))

    running = get_exit_codes(status, excode, 'RUNNING')
    if running is not None:
        for (j, _) in running:
            print('Job {} is still RUNNING'.format(j))

    pending = get_exit_codes(status, excode, 'PENDING')
    if pending is not None:
        for (j, _) in pending:
            print('Job {} is still PENDING'.format(j))

    cancelled = get_exit_codes(status, excode, 'CANCELLED')
    if cancelled is not None:
        for (j, _) in cancelled:
            print('Job {} has been CANCELLED'.format(j))

    return False


def check_batch_file(jsonfile):
    """
    Check all entries of the batch job info in one batch job json file
    """
    with open(jsonfile, 'r') as f:
        batchdata = json.load(f)

    job_ids = []
    for entry in batchdata:
        job_ids.append(str(entry['job_id']))

    return check_batch_job(','.join(job_ids))


def get_job_id(output):
    """Get the job id from the sbatch output"""
    import re
    print(output) # print the output to the screen so the user can see what is going on
    rgx = r'([0-9]+)'
    m = re.search(rgx, output)
    if m:
        return int(m.group(1))
    return None


def append_to_json(json_file, data):
    """If the json_file already exists, append the new json_data line to it, otherwise create it"""
    # convert passed line into list, so that the json in the file can be appended to it
    data = [data]
    # check if file exists, if so add to data
    try:
        f = open(json_file, 'r')
        json_data = json.load(f)
        f.close()

        for d in json_data:
            data.append(d)
    except IOError:
        pass

    with open(json_file, 'w') as f:
        json.dump(data, f, indent=2, sort_keys=True)
