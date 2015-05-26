// Copyright (c) 2014 The Sapience AIFX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dataplume.h"
#include "init.h"
#include "key.h"
#include "keystore.h"
#include "wallet.h"

bool CPlumeContext::SignData(CPlumeDataItem item, std::vector<unsigned char>& vchDataSig)
{
    CKeyID keyId = plume.originatorKey.GetID();
    CKey key;

    if(pwalletMain->GetKey(keyId, key))
    {
        return key.Sign(item.GetHash(), vchDataSig);
    }
    else
    {
        return false;
    }
}

bool CPlumeContext::CheckSignature(CSignedPlumeDataItem item)
{
    CKey key;
    if (!key.SetPubKey(plume.originatorKey))
        return false;

    return key.Verify(item.dataItem.GetHash(), item.vchDataSig);
}

bool CDataReservationProposal::RelayTo(CNode* pnode) const
{
    // returns true if wasn't already contained in the set
    if (pnode->setKnown.insert(GetHash()).second)
    {
        pnode->PushMessage("plmdrp", *this);
        return true;
    }
    return false;
}

void CDataReservationRequest::Broadcast() const
{
    // send to all nodes
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes)
    {
        if((pnode->nServices & NODE_PLUME))
        {
            // returns true if wasn't already contained in the set
            if (pnode->setKnown.insert(GetHash()).second)
            {
                pnode->PushMessage("plmdrr", *this);
                PlumeMsgNotify(GetTime(), "Send", pnode->addrName, "plmdrr", this->GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION));
            }
        }
    }
}


bool CSignedPlumeDataItem::RelayTo(CNode* pnode) const
{
    // returns true if wasn't already contained in the set
    if (pnode->setKnown.insert(GetHash()).second)
    {
        pnode->PushMessage("plmdata", *this);
        return true;
    }
    return false;
}

void CSignedPlumeDataItem::Broadcast() const
{
    // send to all nodes
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes)
    {
        if((pnode->nServices & NODE_PLUME))
        {
            // returns true if wasn't already contained in the set
            if (pnode->setKnown.insert(GetHash()).second)
            {
                pnode->PushMessage("plmdata", *this);
                PlumeMsgNotify(GetTime(), "Send", pnode->addrName, "plmdata", this->GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION));
            }
        }
    }
}

void CDataProposalAcceptance::Broadcast() const
{
    // send to all nodes
    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes)
    {
        if((pnode->nServices & NODE_PLUME))
        {
            // returns true if wasn't already contained in the set
            if (pnode->setKnown.insert(GetHash()).second)
            {
                pnode->PushMessage("plmdra", *this);
                PlumeMsgNotify(GetTime(), "Send", pnode->addrName, "plmdra", this->GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION));
            }
        }
    }
}
