#!/usr/bin/env python3

import argparse
import os
import time
import subprocess
import datetime

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", dest="folder", required=True,
                        help="Specify the folder that contains tlv files.")
    parser.add_argument('-t', dest='timeout', type=int, default=20, 
                        help='number of seconds that waits last tasks finished')
    parser.add_argument("-v", default=False, dest="verbose", action="store_true",
                        help="Verbose mode.")

    args = vars(parser.parse_args())

    folder = args['folder']
    verbose = args['verbose']
    timeout = args['timeout']

    # traverse root directory, and list directories as dirs and files as files
    for root, dirs, files in os.walk(folder, topdown=True):
        #count = 0
        #processed = 0
        for name in files:
            absoluteFile = os.path.join(root, name)
            ##count += 1
            #while True:
            #    psOutput = subprocess.check_output(['ps', '-eo' ,'pid,args'], universal_newlines=True)
            #    numOfProcess = psOutput.count('nc localhost 7376')
            #    if (numOfProcess > 30): # for now, hard code this as 30 processes
            #        time.sleep(timeout)
            #    else:
            #        break
            print (datetime.datetime.now().time(), 'nc localhost 7376 <', absoluteFile)
            command = '/usr/bin/nc localhost 7376 < ' + absoluteFile
            output = subprocess.check_output(command, shell=True)
            #fd = open(absoluteFile, 'rb')
            #try:
            #    sp = subprocess.Popen(['/usr/bin/nc', 'localhost', '7376'], stdin=fd)
            #finally:
            #    fd.close()
