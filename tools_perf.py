"""Usage: tools_perf.py generate [-n NUMBER] [--out OUTFILE] GENOMETABLE
          tools_perf.py convert [--no-split] [-c ALG] [-N] [--out OUTFILE] SAMFILE ROOTFILE
          tools_perf.py run samview FILE VIEWS [RANGE] [-P] [-N] [--out FOLDER]  [--path PATH]...
          tools_perf.py run ramview FILE VIEWS [RANGE] [-P] [-N] [--out FOLDER] [--stats] [--macro MACRO]  [--path PATH]...
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
            compression = arguments['--compression'] if arguments['--compression'] else "kLZMA"

            logfile = "samtoram_{0}_{1}_{2}".format(os.path.basename(samfile).split('.sam')[0], compression, 'split' if split == 'true' else 'nosplit')
            logfile = os.path.join(outfolder, logfile)

            samtoram_cmd = [
                "/usr/bin/time", "-v", "--output={0}.perf".format(logfile),
                "root", "-q", "-l", "-b", "samtoram.C{4}(\"{0}\", \"{1}\", {2}, \"{3}\")".format(samfile, rootfile, split, compression,
                                                                                                 compilation_flag)
            ]

            print(" ".join(samtoram_cmd))
            print("\nIs this the command you want yo issue [y/N]\n")
            while(True):
                choice = input().lower()

                if choice == 'y':
                    break
                elif choice == 'n':
                    sys.exit(1)
                else:
                    print("Invalid Choice [y/N]")

            print("[{2}] Executing samtoram from {0} to {1}".format(samfile, rootfile, datetime.now().strftime('%Y-%m-%d %H:%M:%S')))
            with open(logfile + ".log", 'w') as f:
                processes.append(subprocess.Popen(samtoram_cmd, stdout=f))

        elif arguments['parsetreestats']:
            for file in arguments['TTREEPERFSTATS']:
                textfile = os.path.splitext(file)[0] + ".treeperf"
                imagefile = os.path.splitext(file)[0] + ".png"

                parsetreestats_cmd = [
                    "root", "-q", "-l", "-b", 'parsetreestats.C+("{0}", true, "{1}")'.format(file, imagefile)
                ]

                print("[{1}] Executing parsetreestats on {0}".format(file, datetime.now().strftime('%Y-%m-%d %H:%M:%S')))
                with open(textfile, 'w') as f:
                    processes.append(subprocess.Popen(parsetreestats_cmd, stdout=f))

        elif arguments['run']:
            df = pd.read_csv(arguments['VIEWS'])

            if arguments['RANGE']:
                df = df.ix[rangestr2list(arguments['RANGE'])]

            arguments['--path'] = ['.'] + arguments['--path']

            for index, row in df.iterrows():

                region = "{0}:{1}-{2}".format(row['rname'], row['start'], row['end'])

                if arguments['samview']:

                    bamfile = arguments['FILE']

                    if not os.path.isfile(bamfile):
                        for path in arguments['--path']:
                            real_bamfile = os.path.join(path, bamfile)
                            if os.path.isfile(real_bamfile):
                                bamfile = real_bamfile
                                break
                        else:
                            print("Could not find {0}".format(bamfile))
                            sys.exit(1)

                    logfile = "samtools__{0}__{1}".format(os.path.basename(bamfile), region)
                    logfile = os.path.join(outfolder, logfile)

                    samtools_cmd = [
                        "/usr/bin/time", "-v", "--output={0}.perf".format(logfile),
                        "samtools", "view", bamfile, region
                    ]

                    print("[{2}] Executing samtools view on {0} {1}".format(bamfile, region, datetime.now().strftime('%Y-%m-%d %H:%M:%S')))
                    with open(logfile + ".log", 'w') as f:
                        processes.append(subprocess.Popen(samtools_cmd, stdout=f))

                elif arguments['ramview']:

                    rootfile = arguments['FILE']

                    if not os.path.isfile(rootfile):
                        for path in arguments['--path']:
                            real_ramfile = os.path.join(path, rootfile)
                            if os.path.isfile(real_ramfile):
                                rootfile = real_ramfile
                                break
                        else:
                            print("Could not find {0}".format(rootfile))
                            sys.exit(1)

                    logfile = "ramtools__{0}__{1}".format(os.path.basename(rootfile), region)
                    logfile = os.path.join(outfolder, logfile)

                    ramview_macro = arguments['--macro'] if arguments['--macro'] else "ramview.C"

                    ramtools_cmd = [
                        "/usr/bin/time", "-v", "--output={0}.perf".format(logfile),
                        "root", "-q", "-l", "-b"
                    ]

                    if not arguments['--stats']:
                        ramtools_cmd += ["{2}{3}(\"{0}\", \"{1}\")".format(rootfile, region, ramview_macro, compilation_flag)]
                    else:
                        ttreeperffile = logfile + '.root'
                        ramtools_cmd += ["{2}{3}(\"{0}\", \"{1}\", true, \"{4}\")".format(rootfile, region, ramview_macro, compilation_flag, ttreeperffile)]

                    print("[{2}] Executing ramtools view on {0} {1}".format(rootfile, region, datetime.now().strftime('%Y-%m-%d %H:%M:%S')))
                    with open(logfile + ".log", 'w') as f:
                        processes.append(subprocess.Popen(ramtools_cmd, stdout=f))

                if not arguments['-P']:
                    exit_codes = [p.wait() for p in processes]

            exit_codes = [p.wait() for p in processes]
