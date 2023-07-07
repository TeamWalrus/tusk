from pathlib import Path
from shutil import copytree, rmtree, copyfileobj
from subprocess import check_output, Popen, PIPE, STDOUT, CalledProcessError
import os
import gzip

Import("env")

def gzipFile(file):
    with open(file, 'rb') as f_in:
        with gzip.open(file + '.gz', 'wb') as f_out:
            copyfileobj(f_in, f_out)
    os.remove(file)

def buildWeb():
    os.chdir("interface")
    print("Building interface with npm")
    try:
        env.Execute("npm install")
        env.Execute("npm run build")
        buildPath = Path("../interface/dist/")
        dataPath = Path("../data/")
        if dataPath.exists() and dataPath.is_dir():
            rmtree(dataPath)        
        print("Copying interface to data directory")
        copytree(buildPath, dataPath)
        for currentpath, folders, files in os.walk(dataPath):
            for file in files:
                gzipFile(os.path.join(currentpath, file))
    finally:
        os.chdir("..")

if (len(BUILD_TARGETS) == 0 or "upload" in BUILD_TARGETS):
    buildWeb()
else:
    print("Skipping build interface step for target(s): " + ", ".join(BUILD_TARGETS))