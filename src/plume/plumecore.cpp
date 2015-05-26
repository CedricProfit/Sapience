// Copyright (c) 2014 The Sapience AIFX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dhtmessages.h"
#include "plumecore.h"
#include "main.h"
#include "plumepeer.h"
#include "wallet.h"
#include "util.h"
#include "init.h"
#include "plumelocaldb.h"
#include "ailib/ailib.h"

using namespace std;
using namespace boost;

extern bool bPlumeUserEnabled;

CCriticalSection cs_plumecore;
std::map<uint256, CNeuralNetworkHeader> mapMyNeuralNetworks;          // map of neural networks this node originated
std::map<uint256, CPlumeHeader> mapMyDataPlumes;                      // map of data plumes this node originated
std::map<uint256, CPlumeHeader> mapMyServicedPlumes;                  // map of data plumes this node is a Neural Node for
std::map<uint256, CPlumeHeader> mapPublicDataPlumes;                  // map of public data plumes
std::map<uint256, CDataReservationRequest> mapReservationsWaiting;    // data reservation requests sent, awaiting proposals
std::map<uint256, CDataReservationProposal> mapProposalsReceived;     // data reservation proposals received, awaiting acceptance
std::map<uint256, CDataReservationProposal> mapProposalsWaiting;      // data reservation proposals waiting for client to accept
std::map<uint256, CDataProposalAcceptance> mapProposalsAccepted;     // data reservation proposals accepted
std::map<uint256, int64_t> mapPeerLastDhtInvUpdate;                   // last time we requested infohash inventory from the peer
std::map<uint256, std::vector<uint256> > mapPeerInfoHashes;           // list of peers for each infohash
std::set<uint256> setServicedPlumes;                                  // plumes I am a Neural Node for
std::map<uint256, CDhtGetResponse> mapGetResponses;                   // responses to get requests

std::map<uint256, std::vector<CDataChunk> > mapChunksWaiting;                       // chunks awaiting use

CPlumePeer lastDhtPeer;
int nAnnounceInterval = PLUME_ANNOUNCE_INTERVAL;

std::map<uint256, CPlumePeer> mapPlumePeers;
int64_t nTimeLastAnnounce = GetTime();
int64_t nTimeLastStats = GetTime();
uint256 myPlumeId = uint256(0);
double nDataReservationRate = PLUME_DEFAULT_DATA_RESERVATION_RATE;
double nDataAccessRate = PLUME_DEFAULT_DATA_ACCESS_RATE;
unsigned int nAllocatedBytes = PLUME_ALLOCATED_DEFAULT;
unsigned int nTotalDhtEntries = 0;
int64_t nLastMsgNotify = GetTime();
int64_t nLastInvTime = 0;

void NeuralNetworkHeaderNotify(CNeuralNetworkHeader header)
{
    uiInterface.NotifyNeuralNetworkUpdated(header);
    PlumeLog("Neural Network Header received.");
}

void PlumeHeaderNotify(CPlumeHeader header)
{
    uiInterface.NotifyDataPlumeUpdated(header);
    PlumeLog("Data Plume Header received.");
}

void PlumeProposalNotify(CDataReservationProposal proposal)
{
    uiInterface.NotifyPlumeProposalReceived(proposal);
    PlumeLog("Data Reservation Proposal received");
}

void PlumePeerCountNotify(int peersConnected)
{
    uiInterface.NotifyPlumeConnectedPeerCountChanged(peersConnected);
    PlumeLog("%d plume peers connected", peersConnected);
}

void PlumePeerNotify(CPlumePeer plumepeer)
{
    uiInterface.NotifyPlumePeer(plumepeer);
    PlumeLog("Plume peer update received %s", plumepeer.address.ToString().c_str());
}

