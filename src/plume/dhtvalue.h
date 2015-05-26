// Copyright (c) 2014 The Sapience AIFX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef PLUME_DHT_VALUE_H
#define PLUME_DHT_VALUE_H

class DhtValue {
public:
    uint256 nPlumeId;   // the data plume this value belongs to
    int64_t nNonce;     // nonce to avoid collisions
    std::string sValue; // all values are json
    // pubkey of originator
    // signature of originator
    // expiration time
};

#endif // PLUME_DHT_VALUE_H
