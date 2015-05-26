// Copyright (c) 2014 The Sapience AIFX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef PLUME_LOCALDB_H
#define PLUME_LOCALDB_H

#include "plumepeer.h"
#include "dht.h"
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include "db.h"

extern CCriticalSection cs_localdb; // for locking leveldb operations

bool WriteDhtValue(uint256 infohash, std::string dhtvalue);
bool ReadDhtValue(uint256 infohash, std::string& value);
bool HaveDhtValue(uint256 infohash);
bool ReadPlumePeer(uint256 peerid, CPlumePeer& plumepeer);
bool WritePlumePeer(uint256 peerid, CPlumePeer& plumepeer);
bool HavePlumePeer(uint256 peerid);
bool ReadProposalQueue(std::map<uint256, CDataReservationProposal>& mapProposals);
bool WriteProposalQueue(std::map<uint256, CDataReservationProposal> mapProposals);
bool ReadProposalWaitingQueue(std::map<uint256, CDataReservationProposal>& mapProposals);
bool WriteProposalWaitingQueue(std::map<uint256, CDataReservationProposal> mapProposals);
bool ReadReservationQueue(std::map<uint256, CDataReservationRequest>& mapRequests);
bool WriteReservationQueue(std::map<uint256, CDataReservationRequest> mapRequests);
bool ReadAcceptanceQueue(std::map<uint256, CDataProposalAcceptance>& mapAccepts);
bool WriteAcceptanceQueue(std::map<uint256, CDataProposalAcceptance> mapAccepts);
bool ReadPublicPlumeMap(std::map<uint256, CPlumeHeader>& mapPlumes);
bool WritePublicPlumeMap(std::map<uint256, CPlumeHeader> mapPlumes);
bool ReadMyPlumeMap(std::map<uint256, CPlumeHeader>& mapPlumes);
bool WriteMyPlumeMap(std::map<uint256, CPlumeHeader> mapPlumes);
bool ReadServicedPlumeMap(std::map<uint256, CPlumeHeader>& mapPlumes);
bool WriteServicedPlumeMap(std::map<uint256, CPlumeHeader> mapPlumes);
bool ReadPeerLastDhtUpdateMap(std::map<uint256, int64_t>& mapPeerTimes);
bool WritePeerLastDhtUpdateMap(std::map<uint256, int64_t> mapPeerTimes);
bool ReadInfohashPeerList(uint256 infohash, std::vector<uint256>& peerList);
bool WriteInfohashPeerList(uint256 infohash, std::vector<uint256> peerList);
bool ReadServicedPlumes(std::set<uint256>& setServicedPlumes);
bool WriteServicedPlumes(std::set<uint256> setServicedPlumes);
bool ReadPlumeInfohashes(uint256 plumeId, std::set<uint256>& infohashes);
bool WritePlumeInfohashes(uint256 plumeId, std::set<uint256> infohashes);
bool HavePlumeInfohashes(uint256 plumeId);
std::vector<CPlumePeer> ReadAllPlumePeers();

bool ReadMyNeuralNetworks(std::map<uint256, CNeuralNetworkHeader>& mapNetworks);
bool WriteMyNeuralNetworks(std::map<uint256, CNeuralNetworkHeader> mapNetworks);

bool TryOpenLocalDB();
size_t GetSizeBytesOnDisk();
uint CountDht();

class LocalDB
{
public:
    LocalDB()
    {
        activeBatch = NULL;
    };

    ~LocalDB()
    {
        if (activeBatch)
            delete activeBatch;
    };

    bool Open(const char* pszMode="r+");

    bool ScanBatch(const CDataStream& key, std::string* value, bool* deleted) const;

    bool TxnBegin();
    bool TxnCommit();
    bool TxnAbort();

    bool ReadDhtValue(uint256 infohash, std::string& dhtvalue);
    bool WriteDhtValue(uint256 infohash, std::string dhtvalue);
    bool ExistsDhtInfoHash(uint256 infohash);

    bool ReadPlumePeer(uint256 peerid, CPlumePeer& plumepeer);
    bool WritePlumePeer(uint256 peerid, CPlumePeer& plumepeer);
    bool ExistsPlumePeer(uint256 peerid);

    bool ReadProposalQueue(std::map<uint256, CDataReservationProposal>& mapProposals);
    bool WriteProposalQueue(std::map<uint256, CDataReservationProposal> mapProposals);
    bool ReadProposalWaitingQueue(std::map<uint256, CDataReservationProposal>& mapProposals);
    bool WriteProposalWaitingQueue(std::map<uint256, CDataReservationProposal> mapProposals);
    bool ReadReservationQueue(std::map<uint256, CDataReservationRequest>& mapRequests);
    bool WriteReservationQueue(std::map<uint256, CDataReservationRequest> mapRequests);
    bool ReadAcceptanceQueue(std::map<uint256, CDataProposalAcceptance>& mapAccepts);
    bool WriteAcceptanceQueue(std::map<uint256, CDataProposalAcceptance> mapAccepts);

    bool ReadPublicPlumeMap(std::map<uint256, CPlumeHeader>& mapPlumes);
    bool WritePublicPlumeMap(std::map<uint256, CPlumeHeader> mapPlumes);
    bool ReadMyPlumeMap(std::map<uint256, CPlumeHeader>& mapPlumes);
    bool WriteMyPlumeMap(std::map<uint256, CPlumeHeader> mapPlumes);
    bool ReadServicedPlumeMap(std::map<uint256, CPlumeHeader>& mapPlumes);
    bool WriteServicedPlumeMap(std::map<uint256, CPlumeHeader> mapPlumes);

    bool ReadPeerLastDhtUpdateMap(std::map<uint256, int64_t>& mapPeerTimes);
    bool WritePeerLastDhtUpdateMap(std::map<uint256, int64_t> mapPeerTimes);
    bool ReadInfohashPeerList(uint256 infohash, std::vector<uint256>& peerList);
    bool WriteInfohashPeerList(uint256 infohash, std::vector<uint256> peerList);

    bool ReadServicedPlumes(std::set<uint256>& setServicedPlumes);
    bool WriteServicedPlumes(std::set<uint256> setServicedPlumes);

    bool ReadPlumeInfohashes(uint256 plumeId, std::set<uint256>& infohashes);
    bool WritePlumeInfohashes(uint256 plumeId, std::set<uint256> infohashes);
    bool ExistsPlumeInfohashes(uint256 plumeId);

    bool ReadMyNeuralNetworks(std::map<uint256, CNeuralNetworkHeader>& mapNetworks);
    bool WriteMyNeuralNetworks(std::map<uint256, CNeuralNetworkHeader> mapNetworks);

    leveldb::DB *pdb;       // points to the global instance
    leveldb::WriteBatch *activeBatch;

};

#endif // PLUME_LOCALDB_H
