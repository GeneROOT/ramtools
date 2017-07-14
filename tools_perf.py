"""Usage: tools_perf.py generate [-n NUMBER] [--rname RNAMES]
                                 [--region MIN MAX] [--out OUTFILE] GENOME...
          tools_perf.py run FILE [RANGE] [--out FOLDER] [--path PATH]...
          tools_perf.py parse [--out OUTFILE] LOGFILE...

Preprocessing and postprocessing for evaluating the performance of ramtools functions

Arguments:
  GENOME      Identifier of the genome
  FILE        CSV file with genome, rname and region
  RANGE       Range in comma/dash separated value for the experiments to execute
  LOGFILE     Output of calling the tools_perf run on a set of files

Options:
  -h --help
  -n NUMBER             Amount of records to generate
  -c, --rname RNAMES    Range of chromosomes in comma separated format
  -r, --region MIN MAX  Values to consider for the view function generation
  -o, --out OUTFILE     File to save/append values, defaults to stdin
  -p, --path path       Additional paths to look for bam/root files
"""
import os
import random
import sys
from docopt import docopt
import pandas as pd
import subprocess
from datetime import datetime

def rangestr2list(s):
    return sum(((list(range(*[int(j) + k for k,j in enumerate(i.split('-'))]))
         if '-' in i else [int(i)]) for i in s.split(',')), [])


if __name__ == '__main__':
    arguments = docopt(__doc__)
    
    # print(arguments)
    if arguments['generate']:

        N = int(arguments['-n']) if arguments['-n'] else 10

        outfile = arguments['--out'] if arguments['--out'] else None
        
        rnames = arguments['--rname'].split(',') if arguments['--rname'] else list(range(46))

        region_min = int(arguments['--region']) if arguments['--region'] else 1
        region_max = int(arguments['MAX']) if arguments['MAX'] else int(1e7)

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

        for i in range(N):
            genome = random.choice(arguments['GENOME'])
            rname = random.choice(rnames)
            a, b = random.randint(region_min, region_max), random.randint(region_min, region_max)
            a, b = min(a, b), max(a, b)
            print("{0},{1},{2},{3},{4}".format(i+offset, genome, rname, a, b), file=outfile)

    elif arguments['run']:
        df = pd.read_csv(arguments['FILE'])
        
        if arguments['RANGE']:
            df = df.ix[rangestr2list(arguments['RANGE'])]

        arguments['--path'] = ['.'] + arguments['--path']

        outfolder = arguments['--out'] if arguments['--out'] else '.'
        os.makedirs(outfolder, exist_ok=True)

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


            logfile = "samtools_{0}_{1}".format(row['genome'], region)
            logfile = os.path.join(outfolder, logfile)

            samtools_cmd = [
            "/usr/bin/time", "-v", "--output={0}.perf".format(logfile),
            "samtools", "view", bamfile, region
            ]

            print("[{2}] Executing samtools view on {0}.bam {1}".format(rootfile, region, datetime.now().strftime('%Y-%m-%d %H:%M:%S')))
            with open(logfile + ".log" , 'w') as f:
                subprocess.call(samtools_cmd, stdout=f)


            logfile = "ramtools_{0}_{1}".format(row['genome'], region)
            logfile = os.path.join(outfolder, logfile)

            ramtools_cmd = [
            "/usr/bin/time", "-v", "--output={0}.perf".format(logfile),
            "root", "-q", "-l", "-b", "ramview.C(\"{0}\", \"{1}\")".format(rootfile, region)
            ]

            print("[{2}] Executing ramtools view on {0}.root {1}".format(rootfile, region, datetime.now().strftime('%Y-%m-%d %H:%M:%S')))
            with open(logfile + ".log", 'w') as f:
                subprocess.call(ramtools_cmd, stdout=f)