void PlumeMsgNotify(int64_t nTime, std::string msgType, std::string peerAddress, std::string msg, unsigned int sizeKb)
{
/*#ifdef ANDROID
    if(GetTime() - nLastMsgNotify > 5)
    {
        nLastMsgNotify = GetTime();
        uiInterface.NotifyPlumeMessage(nTime, msgType, peerAddress, msg, sizeKb);
    }
#else*/
    uiInterface.NotifyPlumeMessage(nTime, msgType, peerAddress, msg, sizeKb);
//#endif

#ifndef ANDROID
    PlumeLog("%s %s %s %d", msgType.c_str(), peerAddress.c_str(), msg.c_str(), sizeKb);
#endif
}

int PlumeLog(const char * pszFormat, ...)
{
//#ifndef ANDROID
    std::string buffer;
    buffer += strprintf("%s ", DateTimeStrFormat("%x %H:%M:%S", GetTime()).c_str());
    va_list arg_ptr;
    va_start(arg_ptr, pszFormat);
    buffer += vstrprintf(pszFormat, arg_ptr);
    va_end(arg_ptr);

    // raise a signal with the log item to surface in UI
    uiInterface.NotifyPlumeLogEntry(buffer);
//#endif

    if(fDebug)
    {
        // write as usual
        int ret = 0;
        va_list arg_ptrb;
        va_start(arg_ptrb, pszFormat);
        ret = printf(pszFormat, arg_ptrb);
        va_end(arg_ptrb);
    }
    return 0;
}

bool SaveMyNetworks()
{
    LOCK(cs_plumecore);
    return WriteMyNeuralNetworks(mapMyNeuralNetworks);
}

bool SaveMyPlumes()
{
    LOCK(cs_plumecore);
    return WriteMyPlumeMap(mapMyDataPlumes);
}

bool SaveReservationsWaiting()
{
    LOCK(cs_plumecore);
    return WriteReservationQueue(mapReservationsWaiting);
}

