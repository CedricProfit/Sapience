// Copyright (c) 2014 The Sapience AIFX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "dataplume.h"
#include "init.h"
#include "key.h"
#include "keystore.h"
#include "plumeapi.h"
#include "plumecore.h"
#include "plumelocaldb.h"
#include "plumeheader.h"
#include "util.h"
#include "wallet.h"

using namespace std;

std::vector<uint256> CPlumeApi::GetLastInfohashes(uint256 plumeId, int count, std::string& err)
{
    std::vector<uint256> infohashes;
    std::set<uint256> infohashSet;
    std::map<int64_t, uint256> mapByTime;
    bool readResult = ReadPlumeInfohashes(plumeId, infohashSet);
    if(!readResult)
    {
        err = "Couldn't read from database.";
        return infohashes;
    }

    BOOST_FOREACH(uint256 infohash, infohashSet)
    {
        // get the data record
        CSignedPlumeDataItem dataItem;
        std::string err;
        if(!GetRecord(infohash, 30, dataItem, err))
            return infohashes;

        mapByTime.insert(make_pair(dataItem.dataItem.key.nCreatedTime, infohash));
    }

    int i = 0;
    BOOST_REVERSE_FOREACH(PAIRTYPE(const int64_t, uint256)& p, mapByTime)
    {
        if(i < count)
        {
            infohashes.push_back(p.second);
            i += 1;
        }
        else
        {
            break;
        }
    }

    return infohashes;
}

std::vector<uint256> CPlumeApi::GetPlumeInfohashes(uint256 plumeId, int count, int offset, std::string& err)
{
    std::vector<uint256> infohashes;
    std::set<uint256> infohashSet;
    bool readResult = ReadPlumeInfohashes(plumeId, infohashSet);
    if(!readResult)
    {
        err = "Couldn't read from database.";
        return infohashes;
    }

    int i = 0;
    int j = 0;
    BOOST_FOREACH(uint256 infohash, infohashSet)
    {
        if((i+1) < offset)
        {
            i += 1;
            continue;
        }
        else
        {
            if(j < count)
            {
                infohashes.push_back(infohash);
                j += 1;
            }
            else
            {
                break;
            }
        }
    }

    return infohashes;
}

bool CPlumeApi::GetRecord(uint256 infohash, int timeout, CSignedPlumeDataItem &dataItem, std::string& err)
{
    // first check locally
    std::string strValue;
    bool foundLocal = ReadDhtValue(infohash, strValue);
    if(foundLocal)
    {
        CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_NETWORK, CLIENT_VERSION);
        ssValue >> dataItem;
        return true;
    }
    else
    {

    }

    err = "Infohash item timed out or not found.";
    return false;
}

bool CPlumeApi::PreviewPlume(uint256 plumeId, int timeout, CSignedPlumeDataItem& dataItem)
{
    // find the first infohash for the plume
    std::string err;
    std::vector<uint256> hashes = GetPlumeInfohashes(plumeId, 1, 0, err);
    if(hashes.size() > 0)
    {
        uint256 infohash = hashes[0];
        std::string err;
        return GetRecord(infohash, timeout, dataItem, err);
    }
    else
    {
        return false;
    }
}

CPlumePeer CPlumeApi::GetPlumePeer(uint256 peerId)
{
    CPlumePeer peer;
    if(mapPlumePeers.find(peerId) != mapPlumePeers.end())
        peer = mapPlumePeers[peerId];
    return peer;
}

CPlumeHeader CPlumeApi::CreateDataPlume(std::string name, std::string attrOneName, std::string attrTwoName, std::string attrThreeName, int neuralNodesRequested, bool isPublic)
{
    CPlumeHeader hdr;
    hdr.sPlumeName = name;
    hdr.sAttrOneName = attrOneName;
    hdr.sAttrTwoName = attrTwoName;
    hdr.sAttrThreeName = attrThreeName;
    hdr.nNeuralNodesRequested = neuralNodesRequested;
    hdr.bIsPublic = isPublic;

    // set peer id
    hdr.nOriginatorPeerId = MyPeerId();

    // set pubkey
    hdr.originatorKey = pwalletMain->GenerateNewKey();

    hdr.nCreatedTime = GetTime();
    hdr.nLastUpdatedTime = GetTime();

    LOCK(cs_plumecore);
    mapMyDataPlumes.insert(make_pair(hdr.GetPlumeId(), hdr));

    // store
    SaveMyPlumes();
    hdr.Broadcast();
    // return
    PlumeHeaderNotify(hdr);
    return hdr;
}

