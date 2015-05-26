// Copyright (c) 2014 The Sapience AIFX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef PLUMEAPI_H
#define PLUMEAPI_H

#include "dataplume.h"
#include "plumecore.h"
#include "plumeheader.h"

class CPlumeApi
{
public:
    CPlumeHeader CreateDataPlume(std::string name, std::string attrOneName, std::string attrTwoName, std::string attrThreeName, int neuralNodesRequested, bool isPublic);
    uint256 CreateDataReservationRequest(CPlumeHeader header, double kbHours);
    std::vector<CDataReservationRequest> ListDataReservationRequests();
    std::vector<CDataReservationProposal> ListDataReservationProposals(uint256 requestId);
    std::vector<CDataReservationProposal> ListDataProposalsWaiting();
    std::vector<CDataProposalAcceptance> ListProposalsAccepted();
    bool AcceptProposal(uint256 proposalId, std::string& err);
    bool AddRecord(CPlumeDataItem dataItem, std::string& err);
    std::vector<CDataChunk> GetChunksAvailable(uint256 plumeId);
    void ClearChunkQueue(uint256 plumeId);
    bool RequestGetChunk(uint256 plumeId, int64_t chunkStart);
    bool RequestGet(uint256 infohash);
    std::vector<uint256> ListDhtGetResponsesAvailable();
    bool GetDhtResponse(uint256 infohash, CDhtGetResponse& response);
    std::vector<CPlumeHeader> ListMyDataPlumes();
    std::vector<CPlumeHeader> ListPublicDataPlumes();
    std::vector<CPlumeHeader> ListMyServicedDataPlumes();
    std::vector<CPlumePeer> ListPlumePeers();
    CPlumeHeader GetPlume(uint256 plumeId);
    CPlumePeer GetPlumePeer(uint256 peerId);
    bool PreviewPlume(uint256 plumeId, int timeout, CSignedPlumeDataItem& dataItem);
    std::vector<uint256> GetPlumeInfohashes(uint256 plumeId, int count, int offset, std::string& err);
    bool GetRecord(uint256 infohash, int timeout, CSignedPlumeDataItem& dataItem, std::string& err);
    std::vector<uint256> GetLastInfohashes(uint256 plumeId, int count, std::string& err);
};


#endif // PLUMEAPI_H
