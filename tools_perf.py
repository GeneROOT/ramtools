"""Usage: tools_perf.py generate [-n NUMBER] [--out OUTFILE] GENOMETABLE
          tools_perf.py convert [--no-split] [-c ALG] [-N] [--out OUTFILE] SAMFILE ROOTFILE
          tools_perf.py run samview FILE VIEWS [RANGE] [-P] [-N] [--io] [--out FOLDER]  [--path PATH]...
          tools_perf.py run ramview FILE VIEWS [RANGE] [-P] [-N] [--io] [--out FOLDER] [--cache] [--stats] [--macro MACRO]  [--path PATH]...
          tools_perf.py parse [--out OUTFILE] LOGFILE...
          tools_perf.py parsetreestats TTREEPERFSTATS...

Preprocessing and postprocessing for evaluating the performance of ramtools functions

Arguments:
  GENOMETABLE       CSV file with name of genome and ranges for each RNAME
  VIEWS             CSV file with genome, rname and region
  RANGE             Range in comma/dash separated value for the experiments to execute
  LOGFILE           Output of calling the tools_perf run on a set of files
  FILE              Custom rootfile for the provided genome
  TTREEPERFSTATS    File with TTreePerfStats

Options:
  -h --help
  -n NUMBER             Amount of records to generate
  -N                    Avoid compiling code
  -o, --out OUTFILE     File to save/append values, defaults to stdin
  -p, --path path       Additional paths to look for bam/root files
  --macro MACRO         Custom ramview macro to crossvalidate
  --no-split            Reduce Splitlevel for banches
  -c, --compression ALG Compression algorithm of choice
  -s, --stats           Print TTreeStats to file
  --cache               Enable TTreeCache
"""
import os
import random
import sys
from docopt import docopt
import pandas as pd
import subprocess
from datetime import datetime


def rangestr2list(s):
    return sum(((list(range(*[int(j) + k for k, j in enumerate(i.split('-'))]))
                if '-' in i else [int(i)]) for i in s.split(',')), [])


def clear_buffer_cache():
    code = subprocess.call(['sudo', 'sh', '-c', "echo 3 > /proc/sys/vm/drop_caches"])
    if code == 0:
        return
    else:
        print(code)
        sys.exit(1)


def sudo_reauth():
    subprocess.call("while true; do sudo -n true; sleep 60; kill -0 "$$" || exit; done 2>/dev/null &", shell=True)


def find_file_in_paths(file, paths):
    if not os.path.isfile(file):
        for path in paths:
            real_file = os.path.join(path, file)
            if os.path.isfile(real_file):
                return real_file
        else:
            print("Could not find {0}".format(file))
            sys.exit(1)
    else:
        return file


def lauch_and_save_output(cmd, outfile):
    with open(outfile, 'w') as f:
        processes.append(subprocess.Popen(cmd, stdout=f))


def manual_check()cmd:
    print(" ".join(cmd))
    print("\nIs this the command you want yo issue [y/N]\n")
    while(True):
        choice = input().lower()

        if choice == 'y':
            break
        elif choice == 'n':
            sys.exit(1)
        else:
            print("Invalid Choice [y/N]")


def print_timestamp():
    print("[{0}] ".format(datetime.now().strftime('%Y-%m-%d %H:%M:%S')), end='')


def wrap_root_cmd(cmd):
    return ["root", "-q", "-l", "-b"] + cmd


def wrap_time_cmd(cmd, time_logfile):
    pass


def wrap_io_cmd(cmd, io_logfile):
    pass


