#!/usr/bin/env bash
# Usage: ramview_prof <file.sam>

# set -x

samfile=$1
file=${samfile%.*}
region=$2

# samtoram
ramfile="$file.root"

if [ ! -f $ramfile ]; then
    echo "Root file not found! Generating..."
    /usr/bin/time -v --output="samtoram_$file.perf" root -q -l "samtoram.C(\"$samfile\", \"$ramfile\")" > "samtoram_$file.log"
fi

# ramtools
/usr/bin/time -v --output="ramtools_$file_$region.perf" root -q -l "ramview.C(\"$ramfile\", \"$region\")" > "ramtools_$file_$region.log"

mkdir -p output
mv *.perf output
mv *.log output