void InitializePlumeCore(void* parg)
{
    PlumeLog("Initializing Plume Core...");

    // set our configured data reservation rate
    if (mapArgs.count("-datareservationrate"))
    {
        nDataReservationRate = GetDoubleArg("-datareservationrate", PLUME_DEFAULT_DATA_RESERVATION_RATE);
        PlumeLog("datareservationrate configured: %d", nDataReservationRate);
    }

    // set our configured data access rate
    if (mapArgs.count("-dataaccessrate"))
    {
        nDataAccessRate = GetDoubleArg("-dataaccessrate", PLUME_DEFAULT_DATA_ACCESS_RATE);
        PlumeLog("dataaccessrate configured: %d", nDataAccessRate);
    }

    PlumeLog("My Peer Id: %s", MyPeerId().ToString().c_str());
    PlumeLog("Attempting to open plume local database...");
    if(TryOpenLocalDB())
    {
        PlumeLog("Successfully opened plume local database");

        // read in values from disk
        /*std::map<uint256, CPlumeHeader> mapMyDataPlumes;                      // map of data plumes this node originated
        std::map<uint256, CPlumeHeader> mapMyServicedPlumes;                  // map of data plumes this node is a Neural Node for
        std::map<uint256, CPlumeHeader> mapPublicDataPlumes;                  // map of public data plumes
        std::map<uint256, CDataReservationRequest> mapReservationsWaiting;    // data reservation requests sent, awaiting proposals
        std::map<uint256, CDataReservationProposal> mapProposalsReceived;     // data reservation proposals received, awaiting acceptance
        std::map<uint256, CDataReservationProposal> mapProposalsWaiting;      // data reservation proposals waiting for client to accept
        std::map<uint256, CDataProposalAcceptance> mapProposalsAccepted;     // data reservation proposals accepted
        std::map<uint256, int64_t> mapPeerLastDhtInvUpdate;                   // last time we requested infohash inventory from the peer
        std::map<uint256, std::vector<uint256> > mapPeerInfoHashes;           // list of peers for each infohash
        std::set<uint256> setServicedPlumes;                                  // plumes I am a Neural Node for
        */

        LOCK(cs_plumecore);
        ReadMyPlumeMap(mapMyDataPlumes);
        ReadPeerLastDhtUpdateMap(mapPeerLastDhtInvUpdate);
        ReadProposalQueue(mapProposalsReceived);
        ReadProposalWaitingQueue(mapProposalsWaiting);
        ReadPublicPlumeMap(mapPublicDataPlumes);
        ReadReservationQueue(mapReservationsWaiting);
        ReadServicedPlumeMap(mapMyServicedPlumes);
        ReadServicedPlumes(setServicedPlumes);
        ReadAcceptanceQueue(mapProposalsAccepted);
        ReadMyNeuralNetworks(mapMyNeuralNetworks);
        std::vector<CPlumePeer> ppeers = ReadAllPlumePeers();
        BOOST_FOREACH(CPlumePeer p, ppeers)
        {
            p.AssociateNode();
            mapPlumePeers.insert(make_pair(p.peerinfo.nPlumePeerId, p));
        }
        ReadPeerLastDhtUpdateMap(mapPeerLastDhtInvUpdate);

        BOOST_FOREACH(PAIRTYPE(const uint256, CPlumeHeader)& p, mapMyServicedPlumes)
        {
            if(mapPublicDataPlumes.find(p.first) != mapPublicDataPlumes.end())
                mapPublicDataPlumes.erase(p.first);
        }

        BOOST_FOREACH(PAIRTYPE(const uint256, CPlumeHeader)& p, mapMyDataPlumes)
        {
            if(mapPublicDataPlumes.find(p.first) != mapPublicDataPlumes.end())
                mapPublicDataPlumes.erase(p.first);
        }

        BOOST_FOREACH(PAIRTYPE(const uint256, CPlumeHeader)& p, mapMyDataPlumes)
        {
            PlumeHeaderNotify(p.second);
        }

        BOOST_FOREACH(PAIRTYPE(const uint256, CPlumeHeader)& p, mapPublicDataPlumes)
        {
            PlumeHeaderNotify(p.second);
        }

        BOOST_FOREACH(PAIRTYPE(const uint256, CPlumeHeader)& p, mapMyServicedPlumes)
        {
            PlumeHeaderNotify(p.second);
        }

        BOOST_FOREACH(PAIRTYPE(const uint256, CNeuralNetworkHeader)& p, mapMyNeuralNetworks)
        {
            NeuralNetworkHeaderNotify(p.second);
        }
    }
    else
        PlumeLog("WARNING: Could not open plume local database!");
}

// Plume main loop thread
void PlumeProcess()
{
    PlumeLog("Starting Plume processing thread...");

    SetThreadPriority(THREAD_PRIORITY_LOWEST);

    // Make this thread recognisable as the Plume thread
    RenameThread("sapience-plume");

    while(true && fPlumeEnabled)
    {
        if (fShutdown)
            return;

        if(!bPlumeUserEnabled)
        {
            MilliSleep(60000);
            continue;
        }

        if(GetTime() - nTimeLastStats > 120)
        {
            nTotalDhtEntries = CountDht();
            PlumeLog("Total DHT Entries: %d", nTotalDhtEntries);
            PlumeLog("Total known plume peers %d", mapPlumePeers.size());
            // update peer list connected states
            BOOST_FOREACH(const PAIRTYPE(uint256, CPlumePeer)& p, mapPlumePeers)
            {
                PlumePeerNotify(p.second);
            }

            nTimeLastStats = GetTime();
        }

        // Broadcast our announce every X seconds
        if(GetTime() - nTimeLastAnnounce > nAnnounceInterval)
        {
            PlumeLog("Broadcasting Plume announce");

            // send a CPlumeAnnounce message
            CPlumeAnnounce announce;
            announce.Init();
            size_t used = GetSizeBytesOnDisk();
            announce.nAllocatedBytes = nAllocatedBytes;
            announce.nAvailableBytes = nAllocatedBytes - used;
            announce.nDataAccessFee = nDataAccessRate;
            announce.nDataReservationFee = nDataReservationRate;
            announce.nTotalDhtEntries = nTotalDhtEntries;
            announce.Broadcast();

            nTimeLastAnnounce = GetTime();
        }

#ifdef ANDROID
        MilliSleep(500);
#else
        MilliSleep(100);
#endif
    }
}

