// Copyright (c) 2014 The Sapience AIFX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "plumecore.h"
#include "plumepeer.h"
#include "net.h"
#include "ui_interface.h"
#include <iostream>

void CPlumePeer::Init()
{
    pnode = NULL;
    nTtl = 0;
    nUseScore = 0;
    peerinfo.Init();
}

double CPlumePeer::PercentUtilization()
{
    if(peerinfo.nAllocatedBytes == 0)
        return 0;
    else
        return 100 * ( (peerinfo.nAllocatedBytes - peerinfo.nAvailableBytes) / peerinfo.nAllocatedBytes);
}

double CPlumePeer::PercentAvailable()
{
    if(peerinfo.nAllocatedBytes == 0)
        return 0;
    else
        return 100.0 * ((double)peerinfo.nAvailableBytes / (double)peerinfo.nAllocatedBytes);
}

void CPlumePeer::AssociateNode()
{
    // try to look up and get a reference to the core node
    pnode = FindNode(address);
}

std::string CPlumePeer::ToString()
{
    std::stringstream ret;
    ret << std::fixed;
    ret << std::setprecision(8);
    ret << "addr: " << address.ToStringIPPort();
    ret << "\nusescore: " << (int64_t)nUseScore;
    ret << "\nplumepeerid: " << peerinfo.nPlumePeerId.ToString();
    ret << "\nallocatedbytes: " << (int64_t)peerinfo.nAllocatedBytes;
    ret << "\navailablebytes: " << (int64_t)peerinfo.nAvailableBytes;
    ret << "\ndatareservationfee: " << peerinfo.nDataReservationFee;
    ret << "\ndataaccessfee: " << peerinfo.nDataAccessFee;
    ret << "\ntotaldht: " << (int64_t)peerinfo.nTotalDhtEntries;
    return ret.str();
}


void CPlumeAnnounce::Init()
{
    nPlumePeerId = MyPeerId();
    nAllocatedBytes = 0;
    nAvailableBytes = 0;
    nTotalDhtEntries = 0;
    nDataReservationFee = 0;
    nDataAccessFee = 0;
    nRelayUntil = GetAdjustedTime() + 60;  // valid until 60 seconds from now
}

void CPlumeAnnounce::Broadcast()
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
                if (GetAdjustedTime() < nRelayUntil)
                {
                    pnode->PushMessage("plmpeer", *this);
                    PlumeMsgNotify(GetTime(), "Send", pnode->addrName, "plmpeer", this->GetSerializeSize(SER_NETWORK, PROTOCOL_VERSION));
                }
            }
        }
    }
}
