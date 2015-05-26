// Copyright (c) 2014 The Sapience AIFX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef PLUMEHEADER_H
#define PLUMEHEADER_H

#include "util.h"
#include "key.h"
#include "net.h"

class CPlumeHeader
{
public:
    std::string sPlumeName;
    uint256 nOriginatorPeerId;
    CPubKey originatorKey;
    std::string sAttrOneName;
    std::string sAttrTwoName;
    std::string sAttrThreeName;
    int64_t nExpirationDate;
    int nNeuralNodesRequested;
    bool bIsPublic;
    std::set<uint256> lNeuralNodes;
    int64_t nCreatedTime;
    int64_t nLastUpdatedTime;

    CPlumeHeader()
    {
        nExpirationDate = GetTime();
        nCreatedTime = GetTime();
        nLastUpdatedTime = GetTime();
    }

    IMPLEMENT_SERIALIZE(
            READWRITE(sPlumeName);
            READWRITE(nOriginatorPeerId);
            READWRITE(originatorKey);
            READWRITE(sAttrOneName);
            READWRITE(sAttrTwoName);
            READWRITE(sAttrThreeName);
            READWRITE(nExpirationDate);
            READWRITE(nNeuralNodesRequested);
            READWRITE(bIsPublic);
            READWRITE(lNeuralNodes);
            READWRITE(nCreatedTime);
            READWRITE(nLastUpdatedTime);
        )

    uint256 GetPlumeId() const
    {
        std::string sId = sPlumeName + nOriginatorPeerId.ToString();
        return Hash(sId.begin(), sId.end());
    }

    uint256 GetHash() const
    {
        return SerializeHash(*this);
    }

    void Broadcast() const;
    bool RelayTo(CNode* pnode) const;

};

#endif // PLUMEHEADER_H
