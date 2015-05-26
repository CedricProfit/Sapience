// Copyright (c) 2014 The Sapience AIFX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "net.h"
#include "plumeheader.h"
#include "plumecore.h"

void CPlumeHeader::Broadcast() const
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
                pnode->PushMessage("plmhdr", *this);
                PlumeMsgNotify(GetTime(), "Send", pnode->addrName, "plmhdr", this->GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION));
            }
        }
    }
}

bool CPlumeHeader::RelayTo(CNode* pnode) const
{
    // returns true if wasn't already contained in the set
    if (pnode->setKnown.insert(GetHash()).second)
    {
        pnode->PushMessage("plmhdr", *this);
        return true;
    }
    return false;
}