uint256 MyPeerId()
{
    if(myPlumeId == uint256(0))
    {
        myPlumeId = pwalletMain->vchDefaultKey.GetHash();
    }

    return myPlumeId;
}


bool PlumeReceiveMessage(CNode* pfrom, std::string strCommand, CDataStream& vRecv)
{
    if(!pfrom || !pfrom->fSuccessfullyConnected || pfrom->fDisconnect) // make sure they didn't drop off
        return false;

    PlumeLog("PlumeReceiveMessage() %s %s.", pfrom->addrName.c_str(), strCommand.c_str());
    PlumeMsgNotify(GetTime(), "Receive", pfrom->addrName, strCommand, vRecv.size());

    if(strCommand == "plmpeer")
    {
        // a plume peer announcement received
        // if we already have this peer, update it
        CPlumeAnnounce announce;
        vRecv >> announce;

        uint256 announceHash = announce.GetHash();
        if (pfrom->setKnown.count(announceHash) == 0)
        {
            // haven't already processed this particular announcement
            CPlumePeer pp;
            pp.peerinfo = announce;
            pp.address = pfrom->addr;
            pp.pnode = pfrom;

            // update set of known nodes
            if(mapPlumePeers.find(pp.peerinfo.nPlumePeerId) != mapPlumePeers.end())
                mapPlumePeers[pp.peerinfo.nPlumePeerId] = pp;
            else
                mapPlumePeers.insert(make_pair(pp.peerinfo.nPlumePeerId, pp));

            // signal that node update occurred
            PlumePeerNotify(pp);

            pfrom->setKnown.insert(announceHash);

            // send inventory request
            SendPlmInv(pp);

            // count peers
            int numPeers = 0;
            BOOST_FOREACH(PAIRTYPE(const uint256, CPlumePeer)& p, mapPlumePeers)
            {
                if(p.second.pnode && p.second.pnode->fSuccessfullyConnected && ! p.second.pnode->fDisconnect)
                    numPeers++;
            }
            PlumePeerCountNotify(numPeers);
        }
        else
        {
            PlumeLog("Already processed this plume announce");
        }
    }
    else if(strCommand == "plminv")
    {
        // request for plume inventory since a time
        int64_t nTimeSince = 0;
        vRecv >> nTimeSince;

        BOOST_FOREACH(PAIRTYPE(const uint256, CPlumeHeader)& p, mapPublicDataPlumes)
        {
            if(p.second.nLastUpdatedTime >= nTimeSince)
                p.second.RelayTo(pfrom);
        }
    }
    else if(strCommand == "plmhdr")
    {
        // received a data plume header
        CPlumeHeader plumeHdr;
        vRecv >> plumeHdr;
        if(plumeHdr.bIsPublic)
        {
            // only track if its a public plume, otherwise we don't care (and peer shouldn't be broadcasting their private plumes!)
            LOCK(cs_plumecore);
            mapPublicDataPlumes.insert(make_pair(plumeHdr.GetPlumeId(), plumeHdr));
            // flush to disk
            WritePublicPlumeMap(mapPublicDataPlumes);
        }

    }
    else if(strCommand == "plmdata")
    {
        // add data to a data plume
        // check if we are a Neural Node first, if not ignore
        CSignedPlumeDataItem dataItem;
        vRecv >> dataItem;
        if(setServicedPlumes.find(dataItem.dataItem.key.nPlumeId) != setServicedPlumes.end())
        {
            // we are a Neural Node
            string pdErr;
            if(!CheckAndStoreData(dataItem, pdErr))
                PlumeLog("plmdata Error: %s", pdErr.c_str());
        }
    }
    else if(strCommand == "plmdrr")
    {
        // Data Reservation Request
        // check if we can service this node, if so send proposal
        CDataReservationRequest request;
        vRecv >> request;
        RespondToDataReservationRequest(pfrom, request);
    }
    else if(strCommand == "plmdrp")
    {
        // Data Reservation Proposal
        // check that it belongs to one of our data reservation requests first
        CDataReservationProposal proposal;
        vRecv >> proposal;
        ReceiveProposal(proposal);
    }
    else if(strCommand == "plmdra")
    {
        // Data Reservation Proposal Acceptance
        // the peer has accepted our proposal to provide Neural Node services
        CDataProposalAcceptance acceptance;
        vRecv >> acceptance;
        ReceiveAcceptance(acceptance);
    }
    else if(strCommand == "plmgetchk")
    {
        // get a chunk of up to 100 infohashes starting from provided index
        uint256 plumeId;
        int64_t ind = 0;
        vRecv >> plumeId;
        vRecv >> ind;
        GetChunk(pfrom, plumeId, ind);
    }
    else if(strCommand == "plmchkres")
    {
        // plume chunk response, put it in memory and raise a signal
        std::vector<std::string> dataItems;
        int64_t start = 0;
        uint256 nPlumeId;
        vRecv >> dataItems;
        vRecv >> start;
        vRecv >> nPlumeId;
        ReceiveChunk(dataItems, start, nPlumeId);
    }
    else if(strCommand == "dhtput")
    {
        // put a dht entry
        CDhtStoreRequest put;
        vRecv >> put;
        string dhtErr;
        if(!PutDht(put, dhtErr))
            PlumeLog("dhtput Error: %s", dhtErr.c_str());
    }
    else if(strCommand == "dhtget")
    {
        // get a dht entry
        CDhtGetRequest get;
        vRecv >> get;
        GetDht(pfrom, get);
    }
    else if(strCommand == "dhtgetr")
    {
        // get response
        CDhtGetResponse getr;
        vRecv >> getr;
        GetResponse(getr);
    }
    else
    {
        // ignore unknown commands
    }

    return true;
}

