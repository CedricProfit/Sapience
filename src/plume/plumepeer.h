// Copyright (c) 2014 The Sapience AIFX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef PLUMEPEER_H
#define PLUMEPEER_H

#include "net.h"
#include "util.h"

/* Announcement message to broadcast Plume peer */
class CPlumeAnnounce {
public:
    uint256 nPlumePeerId;           // IP independent identifier for this node.  Hash256 of its default PubKey.
    uint nAllocatedBytes;           // Bytes of space allocated by this node for Plume
    uint nAvailableBytes;           // Bytes of space left on this node for storage
    uint64_t nTotalDhtEntries;      // Total # of entries in the DHT on this node
    double nDataReservationFee;   // Fee this node charges for data reservations, in XAI/kilobyte-hour
    double nDataAccessFee;        // Fee this node charges for public data access, in XAI/query-hour
    int64_t nRelayUntil;            // Relay this message until this time

    IMPLEMENT_SERIALIZE(
        READWRITE(nPlumePeerId);
        READWRITE(nAllocatedBytes);
        READWRITE(nAvailableBytes);
        READWRITE(nTotalDhtEntries);
        READWRITE(nDataReservationFee);
        READWRITE(nDataAccessFee);
        READWRITE(nRelayUntil);
    )

    void Init();
    void Broadcast();

    uint256 GetHash() const
    {
        return SerializeHash(*this);
    }
};

/* A peer that supports Plume DB, encapsulates CNode to add additional attributes */
class CPlumePeer {
public:
    CPlumeAnnounce peerinfo;        // Announcement info from the plume peer
    CNode* pnode;                   // Reference to the core node object for this peer
    CAddress address;               // Address of this peer, for repopulating pnode reference
    uint nTtl;                      // TTL for this node
    uint nUseScore;                 // Incremented on each data store put, rolls over, used for load balancing

    CPlumePeer()
    {
        nTtl = 0;
        nUseScore = 0;
    }

    IMPLEMENT_SERIALIZE(
        READWRITE(peerinfo);
        READWRITE(address);
        READWRITE(nTtl);
        READWRITE(nUseScore);
    )

    void Init();                    // Initialize object
    double PercentUtilization();    // Get the % space utilization on this node, e.g. 16%
    double PercentAvailable();
    void AssociateNode();           // Find a reference to the code CNode peer object
    std::string ToString();
};


#endif // PLUMEPEER_H
