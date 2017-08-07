import os
import matplotlib.pyplot as plt
import numpy as np
import re


def load_perf_file(file, method=""):
    if method == "":
        method = os.path.basename(file).split('_')[0]

    with open(file, 'r') as f:
        s = f.read()

    genome = os.path.basename(file).split('__')[1]
    region = os.path.basename(file).split('__')[2].split('.perf')[0]

    lines = s.split('\n')
    usertime = float(lines[1].split(': ')[1])
    systemtime = float(lines[2].split(': ')[1])
    cpu_usage = float(lines[3].split(': ')[1].split('%')[0])
    memory = int(lines[9].split(': ')[1].split('%')[0])

    filesize = os.stat(file.replace('.perf', '.log')).st_size

    return [genome, method, region, usertime, systemtime, cpu_usage, memory, filesize]


def get_metric(df, column, regions):
    return np.array([df[df['region'] == r][column].values for r in regions])


def compare_metrics(df, methods, regions, column, save=False, relative=None, log=False):
    plt.figure(figsize=(25, 6))

    dfs = [df[df['method'] == m] for m in methods]
    metrics = [get_metric(d, column, regions) for d in dfs]

    x = np.arange(len(regions))
    N = len(methods) + 1

    for i, (m, method) in enumerate(zip(metrics, methods)):
        if relative is None:
            plt.bar(x+1/N*i, m, width=1/N, label=method)
        else:
            plt.bar(x+1/N*i, m/metrics[relative], width=1/N, label=method)
        # print(method, m)

    plt.xticks(x, regions, rotation=15)
    plt.title(column, fontsize=25)
    plt.legend(fontsize=16)

    if log:
        plt.yscale('log')

    if save:
        plt.savefig("images/{0}.png".format(column), format='png')


def load_samtoram_perf(file, method=''):

    with open(file, 'r') as f:
        s = f.read()
    lines = s.split('\n')

    if "Command exited" in lines[0]:
        lines = lines[1:]

    if method == '':
        method = os.path.basename(lines[0].split(', ')[1]).strip(' "')

    usertime = float(lines[1].split(': ')[1])
    systemtime = float(lines[2].split(': ')[1])
    cpu_usage = float(lines[3].split(': ')[1].split('%')[0])
    memory = int(lines[9].split(': ')[1].split('%')[0])

    logfile = file.replace('.perf', '.log')

    with open(logfile, 'r') as f:
        s = f.read()
    lines = s.split('\n')

    filesize = int(lines[4].split(' = ')[-1].strip(' *'))
    compression = float(lines[5].split(' = ')[-1].strip(' *'))

    return [method, usertime, systemtime, cpu_usage, memory, filesize, compression]


def load_samtobam_perf(file, method=''):

    with open(file, 'r') as f:
        s = f.read()
    lines = s.split('\n')

    if "Command exited" in lines[0]:
        lines = lines[1:]

    if method == '':
        method = lines[0].split(' ')[-1].strip(' "').replace('.sam', '.bam')

    usertime = float(lines[1].split(': ')[1])
    systemtime = float(lines[2].split(': ')[1])
    cpu_usage = float(lines[3].split(': ')[1].split('%')[0])
    memory = int(lines[9].split(': ')[1].split('%')[0])

    logfile = file.replace('.perf', '.log')

    with open(logfile, 'r') as f:
        s = f.read()
    lines = s.split('\n')

    filesize = int(lines[1].split('\t')[0].split(': ')[1])
    filesize_org = int(lines[10].split('\t')[0].split(': ')[1])
    compression = filesize_org / filesize

    return [method, usertime, systemtime, cpu_usage, memory, filesize, compression]