void GetChunk(CNode* pfrom, uint256 plumeId, int64_t chunkStart)
{
    if(!pfrom || pfrom->fDisconnect)
        return;

    // given a plume, get up to 100 infohashes from it
    // and send a vector of deserialized CSignedPlumeDataItems
    std::set<uint256> infohashes;
    if(ReadPlumeInfohashes(plumeId, infohashes))
    {
        // we have a list of infohashes for the plume
        if(infohashes.size() > chunkStart)
        {
            std::vector<std::string> items;
            int idx = 0;
            BOOST_FOREACH(uint256 p, infohashes)
            {
                if(idx < chunkStart)
                {
                    idx++;
                    continue;
                }

                if(idx >= (chunkStart + 100))
                    break;

                idx++;
                std::string item;
                if(ReadDhtValue(p, item))
                {
                    items.push_back(item);
                }
            }

            if(items.size() > 0)
            {
                CDataStream ssValue(SER_NETWORK, PROTOCOL_VERSION);
                ssValue << items;
                ssValue << chunkStart;
                ssValue << plumeId;
                pfrom->PushMessage("plmchkres", ssValue);
            }
        }
    }
}

void ReceiveChunk(std::vector<std::string> dataItems, int64_t chunkStart, uint256 plumeId)
{
    CDataChunk chunk;
    // deserialize data items
    BOOST_FOREACH(std::string p, dataItems)
    {
        CDataStream ssValue(p.data(), p.data() + p.size(), SER_NETWORK, PROTOCOL_VERSION);
        CSignedPlumeDataItem signedItem;
        ssValue >> signedItem;
        chunk.dataItems.push_back(signedItem);
    }

    chunk.nChunkStart = chunkStart;
    chunk.nPlumeId = plumeId;

    if(mapChunksWaiting.find(plumeId) == mapChunksWaiting.end())
    {
        std::vector<CDataChunk> chunks;
        mapChunksWaiting.insert(make_pair(plumeId, chunks));
    }

    mapChunksWaiting[plumeId].push_back(chunk);
    uiInterface.NotifyPlumeChunkReceived(plumeId, chunkStart);
}