if __name__ == '__main__':
    arguments = docopt(__doc__)
    compilation_flag = '+' if not arguments['-N'] else ''

    processes = []

    if arguments['generate']:

        N = int(arguments['-n']) if arguments['-n'] else 10

        outfile = arguments['--out'] if arguments['--out'] else None

        offset = 0
        if outfile is not None:
            if not os.path.isfile(outfile):
                with open(outfile, 'w') as f:
                    print(",rname,start,end", file=f)
            else:
                df = pd.read_csv(outfile)
                offset = df.index.max() + 1

        if outfile is not None:
            outfile = open(outfile, 'a')
        else:
            outfile = sys.stdout

        table = pd.read_csv(arguments['GENOMETABLE'])
        table = table[~table['RNAME'].str.startswith('GL')]
        table = table[~table['RNAME'].str.startswith('*')]

        for i in range(N):
            row = table.ix[random.choice(table.index)]
            rname = row['RNAME']
            a, b = random.randint(row['beginPOS'], row['endPOS']), random.randint(row['beginPOS'], row['endPOS'])
            a, b = min(a, b), max(a, b)
            print("{0},{1},{2},{3}".format(i+offset, rname, a, b), file=outfile)

    else:
        outfolder = arguments['--out'] if arguments['--out'] else '.'
        os.makedirs(outfolder, exist_ok=True)

        if arguments['convert']:
            samtoram_macro = arguments['--macro'] if arguments['--macro'] else "samtoram.C"

            samfile = arguments['SAMFILE']
            rootfile = arguments['ROOTFILE']

            split = "false" if arguments['--no-split'] else "true"
            compression = arguments['--compression'] if arguments['--compression'] else "ROOT::kLZMA"

            logfile = "samtoram_{0}_{1}_{2}".format(os.path.basename(samfile).split('.sam')[0], compression, 'split' if split == 'true' else 'nosplit')
            logfile = os.path.join(outfolder, logfile)

            samtoram_cmd = [
                "/usr/bin/time", "-v", "--output={0}.perf".format(logfile),
                "root", "-q", "-l", "-b", "samtoram.C{4}(\"{0}\", \"{1}\", {2}, {3})".format(samfile, rootfile, split, compression,
                                                                                             compilation_flag)
            ]

            manual_check(samtoram_cmd)

            print_timestamp()
            print("Executing samtoram from {0} to {1}".format(samfile, rootfile))
            lauch_and_save_output(samtoram_cmd, logfile + ".log")

        elif arguments['run']:
            df = pd.read_csv(arguments['VIEWS'])

            if arguments['RANGE']:
                df = df.ix[rangestr2list(arguments['RANGE'])]

            arguments['--path'] = ['.'] + arguments['--path']

            sudo_reauth()

            clear_buffer_cache()

            for index, row in df.iterrows():

                region = "{0}:{1}-{2}".format(row['rname'], row['start'], row['end'])

                if arguments['samview']:

                    bamfile = find_file_in_paths(arguments['FILE'], arguments['--path'])

                    logfile = "samtools__{0}__{1}".format(os.path.basename(bamfile), region)
                    logfile = os.path.join(outfolder, logfile)

                    samtools_cmd = [
                        "/usr/bin/time", "-v", "--output={0}.perf".format(logfile),
                        "samtools", "view", bamfile, region
                    ]

                    if arguments['--io']:
                        samtools_cmd = ['strace', '-o', '{0}.io'.format(logfile), '-TC'] + samtools_cmd

                    print_timestamp()
                    print("Executing samtools view on {0} {1}".format(bamfile, region))
                    with open(logfile + ".log", 'w') as f:
                        processes.append(subprocess.Popen(samtools_cmd, stdout=f))

                elif arguments['ramview']:

                    rootfile = find_file_in_paths(arguments['FILE'], arguments['--path'])

                    logfile = "ramtools__{0}__{1}".format(os.path.basename(rootfile), region)
                    logfile = os.path.join(outfolder, logfile)

                    ramview_macro = arguments['--macro'] if arguments['--macro'] else "ramview.C"

                    ramtools_cmd = [
                        "/usr/bin/time", "-v", "--output={0}.perf".format(logfile),
                        "root", "-q", "-l", "-b"
                    ]

                    cache = "false" if arguments['--no-split'] else "true"

                    if not arguments['--stats']:
                        ramtools_cmd += ["{2}{4}(\"{0}\", \"{1}\", {3})".format(rootfile, region, ramview_macro, cache
                                                                                compilation_flag)]
                    else:
                        ttreeperffile = logfile + '.root'
                        ramtools_cmd += ["{2}{4}(\"{0}\", \"{1}\", {3}, true, \"{5}\")".format(rootfile, region, ramview_macro, cache
                                                                                               compilation_flag, ttreeperffile)]

                    if arguments['--io']:
                        ramtools_cmd = ['strace', '-o', '{0}.io'.format(logfile), '-TC'] + ramtools_cmd

                    print_timestamp()
                    print("Executing ramtools view on {0} {1}".format(rootfile, region))
                    lauch_and_save_output(ramtools_cmd, logfile + '.log')

                if not arguments['-P']:
                    exit_codes = [p.wait() for p in processes]
                    clear_buffer_cache()

            exit_codes = [p.wait() for p in processes]

        elif arguments['parsetreestats']:
            for file in arguments['TTREEPERFSTATS']:
                textfile = os.path.splitext(file)[0] + ".treeperf"
                imagefile = os.path.splitext(file)[0] + ".png"

                parsetreestats_cmd = [
                    "root", "-q", "-l", "-b", 'parsetreestats.C+("{0}", true, "{1}")'.format(file, imagefile)
                ]

                print_timestamp()
                print("Executing parsetreestats on {0}".format(file))
                lauch_and_save_output(textfile, parsetreestats_cmd)
