#!/usr/bin/env bash
# Usage: ramview_prof [-k] <file.sam> <region>

# set -x

if [ "$1" = "-k" ]; then
    samfile=$2
    region=$3
    kerberos=true
else
    samfile=$1
    region=$2
fi

file=${samfile%.*}

# samtoram
ramfile="$file.root"

if [ ! -f $ramfile ]; then
    echo "Root file not found! Generating..."

    if [ "$kerberos" = true ]; then
        k5reauth -p jjgonzal -k "$HOME/jjgonzal.keytab" -- \
        /usr/bin/time -v --output="samtoram_$file.perf" root -q -l "samtoram.C(\"$samfile\", \"$ramfile\")" > "samtoram_$file.log"
    else
        /usr/bin/time -v --output="samtoram_$file.perf" root -q -l "samtoram.C(\"$samfile\", \"$ramfile\")" > "samtoram_$file.log"
    fi
    
fi

# ramtools

if [ "$kerberos" = true ]; then
    k5reauth -p jjgonzal -k "$HOME/jjgonzal.keytab" -- \
    /usr/bin/time -v --output="ramtools_$file_$region.perf" root -q -l "ramview.C(\"$ramfile\", \"$region\")" > "ramtools_$file_$region.log"
else
    /usr/bin/time -v --output="ramtools_$file_$region.perf" root -q -l "ramview.C(\"$ramfile\", \"$region\")" > "ramtools_$file_$region.log"
fi



mkdir -p output
mv *.perf output
mv *.log output