uint256 CPlumeApi::CreateDataReservationRequest(CPlumeHeader header, double kbHours)
{
    CDataReservationRequest request;
    request.plumeHeader = header;
    request.nKilobyteHours = kbHours;
    request.nRequestTime = GetTime();

    // store
    LOCK(cs_plumecore);
    mapReservationsWaiting.insert(make_pair(request.GetHash(), request));
    SaveReservationsWaiting();

    request.Broadcast();

    return request.GetHash();
}

std::vector<CDataReservationRequest> CPlumeApi::ListDataReservationRequests()
{
    LOCK(cs_plumecore);
    std::vector<CDataReservationRequest> requests;
    BOOST_FOREACH(PAIRTYPE(const uint256, CDataReservationRequest)& p, mapReservationsWaiting)
    {
        requests.push_back(p.second);
    }
    return requests;
}

std::vector<CDataReservationProposal> CPlumeApi::ListDataReservationProposals(uint256 requestId)
{
    LOCK(cs_plumecore);
    std::vector<CDataReservationProposal> proposals;
    BOOST_FOREACH(PAIRTYPE(const uint256, CDataReservationProposal)& p, mapProposalsReceived)
    {
        if(p.second.nRequestHash == requestId)
            proposals.push_back(p.second);
    }
    return proposals;
}

std::vector<CDataReservationProposal> CPlumeApi::ListDataProposalsWaiting()
{
    std::vector<CDataReservationProposal> waiting;
    LOCK(cs_plumecore);
    BOOST_FOREACH(PAIRTYPE(const uint256, CDataReservationProposal)&p, mapProposalsWaiting)
    {
        waiting.push_back(p.second);
    }
    return waiting;
}

std::vector<CDataProposalAcceptance> CPlumeApi::ListProposalsAccepted()
{
    std::vector<CDataProposalAcceptance> accepts;
    LOCK(cs_plumecore);
    BOOST_FOREACH(PAIRTYPE(const uint256, CDataProposalAcceptance)& p, mapProposalsAccepted)
    {
        accepts.push_back(p.second);
    }
    return accepts;
}

bool CPlumeApi::AcceptProposal(uint256 proposalId, std::string& err)
{
    if(mapProposalsReceived.find(proposalId) == mapProposalsReceived.end())
    {
        err = "Data Reservation Proposal with this proposalId not found in local queue.";
        return false;
    }

    CDataReservationProposal prop = mapProposalsReceived[proposalId];

    if(mapReservationsWaiting.find(prop.nRequestHash) == mapReservationsWaiting.end())
    {
        err = "Data Reservation Request not found in local queue.";
        return false;
    }

    CDataReservationRequest req = mapReservationsWaiting[prop.nRequestHash];

    // plume should be in our local map
    if(mapMyDataPlumes.find(req.plumeHeader.GetPlumeId()) == mapMyDataPlumes.end())
    {
        err = "Data Plume not found in local database.";
        return false;
    }

    CPlumeHeader hdr = mapMyDataPlumes[req.plumeHeader.GetPlumeId()];

    // send payment
    double totalCost = req.nKilobyteHours * prop.nKilobyteHourRate;
    int64_t nAmount = roundint64(totalCost * COIN);
    CBitcoinAddress address(prop.paymentPubKey.GetID());
    CWalletTx wtx;
    string sNarr;
    string strError = pwalletMain->SendMoneyToDestination(address.Get(), nAmount, sNarr, wtx);
    if(strError != "")
    {
        err = strError;
        return false;
    }

    // add peer to neural node list
    LOCK(cs_plumecore);
    hdr.lNeuralNodes.insert(prop.nPeerId);
    mapMyDataPlumes[hdr.GetPlumeId()] = hdr;
    if(!WriteMyPlumeMap(mapMyDataPlumes))
    {
        err = "Updating neural node list to disk failed.";
        return false;
    }

    CDataProposalAcceptance accept;
    accept.nProposalHash = proposalId;
    mapProposalsAccepted.insert(make_pair(accept.nProposalHash, accept));
    accept.Broadcast();
    if(!WriteAcceptanceQueue(mapProposalsAccepted))
    {
        err = "Saving proposal acceptance queue to disk failed.";
        return false;
    }

    err = "";
    return true;
}

