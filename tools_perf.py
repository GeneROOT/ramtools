"""Usage: tools_perf.py generate [-n NUMBER] [--out OUTFILE] GENOMETABLE...
          tools_perf.py run samview FILE [RANGE] [-P] [--out FOLDER] [--path PATH]...
          tools_perf.py run ramview FILE [RANGE] [-P] [--out FOLDER] [--macro MACRO] [--path PATH]...
          tools_perf.py parse [--out OUTFILE] LOGFILE...

Preprocessing and postprocessing for evaluating the performance of ramtools functions

Arguments:
  GENOMETABLE CSV file with name of genome and ranges for each RNAME
  FILE        CSV file with genome, rname and region
  RANGE       Range in comma/dash separated value for the experiments to execute
  LOGFILE     Output of calling the tools_perf run on a set of files

Options:
  -h --help
  -n NUMBER             Amount of records to generate
  -o, --out OUTFILE     File to save/append values, defaults to stdin
  -p, --path path       Additional paths to look for bam/root files
  --macro MACRO         Custom ramview macro to crossvalidate
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

    print(arguments)
    if arguments['generate']:

        N = int(arguments['-n']) if arguments['-n'] else 10

        outfile = arguments['--out'] if arguments['--out'] else None

        offset = 0
        if outfile is not None:
            if not os.path.isfile(outfile):
                with open(outfile, 'w') as f:
                    print(",genome,rname,start,end", file=f)
            else:
                df = pd.read_csv(outfile)
                offset = df.index.max() + 1

        if outfile is not None:
            outfile = open(outfile, 'a')
        else:
            outfile = sys.stdout

        tables = {table[:-len('.root.idx')]: pd.read_csv(table) for table in arguments['GENOMETABLE']}

        for i in range(N):
            genome = random.choice(list(tables.keys()))
            table = tables[genome]
            table = table[~table['RNAME'].str.startswith('GL')]
            row = table.ix[random.choice(table.index)]
            rname = row['RNAME']
            a, b = random.randint(row['beginPOS'], row['endPOS']), random.randint(row['beginPOS'], row['endPOS'])
            a, b = min(a, b), max(a, b)
            print("{0},{1},{2},{3},{4}".format(i+offset, genome, rname, a, b), file=outfile)

    elif arguments['run']:
        df = pd.read_csv(arguments['FILE'])

        if arguments['RANGE']:
            df = df.ix[rangestr2list(arguments['RANGE'])]

        arguments['--path'] = ['.'] + arguments['--path']

        outfolder = arguments['--out'] if arguments['--out'] else '.'
        os.makedirs(outfolder, exist_ok=True)

        processes = []

        for index, row in df.iterrows():

            bamfile = "{0}.bam".format(row['genome'])
            rootfile = "{0}.root".format(row['genome'])

            if not os.path.isfile(bamfile):
                for path in arguments['--path']:
                    real_bamfile = os.path.join(path, bamfile)
                    if os.path.isfile(real_bamfile):
                        bamfile = real_bamfile
                        break
                else:
                    print("Could not find {0}".format(bamfile))
                    sys.exit(1)

            if not os.path.isfile(rootfile):
                for path in arguments['--path']:
                    real_ramfile = os.path.join(path, rootfile)
                    if os.path.isfile(real_ramfile):
                        rootfile = real_ramfile
                        break
                else:
                    print("Could not find {0}".format(rootfile))
                    sys.exit(1)

            region = "{0}:{1}-{2}".format(row['rname'], row['start'], row['end'])

            if arguments['samview']:

                logfile = "samtools_{0}_{1}".format(row['genome'], region)
                logfile = os.path.join(outfolder, logfile)

                samtools_cmd = [
                    "/usr/bin/time", "-v", "--output={0}.perf".format(logfile),
                    "samtools", "view", bamfile, region
                ]

                print("[{2}] Executing samtools view on {0} {1}".format(bamfile, region, datetime.now().strftime('%Y-%m-%d %H:%M:%S')))
                with open(logfile + ".log", 'w') as f:
                    processes.append(subprocess.Popen(samtools_cmd, stdout=f))

            elif arguments['ramview']:

                logfile = "ramtools_{0}_{1}".format(row['genome'], region)
                logfile = os.path.join(outfolder, logfile)

                ramview_macro = arguments['--macro'] if arguments['--macro'] else "ramview.C"

                ramtools_cmd = [
                    "/usr/bin/time", "-v", "--output={0}.perf".format(logfile),
                    "root", "-q", "-l", "-b", "{2}(\"{0}\", \"{1}\")".format(rootfile, region, ramview_macro)
                ]

                print("[{2}] Executing ramtools view on {0} {1}".format(rootfile, region, datetime.now().strftime('%Y-%m-%d %H:%M:%S')))
                with open(logfile + ".log", 'w') as f:
                    processes.append(subprocess.Popen(ramtools_cmd, stdout=f))

            if not arguments['-P']:
                exit_codes = [p.wait() for p in processes]

        exit_codes = [p.wait() for p in processes]
