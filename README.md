# ndn-validator-examples

Tools for personal use.

  This repository contains some examples for demonstrating how to use ndn validators.
Two validators are used here: one is the validator-regex, the other is validator-config.

  Basically the producer/consumer examples in the ndn-cxx library are modified here. The producer-sign
publishes data by signing it using the key chain (CERTIFICATE-CHAIN shows how to create the certificate
 chain from scatch). consumer-sign utilizes validator-regex to verify the data, and the 
consumer-sign-config uses validator-config. The certs-producer are providing the producer's certificate.
Those examples can be improved in many ways, but here just some naive examples to teach new users on
how to use ndn-cxx library validators.
