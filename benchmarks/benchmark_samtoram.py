import argparse
import os
import subprocess
import sys
import time
from pathlib import Path


def check_prereqs(sam: Path):
    base = Path("..")
    for f in ["samtoram.C", "ramrecord.C", "ramrecord.h", "utils.h"]:
        if not (base / f).exists():
            sys.exit(f"missing: {f}")

    if not sam.exists():
        sys.exit(f"missing: {sam}")

    if subprocess.run(["which", "root"], capture_output=True).returncode != 0:
        sys.exit("ROOT is not on PATH")


def count_sam_records(sam: Path) -> int:
    return sum(1 for l in sam.open() if not l.startswith("@"))


def run_conversion(sam: Path, out: Path):
    if out.exists():
        out.unlink()

    cmd = f'root -b -q \'../samtoram.C("{sam}","{out}")\''

    start = time.perf_counter()
    p = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    elapsed = time.perf_counter() - start

    return elapsed, p.returncode, p.stderr


def inspect_output(out: Path):
    cmd = f'root -l -b -q \'inspect_entries.C("{out}")\''
    p = subprocess.run(cmd, shell=True, capture_output=True, text=True)

    if p.returncode != 0:
        sys.exit("Failed to inspect ROOT file")

    try:
        entries = int(p.stdout.strip().split()[-1])
    except:
        sys.exit("Could not parse ROOT output")

    size = out.stat().st_size
    return entries, size


def human_size(b):
    for unit in ["B", "KB", "MB", "GB"]:
        if b < 1024:
            return f"{b:.2f} {unit}"
        b /= 1024
    return f"{b:.2f} TB"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--runs", type=int, default=1)
    args = parser.parse_args()

    sam = Path("../samexample.sam")
    out = Path("../output.root")

    check_prereqs(sam)

    sam_records = count_sam_records(sam)
    sam_size = sam.stat().st_size

    timings = []

    print(f"\nBenchmarking {sam} → {out}\n")

    for i in range(args.runs):
        print(f"run {i+1}/{args.runs} ... ", end="", flush=True)

        elapsed, rc, err = run_conversion(sam, out)

        if rc != 0:
            print("FAILED")
            print(err)
            sys.exit(1)

        timings.append(elapsed)
        print(f"{elapsed:.3f} s")

    entries, ram_size = inspect_output(out)

    best = min(timings)
    throughput = entries / best

    print("\n===== SAM → RAM Benchmark =====")
    print(f"SAM file: {sam}")
    print(f"Output file: {out}")
    print(f"SAM size: {human_size(sam_size)}")
    print(f"RAM size: {human_size(ram_size)}")
    print(f"SAM records: {sam_records}")
    print(f"TTree entries: {entries}")
    print(f"Best time: {best:.3f} s")
    print(f"Throughput: {throughput:,.0f} records/sec\n")


if __name__ == "__main__":
    main()
