import subprocess
import sys
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parent
SAM_FILE = ROOT_DIR / "samexample.sam"
OUT_FILE = ROOT_DIR / "ramexample.root"


def run_conversion():
    macro = f'.x {ROOT_DIR}/samtoram.C("{SAM_FILE}","{OUT_FILE}")'
    result = subprocess.run(
        ["root", "-b", "-q", macro],
        capture_output=True,
        text=True,
        cwd=ROOT_DIR,
    )
    if result.returncode != 0:
        print("FAIL  conversion exited non-zero")
        print(result.stderr[-600:])
        sys.exit(1)


def check_output():
    import ROOT
    ROOT.gROOT.SetBatch(True)
    ROOT.gErrorIgnoreLevel = ROOT.kWarning

    if not OUT_FILE.exists():
        print("FAIL  output file not created")
        sys.exit(1)

    f = ROOT.TFile.Open(str(OUT_FILE), "READ")
    if not f or f.IsZombie():
        print("FAIL  cannot open ROOT file")
        sys.exit(1)

    tree = f.Get("RAM")
    if not tree:
        print("FAIL  TTree 'RAM' not found")
        f.Close()
        sys.exit(1)

    entries = int(tree.GetEntries())
    f.Close()
    return entries


def main():
    print("  converting samexample.sam ... ", end="", flush=True)
    run_conversion()
    print("done")

    print("  checking output ROOT file  ... ", end="", flush=True)
    entries = check_output()
    print("done")

    print(f"  TTree 'RAM' entries: {entries:,}")

    if entries <= 0:
        print("FAIL  tree has no entries")
        sys.exit(1)

    print("PASS")


if __name__ == "__main__":
    main()
