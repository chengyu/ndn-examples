1 Compile and install:
=====================

  ./waf configure
  ./waf
  ./waf install


2. Tools for loading data into ndn-repo-ng
==========================================

  writeNdnFile reads raw data and generates tlv file as output.

  tlvFilesBatchGen.py generates multiple tlv files for a given list of raw data files, the list should also contain the assigned NDN names (refer to scripts/test-mapping.txt for examples)

  tlvFilesLoader.py reads the tlv files in a folder and inject them into the ndn-repo-ng


3. Usage:
=========

  1. start repo-ng

    A. Edit the repo-ng.conf (/usr/local/etc/ndn/repo-ng.conf), configure the prefix for data;
    B. Uncomment out the tcp_bulk_insert; 

  2. Generate tlv files

    python tlvFilesBatchGen.py -f <list_for_file_and_names> -o <output_dir>

    example: python tlvFilesBatchGen.py -f test-mapping.txt -o output

  3. Load tlv files into repo-ng

    python tlvFilesLoader.py -f <tlv_files_dir>

    example: python tlvFilesLoader.py -f output
