// Copyright (c) 2014 The Sapience AIFX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "init.h"
#include "main.h"
#include "bitcoinrpc.h"
#include "plumeapi.h"
#include "dataplume.h"
#include "dht.h"
#include "dhtmessages.h"
#include "plumeheader.h"
#include "plumepeer.h"
#include "util.h"
#include "base58.h"
#include "ailib/ailib.h"

using namespace json_spirit;
using namespace std;

/*
 * x plmpeerinfo - show plume peers
x plmlistreswaiting - list of data reservation requests waiting for proposals
x plmlistproposals - list of proposals received for a reservation request
x plmacceptproposal - accept a data reservation proposal
x plmcreateplume - create a data plume
x plmlistplumes - list plumes available
x plmplumedetails - show plume details
x plmplumepreview - preview data in a plume
x plmaddrecord - add a record to a data plume
x plmgetrecord - get a record from a data plume by infohash
x plmgethashes - get infohashes of data plume records
*/

Value plmgetlasthashes(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 2)
        throw runtime_error(
            "plmgetlasthashes <plumeid> <count>\n"
            "<plumeid> is the id of the plume\n"
            "<count> is the number of infohashes to retrieve\n"
            "Returns the most recent last N infohashes by created time.");

    uint256 plumeId = uint256(params[0].get_str());
    int count = params[1].get_int();
    CPlumeApi api;
    std::string err;
    std::vector<uint256> infohashes = api.GetLastInfohashes(plumeId, count, err);
    Object ret;
    Array arr;

    BOOST_FOREACH(uint256 p, infohashes)
    {
        arr.push_back(p.ToString());
    }

    ret.push_back(Pair("infohashes", arr));
    if(err != "")
        ret.push_back(Pair("error", err));
    return ret;
}

Value plmgethashes(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 3)
        throw runtime_error(
            "plmgethashes <plumeid> <count> [offset]\n"
            "<plumeid> is the id of the plume\n"
            "<count> is the number of infohashes to retrieve\n"
            "[offset default=0] is the starting index in list of infohashes");

    uint256 plumeId = uint256(params[0].get_str());
    int count = params[1].get_int();
    int offset = 0;
    if(params.size() == 3)
        offset = params[2].get_int();

    CPlumeApi api;
    std::string err;
    std::vector<uint256> infohashes = api.GetPlumeInfohashes(plumeId, count, offset, err);
    Object ret;
    Array arr;

    BOOST_FOREACH(uint256 p, infohashes)
    {
        arr.push_back(p.ToString());
    }

    ret.push_back(Pair("infohashes", arr));
    if(err != "")
        ret.push_back(Pair("error", err));
    return ret;
}

Value plmaddrecord(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 5 || params.size() > 5)
        throw runtime_error(
            "plmaddrecord <plumeid> <attrOneValue> <attrTwoValue> <attrThreeValue> <datavalue>\n"
            "<plumeid> is the id of the plume\n"
            "<attrOneValue> key attribute 1\n"
            "<attrTwoValue> key attribute 2\n"
            "<attrThreeValue> key attribute 3\n"
            "<datavalue> json data value");

    uint256 plumeId = uint256(params[0].get_str());
    std::string attrOneValue = params[1].get_str();
    std::string attrTwoValue = params[2].get_str();
    std::string attrThreeValue = params[3].get_str();
    std::string value = params[4].get_str();

    CPlumeApi api;
    CPlumeDataItem dataItem;
    CPlumeKey plumeKey(plumeId, attrOneValue, attrTwoValue, attrThreeValue);
    dataItem.key = plumeKey;
    dataItem.value = value;

    string err;
    bool successful = api.AddRecord(dataItem, err);
    Object ret;
    ret.push_back(Pair("successful", successful));
    if(!successful)
        ret.push_back(Pair("errors", err));
    return ret;
}

Value plmgetrecord(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 2)
        throw runtime_error(
            "plmgetrecord <infohash> <timeout>\n"
            "<infohash> infohash of the record\n"
            "<timeout> in seconds, how long to wait for data before timing out");

    uint256 infohash = uint256(params[0].get_str());
    int timeout = params[1].get_int();
    CPlumeApi api;
    CSignedPlumeDataItem dataItem;
    std::string err;
    bool successful = api.GetRecord(infohash, timeout, dataItem, err);
    Object ret;
    if(successful)
    {
        ret.push_back(Pair("infohash", dataItem.dataItem.key.infohash.ToString()));
        ret.push_back(Pair("attronevalue", dataItem.dataItem.key.attributeOneValue));
        ret.push_back(Pair("attrtwovalue", dataItem.dataItem.key.attributeTwoValue));
        ret.push_back(Pair("attrthreevalue", dataItem.dataItem.key.attributeThreeValue));
        ret.push_back(Pair("nonce", dataItem.dataItem.key.nNonce));
        ret.push_back(Pair("createdtime", DateTimeStrFormat(dataItem.dataItem.key.nCreatedTime)));
        ret.push_back(Pair("value", dataItem.dataItem.value));
    }
    else
    {
        ret.push_back(Pair("error", err));
    }
    return ret;
}




