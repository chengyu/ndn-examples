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
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 *
 */

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/validator-config.hpp>
#include <ndn-cxx/util/io.hpp>
#include <boost/assert.hpp>

// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions
namespace examples {

class Consumer : noncopyable
{
public:
  void
  initilize()
  {
    m_face = make_shared<Face>();
    m_validator = make_shared<ValidatorConfig>(*m_face);
    m_validator->load("/Users/chengyu/Dropbox/cert/validator-config.conf");

   // m_validator->addDataVerificationRule(ndn::make_shared<ndn::SecRuleRelative>("^(<>*)$",
   //                                                                            "^([^<KEY>]*)<KEY>(<>*)<ksk-.*><ID-CERT>$",
   //                                                                            ">", "\\1", "\\1\\2", true));

    //ndn::shared_ptr<ndn::IdentityCertificate> anchor = ndn::io::load<ndn::IdentityCertificate>("/Users/chengyu/Dropbox/cert/site1-verify-test.cert");
    //ndn::shared_ptr<ndn::IdentityCertificate> anchor = ndn::io::load<ndn::IdentityCertificate>("/Users/chengyu/Dropbox/cert/root.ndncert");

    //if (static_cast<bool>(anchor))
    //  {
    //    BOOST_ASSERT(anchor->getName().size() >= 1);

    //    m_validator->addTrustAnchor(anchor);
    //    //std::cout << "valid certificate" << std::endl;
    //  }
    //else {
    //  //std::cout << "invalid certificate" << std::endl;
    //  //return;
    //  throw "invalid certificate";
    //}
  }

  void
  run()
  {
    Interest interest(Name("/root/site1"));
    interest.setInterestLifetime(time::milliseconds(1000));
    interest.setMustBeFresh(true);

    m_face->expressInterest(interest,
                            bind(&Consumer::onData, this,  _1, _2),
                            bind(&Consumer::onTimeout, this, _1));

    std::cout << "Sending " << interest << std::endl;

    // processEvents will block until the requested data received or timeout occurs
    m_face->processEvents();
  }

private:
  void
  onDataValidationFailed(const shared_ptr<const Data>& data,
                         const std::string& failureInfo)
  {
    std::cout << "failed" << failureInfo << std::endl;
  }

  void
  displayData(const ndn::shared_ptr<const ndn::Data>& data)
  {
    std::string message(reinterpret_cast<const char*>(data->getContent().value()),
                        data->getContent().value_size());
    std::cout << "msg: " << message << std::endl;
  }

  void
  onData(const Interest& interest, const Data& data)
  {
    //shared_ptr<Data> recData = make_shared<Data>(data);
    //std::cout << "locator : " << data.getSignature().getKeyLocator().getName() << std::endl;

    m_validator->validate(data, bind(&Consumer::displayData, this, _1),
                          bind(&Consumer::onDataValidationFailed, this, _1, _2));
  }

  void
  onTimeout(const Interest& interest)
  {
    std::cout << interest << " timeout " << std::endl;
  }

private:
  shared_ptr<Face> m_face;
  shared_ptr<ValidatorConfig> m_validator;
};

} // namespace examples
} // namespace ndn

int
main(int argc, char** argv)
{
  ndn::examples::Consumer consumer;
  try {
    consumer.initilize();
    consumer.run();
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return 0;
}
