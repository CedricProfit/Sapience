// Copyright (c) 2014 The Sapience AIFX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef PLUME_CORE_H
#define PLUME_CORE_H

#include "net.h"
#include "util.h"
#include "main.h"
#include "dhtmessages.h"
#include "plumepeer.h"
#include "plumeheader.h"
#include "dataplume.h"
#include "ailib/ailib.h"

extern bool fPlumeEnabled;

extern CCriticalSection cs_plumecore;
extern int nAnnounceInterval;
const int PLUME_ANNOUNCE_INTERVAL = 10;                             // interval in seconds between broadcasting our peer info
const int PLUME_ALLOCATED_DEFAULT = 1000000000;                    // Allocate 1000 MB (1GB) by default
const int PLUME_MAX_RECORD_BYTES = 1000000;                        // 1MB - Maximum record size allowed
const int64_t PLUME_MAX_EXPIRATION_DURATION = 30 * 24 * 60 * 60;    // maximum 30 days reservation
const double PLUME_DEFAULT_DATA_RESERVATION_RATE = 0.00001;   // XAI/kilobyte-hour
const double PLUME_DEFAULT_DATA_ACCESS_RATE = 0.0000001;          // XAI/query-hour
extern uint256 myPlumeId;                                           // my plume peer id (hash256 of default pubkey)
extern double nDataReservationRate;
extern double nDataAccessRate;
extern uint nAllocatedBytes;
extern uint nTotalDhtEntries;

extern std::map<uint256, CNeuralNetworkHeader> mapMyNeuralNetworks;  // map of neural networks this node originated


extern std::map<uint256, CPlumePeer> mapPlumePeers;                 // in memory map of known plume peers with associated node
extern std::map<uint256, CPlumeHeader> mapMyDataPlumes;             // map of data plumes this node originated
extern std::map<uint256, CPlumeHeader> mapMyServicedPlumes;                  // map of data plumes this node is a Neural Node for
extern std::map<uint256, CPlumeHeader> mapPublicDataPlumes;                  // map of public data plumes

extern std::map<uint256, CDataReservationRequest> mapReservationsWaiting;    // data reservation requests sent, awaiting proposals
extern std::map<uint256, CDataReservationProposal> mapProposalsReceived;     // data reservation proposals received, awaiting acceptance
extern std::map<uint256, CDataReservationProposal> mapProposalsWaiting;      // data reservation proposals waiting for client to accept
extern std::map<uint256, CDataProposalAcceptance> mapProposalsAccepted;     // data reservation proposals accepted

extern std::map<uint256, int64_t> mapPeerLastDhtInvUpdate;                   // last time we requested infohash inventory from the peer
extern std::map<uint256, std::vector<uint256> > mapPeerInfoHashes;           // list of peers for each infohash

extern std::set<uint256> setServicedPlumes;                                  // plumes I am a Neural Node for
extern std::map<uint256, std::vector<CDataChunk> > mapChunksWaiting;                       // chunks awaiting use
extern std::map<uint256, CDhtGetResponse> mapGetResponses;                   // responses to get requests

bool SaveMyNetworks();
bool SaveMyPlumes();
bool SaveReservationsWaiting();
void InitializePlumeCore(void* parg);
void PlumeProcess();
bool PlumeReceiveMessage(CNode* pfrom, std::string strCommand, CDataStream& vRecv);
uint256 MyPeerId();
void PlumeMsgNotify(int64_t nTime, std::string msgType, std::string peerAddress, std::string msg, uint sizeKb);
void PlumePeerNotify(CPlumePeer plumepeer);
void PlumePeerCountNotify(int peersConnected);
void PlumeProposalNotify(CDataReservationProposal proposal);
int PlumeLog(const char * pszFormat, ...);
void PlumeHeaderNotify(CPlumeHeader header);
void NeuralNetworkHeaderNotify(CNeuralNetworkHeader header);

CPlumeContext GetPlumeContext(uint256 plumeId);
CPlumePeer GetNextAvailableDhtPeer();

void GetChunk(CNode* pfrom, uint256 plumeId, int64_t chunkStart);
void ReceiveChunk(std::vector<std::string> dataItems, int64_t chunkStart, uint256 plumeId);
int64_t GetLastInv(CPlumePeer pp);
void SendPlmInv(CPlumePeer pp);
bool PutDht(CDhtStoreRequest put, std::string& err);
void GetDht(CNode* pfrom, CDhtGetRequest get);
bool CheckAndStoreData(CSignedPlumeDataItem dataItem, std::string& err);
void RespondToDataReservationRequest(CNode* pfrom, CDataReservationRequest request);
void ReceiveProposal(CDataReservationProposal proposal);
void ReceiveAcceptance(CDataProposalAcceptance acceptance);
void GetResponse(CDhtGetResponse getr);

#endif // PLUME_CORE_H