void GetResponse(CDhtGetResponse getr)
{
    // we got a response to our get
    // put on queue and raise signal
    mapGetResponses.insert(make_pair(getr.infohash, getr));
    uiInterface.NotifyPlumeDhtGetResponse(getr.infohash);
}

bool PutDht(CDhtStoreRequest put, std::string& err)
{
    // write to local DHT
    if(!WriteDhtValue(put.infohash, put.value))
    {
        err = "Error writing dht value to local database";
        return false;
    }

    // send to dht node
    for(int i=0; i < 8; i++)
    {
        CPlumePeer p = GetNextAvailableDhtPeer();

        // relay to node
        put.RelayTo(p.pnode);
    }

    err = "";
    return true;
}

void GetDht(CNode* pfrom, CDhtGetRequest get)
{
    if(!pfrom || pfrom->fDisconnect)
        return;

    if(HaveDhtValue(get.infohash))
    {
        CDhtGetResponse getr;
        getr.infohash = get.infohash;
        ReadDhtValue(getr.infohash, getr.value);
        getr.RelayTo(pfrom);
    }
}

bool CheckAndStoreData(CSignedPlumeDataItem dataItem, std::string& err)
{
    uint256 plumeId = dataItem.dataItem.key.nPlumeId;
    // check we are servicing this or it is one of our owned plumes
    if(setServicedPlumes.find(plumeId) == setServicedPlumes.end() &&
            mapMyDataPlumes.find(plumeId) == mapMyDataPlumes.end())
    {
        err = "Plume not owned or serviced by this node.";
        return false;
        // TODO: Send a failure message back
    }

    // verify signature
    CPlumeHeader header;
    if(mapMyServicedPlumes.find(plumeId) != mapMyServicedPlumes.end())
        header = mapMyServicedPlumes[plumeId];
    else if(mapMyDataPlumes.find(plumeId) != mapMyDataPlumes.end())
        header = mapMyDataPlumes[plumeId];

    CPlumeContext plumeContext(header);
    if(!plumeContext.CheckSignature(dataItem))
    {
        // TODO: DoS score peer
        err = "Signature check failed on data item.";
        return false;
    }
    else
    {
        // signature is ok
        // add the data item to our own index
        std::set<uint256> infohashes;
        if(HavePlumeInfohashes(dataItem.dataItem.key.nPlumeId))
            ReadPlumeInfohashes(dataItem.dataItem.key.nPlumeId, infohashes);

        infohashes.insert(dataItem.dataItem.key.infohash);
        WritePlumeInfohashes(dataItem.dataItem.key.nPlumeId, infohashes);

        // TODO: add to PHT

        // write to local DHT
        CDhtStoreRequest put;
        put.infohash = dataItem.dataItem.key.infohash;
        CDataStream ssValue(SER_NETWORK, CLIENT_VERSION);
        ssValue.reserve(sizeof(dataItem));
        ssValue << dataItem;
        put.value = ssValue.str();

        return PutDht(put, err);
    }
}

