#!/usr/bin/env python3

import argparse
import os.path
import time
import subprocess
import datetime

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', dest='mapping file', required=True,
                        help='Specify the data-name-mapping file.')
    parser.add_argument('-o', dest='output folder', required=True,
                        help='Specifying the output folder.')
    parser.add_argument('-t', dest='timeout', type=int, default=30,
                        help='number of seconds that waits last tasks finished')
    parser.add_argument('-sl', dest='start line', type=int, default=1,
                        help='an integer for the start line number')
    parser.add_argument('-el', dest='end line', type=int, default=-1,
                        help='an integer for the end line number')
    # need a parameter to specify the start line and the end line in the mapping file (default is 1st and the last one), so we can start from that line, not from scratch

    args = vars(parser.parse_args())

    mappingFile = args['mapping file']
    outputDir = args['output folder']
    startLine = args['start line']
    endLine = args['end line']
    timeout = args['timeout']

    # need to parse the input
    with open(mappingFile) as f:
        count = 0
        processed = 0
        curLine = startLine
        for line in f:
            count += 1
            if count < startLine:
                 #print (count)
                 continue
            if endLine != -1 and count >= endLine:
                 break
            #if processed > 1 and processed % 30 == 0:
            #    time.sleep(timeout) #usually a 1.2GB file can be processed in 4'3'', some file contains 2G data
            while True:
                psOutput = subprocess.check_output(['ps', '-eo' ,'pid,args'], universal_newlines=True)
                numOfProcess = psOutput.count('tlvFileGenerator')
                #print (numOfProcess)
                #time.sleep(10)
                if (numOfProcess > 30): # for now, hard code this as 30 processes
                    time.sleep(timeout)
                else:
                    break
            (srcFile, ndnName) = line.split()
            fileName = srcFile[(srcFile.rfind('/')+1):(srcFile.rfind('.'))] + '.tlv'
            targetFile = os.path.join(outputDir, fileName)
            print (datetime.datetime.now().time(), curLine, srcFile, ndnName)
            sp = subprocess.Popen(['/usr/local/bin/writeNdnFile', '-b', '8000', '-u', ndnName, srcFile, targetFile])
            curLine += 1
            processed += 1
