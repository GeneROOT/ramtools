"""Usage: tools_perf.py generate [-n NUMBER] [--rname RNAMES]
                                 [--region MIN MAX] [--out OUTFILE] GENOME...
          tools_perf.py run FILE [RANGE] [--path PATH]...
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
  -c, --rname RNAMES    Range of chromosomes in comma/dash separated format
  -r, --region MIN MAX  Values to consider for the view function generation
  -o, --out OUTFILE     File to save/append values, defaults to stdin
  -p, --path path       Additional paths to look for bam/root files
"""
import os
import random
import sys
from docopt import docopt
import pandas as pd

def rangestr2list(s):
    return sum(((list(range(*[int(j) + k for k,j in enumerate(i.split('-'))]))
         if '-' in i else [int(i)]) for i in s.split(',')), [])


if __name__ == '__main__':
    arguments = docopt(__doc__)
    
    # print(arguments)
    if arguments['generate']:

        N = int(arguments['-n']) if arguments['-n'] else 10

        outfile = arguments['--out'] if arguments['--out'] else None
        
        rnames = rangestr2list(arguments['--rname']) if arguments['--rname'] else list(range(46))

        region_min = int(arguments['--region']) if arguments['--region'] else 1
        region_max = int(arguments['MAX']) if arguments['MAX'] else int(1e7)

        offset = 0

        if outfile is not None:
            if not os.path.isfile(outfile):
                with open(outfile, 'w') as f:
                    print(",genome,rname,start,end" ,file=f)
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

