/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2013-2015 Regents of the University of California.
 *               2015      Colorado State University.
 *
 * This file is part of ndn-cxx library (NDN C++ library with eXperimental eXtensions).
 *
 * ndn-cxx library is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ndn-cxx library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received copies of the GNU General Public License and GNU Lesser
 * General Public License along with ndn-cxx, e.g., in COPYING.md file.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 */

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/validator.hpp>

#include <ndn-cxx/encoding/block.hpp>
// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions
namespace examples {

class Producer : noncopyable
{
public:
  void
  run()
  {
    m_face.setInterestFilter("/root/site1",
                             bind(&Producer::onInterest, this, _1, _2),
                             RegisterPrefixSuccessCallback(),
                             bind(&Producer::onRegisterFailed, this, _1, _2));
    m_face.processEvents();
  }

private:
  //void
  //HexPrinter(const Block& block, const std::string& indent = "")
  //{
  //  std::cout << indent;
  //  for (Buffer::const_iterator i = block.begin (); i != block.value_begin(); ++i)
  //    {
  //      std::cout << "0x";
  //      std::cout << std::noshowbase << std::hex << std::setw(2) <<
  //        std::setfill('0') << static_cast<int>(*i);
  //      std::cout << ", ";
  //    }
  //  std::cout << "\n";
  //
  //  if (block.elements_size() == 0 && block.value_size() > 0)
  //    {
  //      std::cout << indent << "    ";
  //      for (Buffer::const_iterator i = block.value_begin (); i != block.value_end(); ++i)
  //        {
  //          std::cout << "0x";
  //          std::cout << std::noshowbase << std::hex << std::setw(2) <<
  //            std::setfill('0') << static_cast<int>(*i);
  //          std::cout << ", ";
  //        }
  //      std::cout << "\n";
  //    }
  //  else
  //    {
  //      for (Block::element_const_iterator i = block.elements_begin();
  //           i != block.elements_end();
  //           ++i)
  //        {
  //          HexPrinter(*i, indent+"    ");
  //        }
  //    }
  //}

  void
  onInterest(const InterestFilter& filter, const Interest& interest)
  {
    std::cout << "<< I: " << interest << std::endl;

    // Create new name, based on Interest's name
    Name dataName(interest.getName());
    dataName
      .append("testApp") // add "testApp" component to Interest name
      .appendVersion();  // add "version" component (current UNIX timestamp in milliseconds)

    static const std::string content = "HELLO";

    // Create Data packet
    shared_ptr<Data> data = make_shared<Data>();
    data->setName(dataName);
    data->setFreshnessPeriod(time::seconds(10));
    data->setContent(reinterpret_cast<const uint8_t*>(content.c_str()), content.size());

    // Sign Data packet with default identity
    // m_keyChain.sign(*data);
    Name producerId("/root/site1"); ///test/site2/user2
    m_keyChain.signByIdentity(*data, producerId);
    // m_keyChain.sign(data, <identityName>);
    // m_keyChain.sign(data, <certificate>);

    // Return Data packet to the requester
    //std::cout << ">> D: " << *data << std::endl;
    //std::cout << "keyLocator : " << data->getSignature().getKeyLocator().getName() << std::endl;
    //Name defaultKeyName = m_keyChain.getDefaultKeyNameForIdentity(producerId);
    //std::cout << "key Name : " << defaultKeyName << std::endl;

    //shared_ptr<PublicKey> defaultPKey = m_keyChain.getPublicKey(defaultKeyName);

    ////verify the signature
    //if (ndn::Validator::verifySignature(*data, data->getSignature(), *defaultPKey)) {
    //  std::cout << "signature OK; " << std::endl;
    //  //std::cout << "Key " << std::endl;
    //  //std::cout << *defaultPKey<< std::endl;
    //  //std::cout << "Signature " << std::endl;
    //  //HexPrinter(data->getSignature().getValue());
    //}
    //else
    //  std::cout << "signature verification failed" << std::endl;


    m_face.put(*data);
  }


  void
  onRegisterFailed(const Name& prefix, const std::string& reason)
  {
    std::cerr << "ERROR: Failed to register prefix \""
              << prefix << "\" in local hub's daemon (" << reason << ")"
              << std::endl;
    m_face.shutdown();
  }

private:
  Face m_face;
  KeyChain m_keyChain;
};

} // namespace examples
} // namespace ndn

int
main(int argc, char** argv)
{
  ndn::examples::Producer producer;
  try {
    producer.run();
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return 0;
}
