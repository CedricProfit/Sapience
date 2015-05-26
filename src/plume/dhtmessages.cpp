// Copyright (c) 2014 The Sapience AIFX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dhtmessages.h"
#include "net.h"

bool CDhtStoreRequest::RelayTo(CNode* pnode) const
{
    // returns true if wasn't already contained in the set
    if (pnode->setKnown.insert(GetHash()).second)
    {
        pnode->PushMessage("dhtput", *this);
        return true;
    }
    return false;
}

bool CDhtGetResponse::RelayTo(CNode* pnode) const
{
    // returns true if wasn't already contained in the set
    if (pnode->setKnown.insert(GetHash()).second)
    {
        pnode->PushMessage("dhtgetr", *this);
        return true;
    }
    return false;
}


bool CDhtGetRequest::RelayTo(CNode* pnode) const
{
    // returns true if wasn't already contained in the set
    if (pnode->setKnown.insert(GetHash()).second)
    {
        pnode->PushMessage("dhtget", *this);
        return true;
    }
    return false;
}
