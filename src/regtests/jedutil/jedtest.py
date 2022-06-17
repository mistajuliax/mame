import os
import sys
import shutil
import filecmp
import subprocess

VERBOSE = False


class JedTest:
    """
    A class containing the information necessary to run a test.
    """
    def __init__(self):
        self.name = ""
        self.jedFile = ""
        self.baselineFile = ""
        self.outputFile = ""
    
    def __repr__(self):
        return "Name: %s\nJedFile: %s\nBaselineFile: %s\nOutputFile: %s" % (self.name, 
                                                                            self.jedFile, 
                                                                            self.baselineFile, 
                                                                            self.outputFile)


def findJedTests(jedPath, baselinePath, outputPath):
    """
    Given a path to some jed files, returns a list of JedTest objects.
    """
    jedTestList = []
    for (path, dirs, files) in os.walk(jedPath):
        if '.svn' in path:
            continue

        for filename in files:
            jedPathFull = os.path.join(path, filename)
            if os.path.splitext(jedPathFull)[1] != '.jed':
                continue
            palName = os.path.splitext(os.path.basename(filename))[0]
            subpathName = path[len(jedPath):].strip(os.sep)

            test = JedTest()
            test.name = subpathName
            test.jedFile = jedPathFull
            test.baselineFile = os.path.join(baselinePath, subpathName, f"{palName}.txt")
            test.outputFile = os.path.join(outputPath, subpathName, f"{palName}.txt")
            jedTestList.append(test)

    return jedTestList


def runViewJedTests(tests, jedUtilApp):
    """
    Given a list of JedTests and a command line program, run them & save the
    results to the files specified in the JedTest objects.
    """
    for test in tests:
        command = [jedUtilApp, "-view", test.jedFile, test.name]
        if VERBOSE:
            print(f"Viewing the JED file: {test.jedFile}")
            print(f'Command: {" ".join(command)}')

        process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        (stdout,stderr) = process.communicate()

        if stderr:
            print(
                f"Error: JED test named {test.name} failed during viewing ("
                + stderr.decode('latin-1').strip()
                + ")."
            )


        with open(test.outputFile, "wb") as fp:
            fp.write(stdout)
    

# MAIN
def main():
    # Some path initializations
    currentDirectory = os.path.dirname(os.path.realpath(__file__))
    jedsPath = os.path.join(currentDirectory, 'jeds')
    baselinePath = os.path.join(currentDirectory, "baseline")
    outputPath = os.path.join(currentDirectory, "output")
    if os.name == 'nt':
        jedUtilApp = os.path.normpath(os.path.join(currentDirectory, "..", "..", "..", "jedutil.exe"))
    else:		
        jedUtilApp = os.path.normpath(os.path.join(currentDirectory, "..", "..", "..", "jedutil"))

    if VERBOSE:
        print(f"JED Path:      {jedsPath}")
        print(f"Baseline Path: {baselinePath}")
        print(f"Output Path:   {outputPath}")
        print(f"jedutil App:   {jedUtilApp}")
        print('')


    # Insure everything exists
    if (not os.path.exists(currentDirectory) or
        not os.path.exists(jedsPath) or
        not os.path.exists(baselinePath) or
        not os.path.exists(jedUtilApp)):
        print(f"One of the above paths does not exist.  Aborting. {jedUtilApp}")
        return 3


    # Gather the tests
    tests = findJedTests(jedsPath, baselinePath, outputPath)
    if not len(tests):
        print("No tests found!")
        return 2


    # Setup the output paths
    if VERBOSE:
        print("Cleaning the output directory")
        print('')
    if os.path.exists(outputPath):
        shutil.rmtree(outputPath)
    os.makedirs(outputPath)
    for test in tests:
        testOutputPath = os.path.dirname(test.outputFile)
        if not os.path.exists(testOutputPath):
            os.makedirs(testOutputPath)


    # Run the commands to create the sample output
    runViewJedTests(tests, jedUtilApp)


    # Insure there are no errors
    success = True
    for test in tests:
        if VERBOSE:
            print(
                f"Diffing the output from viewing the JED file: {os.path.basename(test.jedFile)}"
            )

        if not filecmp.cmp(test.outputFile, test.baselineFile):
            success = False
            print(f"Test {os.path.basename(test.jedFile)} failed")


    # Report
    if success:
        print("All tests ran successfully.")
        return 0


    return 1


# Main stub - returns error code properly
if __name__ == "__main__":
    sys.exit(main())
