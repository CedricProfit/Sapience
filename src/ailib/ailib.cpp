// Copyright (c) 2014 The Sapience AIFX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ailib.h"
#include "util.h"
#include "ui_interface.h"
#include "net.h"
#include "plume/plumecore.h"

CNeuralNetworkHeader AiLib::CreateNeuralNetwork(int nInputs, int nHidden, int nOutputs, std::string sNetworkName, bool fRunLocally, uint256 nStatePlumeId)
{
    CNeuralNetworkHeader hdr;
    hdr.nInputs = nInputs;
    hdr.nHidden = nHidden;
    hdr.nOutputs = nOutputs;
    hdr.sNetworkName = sNetworkName;
    hdr.fRunLocally = fRunLocally;
    hdr.nStatePlumeId = nStatePlumeId;
    LOCK(cs_plumecore);
    mapMyNeuralNetworks.insert(make_pair(hdr.GetNeuralNetworkId(), hdr));

    // store
    SaveMyNetworks();
    hdr.Broadcast();
    // return
    NeuralNetworkHeaderNotify(hdr);
    return hdr;
}

std::string AiLib::HelloWorld()
{
    return "Hello World";
}

void AiLib::Vocalize(std::string sentence)
{
    uiInterface.NotifyVocalizeRequest(sentence);
}

void CNeuralNetworkHeader::Broadcast() const
{
    // send to all nodes
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes)
    {
        if((pnode->nServices & NODE_AI))
        {
            // returns true if wasn't already contained in the set
            if (pnode->setKnown.insert(GetHash()).second)
            {
                pnode->PushMessage("nnhdr", *this);
                PlumeMsgNotify(GetTime(), "Send", pnode->addrName, "nnhdr", this->GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION));
            }
        }
    }
}

bool CNeuralNetworkHeader::RelayTo(CNode* pnode) const
{
    // returns true if wasn't already contained in the set
    if (pnode->setKnown.insert(GetHash()).second)
    {
        pnode->PushMessage("nnhdr", *this);
        return true;
    }
    return false;
}