Value plmplumepreview(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 2)
        throw runtime_error(
            "plmplumepreview <plumeid> <timeout>\n"
            "<plumeid> is the id of the plume\n"
            "<timeout> in seconds, how long to wait for data before timing out\n");

    uint256 plumeId = uint256(params[0].get_str());
    int timeout = params[1].get_int();

    CPlumeApi api;
    CSignedPlumeDataItem dataItem;
    bool succeeded = api.PreviewPlume(plumeId, timeout, dataItem);

    Object ret;

    if(succeeded)
    {
        ret.push_back(Pair("infohash", dataItem.dataItem.key.infohash.ToString()));
        ret.push_back(Pair("attronevalue", dataItem.dataItem.key.attributeOneValue));
        ret.push_back(Pair("attrtwovalue", dataItem.dataItem.key.attributeTwoValue));
        ret.push_back(Pair("attrthreevalue", dataItem.dataItem.key.attributeThreeValue));
        ret.push_back(Pair("nonce", dataItem.dataItem.key.nNonce));
        ret.push_back(Pair("createdtime", DateTimeStrFormat(dataItem.dataItem.key.nCreatedTime)));
        ret.push_back(Pair("value", dataItem.dataItem.value));
    }
    else
    {
        ret.push_back(Pair("error", "timed out"));
    }

    return ret;
}

Value plmcreateplume(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 6 || params.size() > 7)
        throw runtime_error(
            "plmcreateplume <name> <attrOneName> <attrTwoName> <attrThreeName> <kbh> <neuralnodes> [public]\n"
            "<name> is the data reservation request hash"
            "<attrOneName> name of first key attribute\n"
            "<attrTwoName> name of second key attribute\n"
            "<attrThreeName> name of third key attribute\n"
            "<kbh> kilobyte-hours to reserve\n"
            "<neuralnodes> number of neural nodes desired\n"
            "[public default=false] optional param true if public plume, false is private plume");

    std::string plumeName = params[0].get_str();
    std::string attrOneName = params[1].get_str();
    std::string attrTwoName = params[2].get_str();
    std::string attrThreeName = params[3].get_str();
    double kbh = params[4].get_real();
    int neuralNodes = params[5].get_int();
    bool ispublic = false;
    if(params.size() == 7)
        ispublic = params[6].get_bool();

    CPlumeApi api;
    CPlumeHeader hdr = api.CreateDataPlume(plumeName, attrOneName, attrTwoName, attrThreeName, neuralNodes, ispublic);
    uint256 drrId = api.CreateDataReservationRequest(hdr, kbh);

    Object ret;
    ret.push_back(Pair("plumeid", hdr.GetPlumeId().ToString()));
    ret.push_back(Pair("plumename", hdr.sPlumeName));
    ret.push_back(Pair("attrone", hdr.sAttrOneName));
    ret.push_back(Pair("attrtwo", hdr.sAttrTwoName));
    ret.push_back(Pair("attrthree", hdr.sAttrThreeName));
    ret.push_back(Pair("originatorpeerid", hdr.nOriginatorPeerId.ToString()));
    ret.push_back(Pair("created", DateTimeStrFormat(hdr.nCreatedTime)));
    ret.push_back(Pair("neuralnodesrequested", hdr.nNeuralNodesRequested));
    ret.push_back(Pair("datareservationrequest", drrId.ToString()));

    return ret;
}


Value plmpeerinfo(const Array& params, bool fHelp)
{
    if (fHelp)
        throw runtime_error(
            "plmpeerinfo\n"
            "Shows info about plume peers.");

    CPlumeApi api;
    std::vector<CPlumePeer> peers = api.ListPlumePeers();
    Array ret;
    BOOST_FOREACH(CPlumePeer p, peers)
    {
        Object obj;

        obj.push_back(Pair("addr", p.address.ToStringIPPort()));
        obj.push_back(Pair("usescore", (int64_t)p.nUseScore));
        obj.push_back(Pair("plumepeerid", p.peerinfo.nPlumePeerId.ToString()));
        obj.push_back(Pair("allocatedbytes", (int64_t)p.peerinfo.nAllocatedBytes));
        obj.push_back(Pair("availablebytes", (int64_t)p.peerinfo.nAvailableBytes));
        obj.push_back(Pair("datareservationfee", p.peerinfo.nDataReservationFee));
        obj.push_back(Pair("dataaccessfee", p.peerinfo.nDataAccessFee));
        obj.push_back(Pair("totaldht", (int64_t)p.peerinfo.nTotalDhtEntries));

        ret.push_back(obj);
    }

    return ret;
}

