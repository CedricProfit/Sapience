// Copyright (c) 2014 The Sapience AIFX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef AILIB_H
#define AILIB_H

#include "key.h"
#include "net.h"
#include "util.h"

enum TRANSFER_FUNCTION
{
    SIGMOID,
    TANH
};

enum LEARNING_RULE
{
    BACKPROPAGATION,
    BACKPROPAGATION_WITH_MOMENTUM
};

using namespace std;

class CNeuralNetworkHeader
{
public:
    std::string sNetworkName;
    uint256 nOriginatorPeerId;
    CPubKey originatorKey;
    int nInputs;
    int nHidden;
    int nOutputs;
    uint256 nStatePlumeId;
    bool fRunLocally;
    int nTransferFunction;
    int nLearningRule;
    std::string sNeuronTypeAttr;
    std::string sSequenceAttr;
    int64_t nExpirationDate;
    int64_t nCreatedTime;
    int64_t nLastUpdatedTime;

    CNeuralNetworkHeader()
    {
        nExpirationDate = GetTime();
        nCreatedTime = GetTime();
        nLastUpdatedTime = GetTime();
        nTransferFunction = SIGMOID;
        nLearningRule = BACKPROPAGATION;
    }

    IMPLEMENT_SERIALIZE(
            READWRITE(sNetworkName);
            READWRITE(nOriginatorPeerId);
            READWRITE(originatorKey);
            READWRITE(nInputs);
            READWRITE(nHidden);
            READWRITE(nOutputs);
            READWRITE(nStatePlumeId);
            READWRITE(fRunLocally);
            READWRITE(nTransferFunction);
            READWRITE(nLearningRule);
            READWRITE(sNeuronTypeAttr);
            READWRITE(sSequenceAttr);
            READWRITE(nExpirationDate);
            READWRITE(nCreatedTime);
            READWRITE(nLastUpdatedTime);
        )

    uint256 GetNeuralNetworkId() const
    {
        std::string sId = sNetworkName + nOriginatorPeerId.ToString();
        return Hash(sId.begin(), sId.end());
    }

    uint256 GetHash() const
    {
        return SerializeHash(*this);
    }

    void Broadcast() const;
    bool RelayTo(CNode* pnode) const;

};

class AiLib
{
public:
    std::string HelloWorld();
    void Vocalize(std::string sentence);
    CNeuralNetworkHeader CreateNeuralNetwork(int nInputs, int nHidden, int nOutputs, std::string sNetworkName, bool fRunLocally, uint256 nStatePlumeId);
};

#endif
