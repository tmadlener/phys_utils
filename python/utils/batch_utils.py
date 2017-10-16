def parse_sacct_output(output):
    """
    Parse the output of sacct -b -n -P -j JobID
    and return a dict containing information about different exit statuses
    """
    def cond_add_to_dict(d, key, value):
        """
        Add value to dict d under key. If key is already present, a list will be generated
        for the values and the value will be stored there-in
        """
        if key in d:
            if not hasattr(d[key], '__iter__'): # don't create lists of lists of lists
                d[key] = [d[key]]
            d[key].append(value)
        else:
            d[key] = value

    status_dict = {}
    exit_code_dict = {}
    for output_line in output:
        [job_id, status, exit_code] = output_line[0].strip().split('|')

        # filter out the .batch jobstep (which should always be present)
        if '.batch' in job_id: continue

        cond_add_to_dict(status_dict, status, job_id)
        exit_code_dict[job_id] = exit_code

    return [status_dict, exit_code_dict]


def check_batch_job(batchinfo):
    """
    Check one batch job, with all the necessary info
    """
    from utils.miscHelpers import tail

    job_id = str(batchinfo['job_id']) # needed as string later
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

    return False


def check_batch_file(jsonfile):
    """
    Check all entries of the batch job info in one batch job json file
    """
    import json
    with open(jsonfile, 'r') as f:
        batchdata = json.load(f)

    file_status = []
    for entry in batchdata:
        file_status.append(check_batch_job(entry))

    return all(file_status)