bool CPlumeApi::AddRecord(CPlumeDataItem dataItem, std::string& err)
{
    if(mapMyDataPlumes.find(dataItem.key.nPlumeId) == mapMyDataPlumes.end())
        return false;

    CPlumeHeader header = mapMyDataPlumes[dataItem.key.nPlumeId];
    CPlumeContext plumeContext(header);
    CSignedPlumeDataItem signedItem;
    signedItem.dataItem = dataItem;
    plumeContext.SignData(dataItem, signedItem.vchDataSig);
    signedItem.Broadcast();
    return CheckAndStoreData(signedItem, err);
}

std::vector<CDataChunk> CPlumeApi::GetChunksAvailable(uint256 plumeId)
{
    LOCK(cs_plumecore);
    std::vector<CDataChunk> chunks;
    if(mapChunksWaiting.find(plumeId) != mapChunksWaiting.end())
        chunks = mapChunksWaiting[plumeId];
    return chunks;
}

void CPlumeApi::ClearChunkQueue(uint256 plumeId)
{
    LOCK(cs_plumecore);
    if(mapChunksWaiting.find(plumeId) != mapChunksWaiting.end())
        mapChunksWaiting.erase(plumeId);
}

bool CPlumeApi::RequestGetChunk(uint256 plumeId, int64_t chunkStart)
{
    CDataStream ssValue(SER_NETWORK, PROTOCOL_VERSION);
    ssValue << plumeId;
    ssValue << chunkStart;
    CPlumePeer peer = GetNextAvailableDhtPeer();
    peer.pnode->PushMessage("plmchk", ssValue);
    return true;
}

bool CPlumeApi::RequestGet(uint256 infohash)
{
    CDhtGetRequest request;
    request.infohash = infohash;
    CPlumePeer peer = GetNextAvailableDhtPeer();
    return request.RelayTo(peer.pnode);
}

std::vector<uint256> CPlumeApi::ListDhtGetResponsesAvailable()
{
    LOCK(cs_plumecore);
    std::vector<uint256> responses;
    BOOST_FOREACH(PAIRTYPE(const uint256, CDhtGetResponse)& p, mapGetResponses)
    {
        responses.push_back(p.first);
    }
    return responses;
}

bool CPlumeApi::GetDhtResponse(uint256 infohash, CDhtGetResponse& response)
{
    LOCK(cs_plumecore);
    if(mapGetResponses.find(infohash) == mapGetResponses.end())
        return false;

    response = mapGetResponses[infohash];
    return true;
}

std::vector<CPlumeHeader> CPlumeApi::ListMyDataPlumes()
{
    LOCK(cs_plumecore);
    std::vector<CPlumeHeader> headers;
    BOOST_FOREACH(PAIRTYPE(const uint256, CPlumeHeader)& p, mapMyDataPlumes)
    {
        headers.push_back(p.second);
    }
    return headers;
}

std::vector<CPlumeHeader> CPlumeApi::ListPublicDataPlumes()
{
    LOCK(cs_plumecore);
    std::vector<CPlumeHeader> headers;
    BOOST_FOREACH(PAIRTYPE(const uint256, CPlumeHeader)& p, mapPublicDataPlumes)
    {
        headers.push_back(p.second);
    }
    return headers;
}

std::vector<CPlumeHeader> CPlumeApi::ListMyServicedDataPlumes()
{
    LOCK(cs_plumecore);
    std::vector<CPlumeHeader> headers;
    BOOST_FOREACH(PAIRTYPE(const uint256, CPlumeHeader)& p, mapMyServicedPlumes)
    {
        headers.push_back(p.second);
    }
    return headers;
}

std::vector<CPlumePeer> CPlumeApi::ListPlumePeers()
{
    LOCK(cs_plumecore);
    std::vector<CPlumePeer> peers;
    BOOST_FOREACH(PAIRTYPE(const uint256, CPlumePeer)& p, mapPlumePeers)
    {
        peers.push_back(p.second);
    }
    return peers;
}

CPlumeHeader CPlumeApi::GetPlume(uint256 plumeId)
{
    LOCK(cs_plumecore);
    CPlumeHeader hdr;
    if(mapMyDataPlumes.find(plumeId) != mapMyDataPlumes.end())
        hdr = mapMyDataPlumes[plumeId];
    else if(mapPublicDataPlumes.find(plumeId) != mapPublicDataPlumes.end())
        hdr = mapPublicDataPlumes[plumeId];
    else if(mapMyServicedPlumes.find(plumeId) != mapMyServicedPlumes.end())
        hdr = mapMyServicedPlumes[plumeId];

    return hdr;
}