Value plmlistreswaiting(const Array& params, bool fHelp)
{
    if (fHelp)
        throw runtime_error(
            "plmlistreswaiting\n"
            "Lists the data reservation requests in the queue");

    Array ret;
    CPlumeApi api;
    std::vector<CDataReservationRequest> requests = api.ListDataReservationRequests();
    BOOST_FOREACH(CDataReservationRequest request, requests)
    {
        Object obj;
        obj.push_back(Pair("plumeid", request.plumeHeader.GetPlumeId().ToString()));
        obj.push_back(Pair("plumename", request.plumeHeader.sPlumeName));
        obj.push_back(Pair("kilobytehours", request.nKilobyteHours));
        obj.push_back(Pair("requesttime", DateTimeStrFormat(request.nRequestTime)));
        obj.push_back(Pair("requestid", request.GetHash().ToString()));

        ret.push_back(obj);
    }

    return ret;
}

Value plmlistproposals(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 1)
        throw runtime_error(
            "plmlistproposals <requestId>\n"
            "<requestId> is the data reservation request hash");

    uint256 requestId = uint256(params[0].get_str());
    Array ret;
    CPlumeApi api;
    std::vector<CDataReservationProposal> proposals = api.ListDataReservationProposals(requestId);
    BOOST_FOREACH(CDataReservationProposal proposal, proposals)
    {
        Object obj;
        obj.push_back(Pair("proposalid", proposal.GetHash().ToString()));
        obj.push_back(Pair("requestid", proposal.nRequestHash.ToString()));
        obj.push_back(Pair("kbhrate", proposal.nKilobyteHourRate));
        obj.push_back(Pair("responsetime", DateTimeStrFormat(proposal.nResponseTime)));
        obj.push_back(Pair("paymentaddress", CBitcoinAddress(proposal.paymentPubKey.GetID()).ToString()));

        ret.push_back(obj);
    }

    return ret;
}

Value plmacceptproposal(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 1)
        throw runtime_error(
            "plmacceptproposal <proposalId>\n"
            "<proposalId> is the data reservation proposal hash");

    if (pwalletMain->IsLocked())
        throw JSONRPCError(RPC_WALLET_UNLOCK_NEEDED, "Error: Please enter the wallet passphrase with walletpassphrase first.");

    uint256 proposalId = uint256(params[0].get_str());
    Object ret;
    CPlumeApi api;
    string err;
    bool result = api.AcceptProposal(proposalId, err);
    ret.push_back(Pair("successful", result));
    if(!result)
        ret.push_back(Pair("errors", err));

    return ret;
}

Value plmlistplumes(const Array& params, bool fHelp)
{
    if (fHelp)
        throw runtime_error(
            "plmlistplumes\n"
            "Shows info about data plumes we are aware of.");

    CPlumeApi api;
    std::vector<CPlumeHeader> myPlumes = api.ListMyDataPlumes();
    std::vector<CPlumeHeader> publicPlumes = api.ListPublicDataPlumes();
    std::vector<CPlumeHeader> servicedPlumes = api.ListMyServicedDataPlumes();

    Array ret;
    BOOST_FOREACH(CPlumeHeader hdr, myPlumes)
    {
        Object obj;

        obj.push_back(Pair("plumetype", "myplume"));
        obj.push_back(Pair("plumeid", hdr.GetPlumeId().ToString()));
        obj.push_back(Pair("plumename", hdr.sPlumeName));

        ret.push_back(obj);
        //ret.push_back(Pair("myplume", obj));
    }

    BOOST_FOREACH(CPlumeHeader hdr, publicPlumes)
    {
        Object obj;

        obj.push_back(Pair("plumetype", "publicplume"));
        obj.push_back(Pair("plumeid", hdr.GetPlumeId().ToString()));
        obj.push_back(Pair("plumename", hdr.sPlumeName));

        ret.push_back(obj);
        //ret.push_back(Pair("publicplume", obj));
    }

    BOOST_FOREACH(CPlumeHeader hdr, servicedPlumes)
    {
        Object obj;

        obj.push_back(Pair("plumetype", "servicedplume"));
        obj.push_back(Pair("plumeid", hdr.GetPlumeId().ToString()));
        obj.push_back(Pair("plumename", hdr.sPlumeName));

        ret.push_back(obj);
        //ret.push_back(Pair("servicedplume", obj));
    }

    return ret;
}

