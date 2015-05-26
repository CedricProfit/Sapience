// Copyright (c) 2014 The Sapience AIFX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef IBTP_H
#define IBTP_H

#include "util.h"
#include <map>

using namespace std;

struct SChain
{
public:
    std::string sChainName;
    std::string sCurrencyCode;
    unsigned char pchMessageOne;
    unsigned char pchMessageTwo;
    unsigned char pchMessageThree;
    unsigned char pchMessageFour;

    SChain()
    {

    }

    SChain(std::string sName, std::string sCode, unsigned char cOne, unsigned char cTwo, unsigned char cThree, unsigned char cFour)
    {
        sChainName = sName;
        sCurrencyCode = sCode;
        pchMessageOne = cOne;
        pchMessageTwo = cTwo;
        pchMessageThree = cThree;
        pchMessageFour = cFour;
    }
};

class CIbtp
{
public:
    //std::map<std::string, std::vector<unsigned char[4]> > mapBlockchainMessageStart;
    std::vector<SChain> vChains;

    CIbtp()
    {
        LoadMsgStart();
    }

    void LoadMsgStart();
    bool IsIbtpChain(unsigned char msgStart[], std::string& chainName);


};

#endif // IBTP_H
