// Copyright (c) 2014 The Sapience AIFX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef DATAPLUME_H
#define DATAPLUME_H

#include "util.h"
#include "plumeheader.h"

#include <boost/lexical_cast.hpp>


class CDataReservationRequest
{
public:
    int nVersion;
    CPlumeHeader plumeHeader;
    double nKilobyteHours;
    int64_t nRequestTime;

    CDataReservationRequest()
    {
        nVersion = 0;
    }

    IMPLEMENT_SERIALIZE(
        READWRITE(nVersion);
        READWRITE(plumeHeader);
        READWRITE(nKilobyteHours);
        READWRITE(nRequestTime);
    )

    uint256 GetHash() const
    {
        return SerializeHash(*this);
    }

    void Broadcast() const;
};

class CDataReservationProposal
{
public:
    int nVersion;
    uint256 nRequestHash;
    double nKilobyteHourRate;
    int64_t nResponseTime;
    CPubKey paymentPubKey;
    uint256 nPeerId;

    CDataReservationProposal()
    {
        nVersion = 0;
    }

    IMPLEMENT_SERIALIZE(
        READWRITE(nVersion);
        READWRITE(nRequestHash);
        READWRITE(nKilobyteHourRate);
        READWRITE(nResponseTime);
        READWRITE(paymentPubKey);
        READWRITE(nPeerId);
    )

    uint256 GetHash() const
    {
        return SerializeHash(*this);
    }

    bool RelayTo(CNode* pnode) const;
};

class CDataProposalAcceptance
{
public:
    int nVersion;
    uint256 nProposalHash;

    CDataProposalAcceptance()
    {
        nVersion = 0;
    }

    IMPLEMENT_SERIALIZE(
        READWRITE(nVersion);
        READWRITE(nProposalHash);
    )

    uint256 GetHash() const
    {
        return SerializeHash(*this);
    }

    void Broadcast() const;
};

class CPlumeKey
{
public:
    int nVersion;
    uint256 infohash;
    uint256 nPlumeId;
    std::string attributeOneValue;
    std::string attributeTwoValue;
    std::string attributeThreeValue;
    int nNonce;
    int64_t nCreatedTime;

    CPlumeKey()
    {
        std::srand(std::time(0));
        nNonce = std::rand();
    }

    CPlumeKey(uint256 plumeId, std::string attrOne, std::string attrTwo, std::string attrThree)
    {
        nVersion = 0;
        nPlumeId = plumeId;
        attributeOneValue = attrOne;
        attributeTwoValue = attrTwo;
        attributeThreeValue = attrThree;
        nCreatedTime = GetTime();
        std::srand(std::time(0));
        nNonce = std::rand();
        infohash = CreateInfoHash();
    }

    IMPLEMENT_SERIALIZE(
        READWRITE(nVersion);
        READWRITE(infohash);
        READWRITE(nPlumeId);
        READWRITE(attributeOneValue);
        READWRITE(attributeTwoValue);
        READWRITE(attributeThreeValue);
        READWRITE(nNonce);
        READWRITE(nCreatedTime);
    )

    uint256 GetHash() const
    {
        return SerializeHash(*this);
    }

    uint256 CreateInfoHash() const
    {
        std::string key = nPlumeId.ToString() + attributeOneValue + attributeTwoValue + attributeThreeValue + boost::lexical_cast<std::string>(nNonce);
        return Hash(key.begin(), key.end());
    }
};

class CPlumeDataItem
{
public:
    int nVersion;
    CPlumeKey key;
    std::string value;

    CPlumeDataItem()
    {
        nVersion = 0;
    }

    IMPLEMENT_SERIALIZE(
        READWRITE(nVersion);
        READWRITE(key);
        READWRITE(value);
    )

    uint256 GetHash() const
    {
        return SerializeHash(*this);
    }
};

// Data items are signed by the originator using the key associated with the data plume
class CSignedPlumeDataItem
{
public:
    int nVersion;
    CPlumeDataItem dataItem;
    std::vector<unsigned char> vchDataSig;

    IMPLEMENT_SERIALIZE(
        READWRITE(nVersion);
        READWRITE(dataItem);
        READWRITE(vchDataSig);
    )

    CSignedPlumeDataItem()
    {
        nVersion = 0;
    }

    uint256 GetHash() const
    {
        return SerializeHash(*this);
    }

    void Broadcast() const;
    bool RelayTo(CNode* pnode) const;
};

class CDataChunk
{
public:
    int64_t nChunkStart;
    uint256 nPlumeId;
    std::vector<CSignedPlumeDataItem> dataItems;
};

class CPlumeContext
{
public:
    CPlumeHeader plume;

    CPlumeContext(CPlumeHeader plumeHeader)
    {
        plume = plumeHeader;
    }

    bool SignData(CPlumeDataItem item, std::vector<unsigned char>& vchDataSig);
    bool CheckSignature(CSignedPlumeDataItem item);
};

#endif // DATAPLUME_H
