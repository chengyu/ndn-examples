/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014,  Regents of the University of California.
 *
 * This file is part of NDN repo-ng (Next generation of NDN repository).
 * See AUTHORS.md for complete list of repo-ng authors and contributors.
 *
 * repo-ng is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * repo-ng is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * repo-ng, e.g., in COPYING.md file.  if (not, see <http://www.gnu.org/licenses/>.
 */

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/scheduler.hpp>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <stdint.h>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <boost/iostreams/operations.hpp>
#include <boost/iostreams/read.hpp>

using namespace ndn::time;

using std::shared_ptr;
using std::make_shared;
using std::bind;
using std::placeholders::_1;
using std::placeholders::_2;

//static const uint64_t DEFAULT_BLOCK_SIZE = 1000;
static const uint64_t DEFAULT_INTEREST_LIFETIME = 4000;
static const uint64_t DEFAULT_FRESHNESS_PERIOD = 10000;
static const uint64_t DEFAULT_CHECK_PERIOD = 1000;
//static const size_t PRE_SIGN_DATA_COUNT = 11;

class NdnPutFile : ndn::noncopyable
{
public:
  class Error : public std::runtime_error
  {
  public:
    explicit
    Error(const std::string& what)
      : std::runtime_error(what)
    {
    }
  };

  NdnPutFile()
    : isUnversioned(false)
    , freshnessPeriod(DEFAULT_FRESHNESS_PERIOD)
    , interestLifetime(DEFAULT_INTEREST_LIFETIME)
    , blockSize(1000)
    , hasTimeout(false)
    , timeout(0)
    , insertStream(0)
    , isVerbose(false)

    , m_timestampVersion(toUnixTimestamp(system_clock::now()).count())
    , m_checkPeriod(DEFAULT_CHECK_PERIOD)
    , m_currentSegmentNo(0)
    , m_isFinished(false)
  {
  }

  void
  run();

private:
  void
  prepareNextData(uint64_t referenceSegmentNo);

  void
  signData(ndn::Data& data);

public:
  bool isUnversioned;
  std::string identityForData;
  std::string identityForCommand;
  milliseconds freshnessPeriod;
  milliseconds interestLifetime;
  uint16_t blockSize;
  bool hasTimeout;
  milliseconds timeout;
  ndn::Name ndnName;
  std::istream* insertStream;
  std::ostream* outputStream;
  bool isVerbose;

private:
  ndn::KeyChain m_keyChain;
  uint64_t m_timestampVersion;
  milliseconds m_checkPeriod;
  size_t m_currentSegmentNo;
  bool m_isFinished;
  ndn::Name m_dataPrefix;

  typedef std::map<uint64_t, shared_ptr<ndn::Data> > DataContainer;
  DataContainer m_data;
};

void
NdnPutFile::run()
{
  m_dataPrefix = ndnName;
  if (!isUnversioned)
    m_dataPrefix.appendVersion(m_timestampVersion);

  uint8_t *buffer = new uint8_t[blockSize];
  size_t i = 0;
  while (!m_isFinished) {
    std::fill(buffer, buffer + blockSize, 0);
    std::streamsize readSize =
      boost::iostreams::read(*insertStream, reinterpret_cast<char*>(buffer), blockSize);

    if (readSize <= 0) {
      throw Error("Error reading from the input stream");
    }

    shared_ptr<ndn::Data> data =
      make_shared<ndn::Data>(ndn::Name(m_dataPrefix)
                                       .appendSegment(i));

    if (insertStream->peek() == std::istream::traits_type::eof()) {
      data->setFinalBlockId(ndn::name::Component::fromSegment(i));
      m_isFinished = true;
    }

    data->setContent(buffer, readSize);
    data->setFreshnessPeriod(freshnessPeriod);
    signData(*data);

    const ndn::Block& payload = data->wireEncode();
    outputStream->write(reinterpret_cast<const char*>(payload.wire()), payload.size());

    i++;
  }
  delete[] buffer;
}

void
NdnPutFile::signData(ndn::Data& data)
{
  m_keyChain.sign(data);
}


static void
usage()
{
  fprintf(stderr,
          "ndnputfile [-u] [-d]"
          "  [-x freshness] [-l lifetime] [-w timeout] ndn-name infilename outfilename\n"
          "\n"
          " Write a file into a repo.\n"
          "  -u: unversioned: do not add a version component\n"
          "  -x: FreshnessPeriod in milliseconds\n"
          "  -l: InterestLifetime in milliseconds for each command\n"
          "  -w: timeout in milliseconds for whole process (default unlimited)\n"
          "  -b: block size \n"
          "  -v: be verbose\n"
          "  ndn-name: NDN Name prefix for written Data\n"
          "  infilename: input file name; \"-\" reads from stdin\n"
          "  outfilename: output file name\n"
          );
  exit(1);
}

int
main(int argc, char** argv)
{
  NdnPutFile ndnPutFile;
  int opt;

  while ((opt = getopt(argc, argv, "uDi:I:x:l:w:b:vh")) != -1) {
    switch (opt) {
    case 'u':
      ndnPutFile.isUnversioned = true;
      break;
    case 'x':
      try {
        ndnPutFile.freshnessPeriod = milliseconds(boost::lexical_cast<uint64_t>(optarg));
      }
      catch (boost::bad_lexical_cast&) {
        std::cerr << "-x option should be an integer.";
        return 1;
      }
      break;
    case 'l':
      try {
        ndnPutFile.interestLifetime = milliseconds(boost::lexical_cast<uint64_t>(optarg));
      }
      catch (boost::bad_lexical_cast&) {
        std::cerr << "-l option should be an integer.";
        return 1;
      }
      break;
    case 'w':
      ndnPutFile.hasTimeout = true;
      try {
        ndnPutFile.timeout = milliseconds(boost::lexical_cast<uint64_t>(optarg));
      }
      catch (boost::bad_lexical_cast&) {
        std::cerr << "-w option should be an integer.";
        return 1;
      }
      break;
    case 'b':
      try {
        ndnPutFile.blockSize = boost::lexical_cast<uint16_t>(optarg);
      }
      catch (boost::bad_lexical_cast&) {
        std::cerr << "-b option should be an uint16_t.";
        return 1;
      }
      break;
    case 'v':
      ndnPutFile.isVerbose = true;
      break;
    case 'h':
      usage();
      break;
    default:
      break;
    }
  }

  argc -= optind;
  argv += optind;

  if (argc != 3)
    usage();

  ndnPutFile.ndnName = ndn::Name(argv[0]);
  std::ofstream outputFileStream(argv[2], std::ios::out | std::ios::binary);
  ndnPutFile.outputStream = &outputFileStream;
  if (strcmp(argv[1], "-") == 0) {
    ndnPutFile.insertStream = &std::cin;
    ndnPutFile.run();
  }
  else {
    std::ifstream inputFileStream(argv[1], std::ios::in | std::ios::binary);
    if (!inputFileStream.is_open()) {
      std::cerr << "ERROR: cannot open " << argv[1] << std::endl;
      return 1;
    }

    ndnPutFile.insertStream = &inputFileStream;
    ndnPutFile.run();
  }

  // ndnPutFile MUST NOT be used anymore because .insertStream is a dangling pointer

  return 0;
}

//int
//main(int argc, char** argv)
//{
//  try {
//    return repo::main(argc, argv);
//  }
//  catch (std::exception& e) {
//    std::cerr << "ERROR: " << e.what() << std::endl;
//    return 2;
//  }
//}
