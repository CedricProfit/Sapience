// Copyright (c) 2014 The Sapience AIFX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef PLUME_DHT_MESSAGES_H
#define PLUME_DHT_MESSAGES_H

#include "util.h"
#include "net.h"

class CDhtGetRequest
{
public:
    int nVersion;
    uint256 infohash;

    CDhtGetRequest()
    {
        nVersion = 0;
    }

    IMPLEMENT_SERIALIZE(
        READWRITE(nVersion);
        READWRITE(infohash);
    )

    uint256 GetHash() const
    {
        return SerializeHash(*this);
    }

    bool RelayTo(CNode* pnode) const;
};

class CDhtGetResponse
{
public:
    int nVersion;
    uint256 infohash;
    std::string value;

    CDhtGetResponse()
    {
        nVersion = 0;
    }

    IMPLEMENT_SERIALIZE(
        READWRITE(nVersion);
        READWRITE(infohash);
        READWRITE(value);
    )

    uint256 GetHash() const
    {
        return SerializeHash(*this);
    }

    bool RelayTo(CNode* pnode) const;

};

class CDhtStoreRequest
{
public:
    int nVersion;
    uint256 infohash;
    std::string value;

    CDhtStoreRequest()
    {
        nVersion = 0;
    }

    IMPLEMENT_SERIALIZE(
        READWRITE(nVersion);
        READWRITE(infohash);
        READWRITE(value);
    )

    uint256 GetHash() const
    {
        return SerializeHash(*this);
    }

    bool RelayTo(CNode* pnode) const;
};

class CDhtStoreResponse
{
public:

};

#endif // PLUME_DHT_MESSAGES_H
