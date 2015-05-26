// Copyright (c) 2014 The Sapience AIFX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ibtp.h"
#include "main.h"
#include "util.h"
#include <map>

using namespace std;

/*unsigned char ltctn[4] = { 0xfc, 0xc1, 0xb7, 0xdc };
unsigned char saptn[4] = { 0x11, 0x05, 0x0b, 0x07 };
unsigned char sdcmn[4] = { 0xfa, 0xf2, 0xef, 0xb4 };
unsigned char sdctn[4] = { 0x07, 0x11, 0x05, 0x0b };*/

void CIbtp::LoadMsgStart()
{
    vChains.push_back(SChain("LiteCoin Testnet", "LTC", 0xfc, 0xc1, 0xb7, 0xdc));
    vChains.push_back(SChain("Shadow Testnet", "SDC", 0x07, 0x11, 0x05, 0x0b));
    vChains.push_back(SChain("Sapience AIFX", "XAI", 0xfa, 0xb2, 0xef, 0xf2));
    vChains.push_back(SChain("Sapience AIFX Testnet", "XAI", 0x11, 0x05, 0x0b, 0x07));

    //unsigned char ltc[4] = { 0xfb, 0xc0, 0xb6, 0xdb };
    //std::vector<unsigned char[4]> vltc;
    //vltc.push_back(ltc);
    //mapBlockchainMessageStart.insert(make_pair("LiteCoin", vltc ));
    /*unsigned char doge[4] = { 0xc0, 0xc0, 0xc0, 0xc0 };
    mapBlockchainMessageStart.insert(make_pair("DogeCoin", &doge));
    unsigned char sap[4] = { 0xfa, 0xb2, 0xef, 0xf2 };
    mapBlockchainMessageStart.insert(make_pair("Sapience AIFX", &sap));
    unsigned char ltctn[4] = { 0xfc, 0xc1, 0xb7, 0xdc };
    mapBlockchainMessageStart.insert(make_pair("LiteCoin Testnet", &ltctn));
    unsigned char saptn[4] = { 0x11, 0x05, 0x0b, 0x07 };
    mapBlockchainMessageStart.insert(make_pair("Sapience AIFX Testnet", &saptn));
    unsigned char sdcmn[4] = { 0xfa, 0xf2, 0xef, 0xb4 };
    mapBlockchainMessageStart.insert(make_pair("Shadow", &sdcmn));
    unsigned char sdctn[4] = { 0x07, 0x11, 0x05, 0x0b };
    mapBlockchainMessageStart.insert(make_pair("Shadow Testnet", &sdctn));*/
}

bool CIbtp::IsIbtpChain(unsigned char msgStart[], std::string& chainName)
{
    bool bFound = false;
    BOOST_FOREACH(SChain p, vChains)
    {
        unsigned char pchMsg[4] = { p.pchMessageOne, p.pchMessageTwo, p.pchMessageThree, p.pchMessageFour };
        if(memcmp(msgStart, pchMessageStart, sizeof(pchMessageStart)) != 0)
        {
            if(memcmp(msgStart, pchMsg, sizeof(pchMsg)) == 0)
            {
                bFound = true;
                chainName = p.sChainName;
                printf("Found IBTP chain: %s\n", p.sChainName.c_str());
            }
        }
    }

    /*BOOST_FOREACH(PAIRTYPE(const std::string, std::vector<unsigned char[4]>)& p, mapBlockchainMessageStart)
    {
        if(memcmp(msgStart, pchMessageStart, sizeof(pchMessageStart)) != 0)
        {
            if(memcmp(msgStart, p.second[0], sizeof(p.second[0])) == 0) //sizeof(*(p.second))) == 0)
            {
                bFound = true;
                chainName = p.first;
                printf("Found IBTP chain: %s\n", p.first.c_str());
            }
        }
    }*/
    /*
            if(memcmp(msgStart, ltctn, sizeof(ltctn)) == 0)
            {
                // found a matching blockchain
                bFound = true;
                //chainName = p.first;
                chainName = "LiteCoin Testnet";
                printf("Found IBTP chain: %s\n", chainName.c_str());
                //break;
            }
            else if(memcmp(msgStart, saptn, sizeof(saptn)) == 0)
            {
                bFound = true;
                chainName = "Sapience AIFX Testnet";
                printf("Found IBTP chain: %s\n", chainName.c_str());
                //break;
            }
            else if(memcmp(msgStart, sdcmn, sizeof(sdcmn)) == 0)
            {
                bFound = true;
                chainName = "Shadow Testnet";
                printf("Found IBTP chain: %s\n", chainName.c_str());
                //break;
            }
            else if(memcmp(msgStart, sdctn, sizeof(sdctn)) == 0)
            {
                bFound = true;
                chainName = "Shadow Testnet";
                printf("Found IBTP chain: %s\n", chainName.c_str());
                //break;
            }
        */
    return bFound;
}