CPlumePeer GetNextAvailableDhtPeer()
{
    CPlumePeer p = lastDhtPeer;
    BOOST_FOREACH(PAIRTYPE(const uint256, CPlumePeer)& pair, mapPlumePeers)
    {
        CPlumePeer peer = pair.second;
        if(!p.pnode && peer.pnode && (peer.pnode->fSuccessfullyConnected == true) && !peer.pnode->fDisconnect)
        {
            p = pair.second;
        }
        else if(p.pnode && (peer.nUseScore < p.nUseScore) && peer.pnode && (peer.pnode->fSuccessfullyConnected == true && !peer.pnode->fDisconnect))
        {
            p = pair.second;
        }
        else
        {
            break;
        }
    }
    p.nUseScore++;
    if(p.nUseScore > 5)
        p.nUseScore = 1;

    return p;
}

void RespondToDataReservationRequest(CNode* pfrom, CDataReservationRequest request)
{
    if(!pfrom || pfrom->fDisconnect)
        return;

    // check that we have capacity
    if(GetSizeBytesOnDisk() >= nAllocatedBytes)
        return; // we can't service this

    CDataReservationProposal p;
    p.nKilobyteHourRate = nDataReservationRate;
    p.nResponseTime = GetTime();
    p.nRequestHash = request.GetHash();
    p.paymentPubKey = pwalletMain->GenerateNewKey();
    p.nPeerId = MyPeerId();
    p.RelayTo(pfrom); // relay back to node

    LOCK(cs_plumecore);
    mapProposalsWaiting.insert(make_pair(p.GetHash(), p));
    // write to disk
}

void ReceiveProposal(CDataReservationProposal proposal)
{
    LOCK(cs_plumecore);
    // make sure we got a proposal for one of our reservations
    if(mapReservationsWaiting.find(proposal.nRequestHash) == mapReservationsWaiting.end())
        return; // doesn't apply to us

    // proposal applies to us
    mapProposalsReceived.insert(make_pair(proposal.GetHash(), proposal));
    uiInterface.NotifyPlumeProposalReceived(proposal);
    // save to disk
    if(WriteProposalQueue(mapProposalsReceived))
    {
        PlumeLog("Proposal received and written to disk.");
    }
    else
    {
        PlumeLog("Error writing received proposal to disk.");
    }
}

void ReceiveAcceptance(CDataProposalAcceptance acceptance)
{
    LOCK(cs_plumecore);
    // check if we already have it
    if(mapProposalsWaiting.find(acceptance.nProposalHash) == mapProposalsWaiting.end())
        return; // we didn't send this proposal

    CDataReservationProposal drp = mapProposalsWaiting[acceptance.nProposalHash];

    if(mapReservationsWaiting.find(drp.nRequestHash) == mapReservationsWaiting.end())
        return; // we don't have the original request

    CDataReservationRequest drr = mapReservationsWaiting[drp.nRequestHash];

    if(mapProposalsAccepted.find(acceptance.nProposalHash) != mapProposalsAccepted.end())
        return;

    mapReservationsWaiting.erase(drp.nRequestHash);
    mapProposalsAccepted.insert(make_pair(acceptance.nProposalHash, acceptance));

    setServicedPlumes.insert(drr.plumeHeader.GetPlumeId());
    mapMyServicedPlumes.insert(make_pair(drr.plumeHeader.GetPlumeId(), drr.plumeHeader));

    WriteReservationQueue(mapReservationsWaiting);
    WriteProposalQueue(mapProposalsReceived);
    WriteProposalWaitingQueue(mapProposalsWaiting);
    WriteAcceptanceQueue(mapProposalsAccepted);
    WriteServicedPlumes(setServicedPlumes);
    WriteServicedPlumeMap(mapMyServicedPlumes);

    uiInterface.NotifyProposalAccepted(acceptance.nProposalHash);
}

int64_t GetLastInv(CPlumePeer pp)
{
    // last inventory request time for this peer
    return 0;
}

void SendPlmInv(CPlumePeer pp)
{
    // last inv update time for the node
    int64_t lastInv = GetLastInv(pp);
    pp.pnode->PushMessage("plminv", lastInv);
}

CPlumeContext GetPlumeContext(uint256 plumeId)
{
    return CPlumeContext(mapMyDataPlumes[plumeId]);
}