Value plmplumedetails(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 1)
        throw runtime_error(
            "plmplumedetails <plumeId>\n"
            "<plumeId> is the plume id hash");

    uint256 plumeId = uint256(params[0].get_str());
    Object ret;
    CPlumeApi api;
    CPlumeHeader hdr = api.GetPlume(plumeId);
    ret.push_back(Pair("plumeid", hdr.GetPlumeId().ToString()));
    ret.push_back(Pair("plumename", hdr.sPlumeName));
    ret.push_back(Pair("attrone", hdr.sAttrOneName));
    ret.push_back(Pair("attrtwo", hdr.sAttrTwoName));
    ret.push_back(Pair("attrthree", hdr.sAttrThreeName));
    ret.push_back(Pair("originatorpeerid", hdr.nOriginatorPeerId.ToString()));
    ret.push_back(Pair("created", DateTimeStrFormat(hdr.nCreatedTime)));
    //ret.push_back(Pair("expires", DateTimeStrFormat(hdr.nExpirationDate)));
    ret.push_back(Pair("neuralnodesrequested", hdr.nNeuralNodesRequested));

    Array nn;
    BOOST_FOREACH(uint256 p, hdr.lNeuralNodes)
    {
        nn.push_back(p.ToString());
    }
    ret.push_back(Pair("neuralnodes", nn));

    return ret;
}

Value ailistactive(const Array& params, bool fHelp)
{
    if (fHelp)
        throw runtime_error("ailistactive lists active neural network jobs, training sessions and runs.\n");

    Object ret;
    Array jobs;
    ret.push_back(Pair("activejobs", jobs));

    return ret;
}

Value aitrainnn(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 3 || params.size() > 4)
        throw runtime_error(
            "aitrainnn <plumeid> <fields> <maxerror> <learnrate> [momentum]\n"
            "<plumeid> Data plume containing the training set\n"
            "<fields> List of field names in plume in order of inputs\n"
            "<maxerror> Max error\n"
            "<learnrate> Learning rate\n"
            "[momentum] Momentum value (optional)\n");

    Object ret;

    return ret;
}

Value airunnn(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 3)
        throw runtime_error(
            "airunnn <plumeid> <fields>\n"
            "<plumeid> Data plume containing the data set\n"
            "<fields> List of field names in plume in order of inputs/outputs\n");

    Object ret;

    return ret;
}

Value aicreatenn(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 7 || params.size() > 8)
        throw runtime_error(
            "aicreatenn <name> <inputs> <hidden> <outputs> <transfer function> <learning rule> <plume id> <runlocally>\n"
            "<name> is the name of this neural network\n"
            "<inputs> is number of input neurons\n"
            "<hidden> is number of hidden neurons\n"
            "<outputs> is number of output neurons\n"
            "<transfer function> is transfer function enum\n"
            "<learning rule> is rule enum\n"
            "<plume id> is id of the data plume to store state in\n"
            "<runlocally> is true or false to run this neural network on the local node\n");

    std::string networkName = params[0].get_str();
    int nInputs = params[1].get_int();
    int nHidden = params[2].get_int();
    int nOutputs = params[3].get_int();
    int nTransfer = params[4].get_int();
    int nLearning = params[5].get_int();
    std::string plumeHash = params[6].get_str();
    bool fRunLocally = params[7].get_bool();

    uint256 plumeId = uint256(plumeHash);

    AiLib api;
    CNeuralNetworkHeader hdr = api.CreateNeuralNetwork(nInputs, nHidden, nOutputs, networkName, fRunLocally, plumeId);
    uint256 networkId = hdr.GetNeuralNetworkId();

    Object ret;
    ret.push_back(Pair("networkid", networkId.ToString()));
    ret.push_back(Pair("networkname", hdr.sNetworkName));
    ret.push_back(Pair("inputs", hdr.nInputs));
    ret.push_back(Pair("hidden", hdr.nHidden));
    ret.push_back(Pair("outputs", hdr.nOutputs));
    ret.push_back(Pair("transfer", hdr.nTransferFunction));
    ret.push_back(Pair("learning", hdr.nLearningRule));
    ret.push_back(Pair("runlocal", hdr.fRunLocally));
    ret.push_back(Pair("plumeid", hdr.nStatePlumeId.ToString()));

    return ret;
}
