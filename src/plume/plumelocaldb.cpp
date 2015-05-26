// Copyright (c) 2014 The Sapience AIFX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include "plumecore.h"
#include "plumelocaldb.h"
#include "plumepeer.h"
#include "dht.h"
#include <leveldb/db.h>
#include <leveldb/write_batch.h>
#include <leveldb/slice.h>
#include "db.h"
#include "util.h"

CCriticalSection cs_localdb;

leveldb::DB *localDB = NULL;

namespace fs = boost::filesystem;

std::vector<CPlumePeer> ReadAllPlumePeers()
{
    std::vector<CPlumePeer> res;
    LOCK(cs_localdb);
    LocalDB ldb;
    if(!ldb.Open("cr"))
        return res;

    leveldb::Iterator* it = ldb.pdb->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
    {
        if(boost::starts_with(it->key().ToString(), "pp"))
        {
            std::string strValue = it->value().ToString();
            CPlumePeer plumepeer;
            CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
            ssValue >> plumepeer;
            res.push_back(plumepeer);
        }
    }
    return res;
}

uint CountDht()
{
    uint count = 0;
    LOCK(cs_localdb);
    LocalDB ldb;
    if(!ldb.Open("cr"))
        return count;

    leveldb::Iterator* it = ldb.pdb->NewIterator(leveldb::ReadOptions());
    for (it->SeekToFirst(); it->Valid(); it->Next())
        count = count + 1;

    return count;
}

size_t GetSizeBytesOnDisk()
{
    fs::path fullpath = GetDataDir() / "plume";
    size_t size=0;
    for(fs::recursive_directory_iterator it(fullpath);
        it!=fs::recursive_directory_iterator();
        ++it)
    {
        if(!is_directory(*it))
            size+=fs::file_size(*it);
    }

    return size;
}

bool LocalDB::Open(const char* pszMode)
{
    if (localDB)
    {
        pdb = localDB;
        return true;
    };

    bool fCreate = strchr(pszMode, 'c');

    fs::path fullpath = GetDataDir() / "plume";

    if (!fCreate
        && (!fs::exists(fullpath)
            || !fs::is_directory(fullpath)))
    {
        PlumeLog("Plume LocalDB::open() - DB does not exist.");
        return false;
    };

    leveldb::Options options;
    options.create_if_missing = fCreate;
    leveldb::Status s = leveldb::DB::Open(options, fullpath.string(), &localDB);

    if (!s.ok())
    {
        PlumeLog("Plume LocalDB::open() - Error opening db: %s.", s.ToString().c_str());
        return false;
    };

    pdb = localDB;

    return true;
};

class LocalDbBatchScanner : public leveldb::WriteBatch::Handler
{
public:
    std::string needle;
    bool* deleted;
    std::string* foundValue;
    bool foundEntry;

    LocalDbBatchScanner() : foundEntry(false) {}

    virtual void Put(const leveldb::Slice& key, const leveldb::Slice& value)
    {
        if (key.ToString() == needle)
        {
            foundEntry = true;
            *deleted = false;
            *foundValue = value.ToString();
        };
    };

    virtual void Delete(const leveldb::Slice& key)
    {
        if (key.ToString() == needle)
        {
            foundEntry = true;
            *deleted = true;
        };
    };
};

bool LocalDB::ScanBatch(const CDataStream& key, std::string* value, bool* deleted) const
{
    if (!activeBatch)
        return false;

    *deleted = false;
    LocalDbBatchScanner scanner;
    scanner.needle = key.str();
    scanner.deleted = deleted;
    scanner.foundValue = value;
    leveldb::Status s = activeBatch->Iterate(&scanner);
    if (!s.ok())
    {
        PlumeLog("Plume LocalDB ScanBatch error: %s", s.ToString().c_str());
        return false;
    };

    return scanner.foundEntry;
}

bool LocalDB::TxnBegin()
{
    if (activeBatch)
        return true;
    activeBatch = new leveldb::WriteBatch();
    return true;
};

bool LocalDB::TxnCommit()
{
    if (!activeBatch)
        return false;

    leveldb::WriteOptions writeOptions;
    writeOptions.sync = true;
    leveldb::Status status = pdb->Write(writeOptions, activeBatch);
    delete activeBatch;
    activeBatch = NULL;

    if (!status.ok())
    {
        PlumeLog("Plume LocalDB batch commit failure: %s", status.ToString().c_str());
        return false;
    };

    return true;
};

bool LocalDB::TxnAbort()
{
    delete activeBatch;
    activeBatch = NULL;
    return true;
};

bool LocalDB::ReadDhtValue(uint256 infohash, std::string& dhtvalue)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(sizeof(infohash) + 2);
    ssKey << 'd'; // dht
    ssKey << 'i'; // infohash
    ssKey << infohash;
    std::string strValue;

    bool readFromDb = true;
    if (activeBatch)
    {
        // -- check activeBatch first
        bool deleted = false;
        readFromDb = ScanBatch(ssKey, &strValue, &deleted) == false;
        if (deleted)
            return false;
    };

    if (readFromDb)
    {
        leveldb::Status s = pdb->Get(leveldb::ReadOptions(), ssKey.str(), &strValue);
        if (!s.ok())
        {
            if (s.IsNotFound())
                return false;
            PlumeLog("LocalDB read failure: %s", s.ToString().c_str());
            return false;
        };
    };

    try {
        CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
        ssValue >> dhtvalue;
    } catch (std::exception& e) {
        PlumeLog("LocalDB::ReadDhtValue() unserialize threw: %s.", e.what());
        return false;
    }

    return true;
};

bool LocalDB::WriteDhtValue(uint256 infohash, std::string dhtvalue)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(sizeof(infohash) + 2);
    ssKey << 'd'; // dht
    ssKey << 'i'; // infohash
    ssKey << infohash;
    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    ssValue.reserve(sizeof(dhtvalue));
    ssValue << dhtvalue;

    if (activeBatch)
    {
        activeBatch->Put(ssKey.str(), ssValue.str());
        return true;
    };

    leveldb::WriteOptions writeOptions;
    writeOptions.sync = true;
    leveldb::Status s = pdb->Put(writeOptions, ssKey.str(), ssValue.str());
    if (!s.ok())
    {
        PlumeLog("LocalDB write failure: %s", s.ToString().c_str());
        return false;
    };

    return true;
};

bool LocalDB::ExistsDhtInfoHash(uint256 infohash)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(sizeof(infohash)+2);
    ssKey << 'd'; // dht
    ssKey << 'i'; // infohash
    ssKey << infohash;
    std::string unused;

    if (activeBatch)
    {
        bool deleted;
        if (ScanBatch(ssKey, &unused, &deleted) && !deleted)
        {
            return true;
        };
    };

    leveldb::Status s = pdb->Get(leveldb::ReadOptions(), ssKey.str(), &unused);
    return s.IsNotFound() == false;
};

bool LocalDB::ReadPlumePeer(uint256 peerid, CPlumePeer& plumepeer)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(sizeof(peerid) + 2);
    ssKey << 'p'; // plume
    ssKey << 'p'; // peer
    ssKey << peerid;
    std::string strValue;

    bool readFromDb = true;
    if (activeBatch)
    {
        // -- check activeBatch first
        bool deleted = false;
        readFromDb = ScanBatch(ssKey, &strValue, &deleted) == false;
        if (deleted)
            return false;
    };

    if (readFromDb)
    {
        leveldb::Status s = pdb->Get(leveldb::ReadOptions(), ssKey.str(), &strValue);
        if (!s.ok())
        {
            if (s.IsNotFound())
                return false;
            PlumeLog("LocalDB read failure: %s", s.ToString().c_str());
            return false;
        };
    };

    try {
        CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
        ssValue >> plumepeer;
    } catch (std::exception& e) {
        PlumeLog("LocalDB::ReadPlumePeer() unserialize threw: %s.", e.what());
        return false;
    }

    return true;
};

bool LocalDB::WritePlumePeer(uint256 peerid, CPlumePeer& plumepeer)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(sizeof(peerid) + 2);
    ssKey << 'p'; // plume
    ssKey << 'p'; // peer
    ssKey << peerid;
    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    ssValue.reserve(sizeof(plumepeer));
    ssValue << plumepeer;

    if (activeBatch)
    {
        activeBatch->Put(ssKey.str(), ssValue.str());
        return true;
    };

    leveldb::WriteOptions writeOptions;
    writeOptions.sync = true;
    leveldb::Status s = pdb->Put(writeOptions, ssKey.str(), ssValue.str());
    if (!s.ok())
    {
        PlumeLog("LocalDB write failure: %s", s.ToString().c_str());
        return false;
    };

    return true;
};

bool LocalDB::ReadProposalQueue(std::map<uint256, CDataReservationProposal>& mapProposals)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(2);
    ssKey << 'm'; // map
    ssKey << 'p'; // proposals

    std::string strValue;

    bool readFromDb = true;
    if (activeBatch)
    {
        // -- check activeBatch first
        bool deleted = false;
        readFromDb = ScanBatch(ssKey, &strValue, &deleted) == false;
        if (deleted)
            return false;
    };

    if (readFromDb)
    {
        leveldb::Status s = pdb->Get(leveldb::ReadOptions(), ssKey.str(), &strValue);
        if (!s.ok())
        {
            if (s.IsNotFound())
                return false;
            PlumeLog("LocalDB read failure: %s", s.ToString().c_str());
            return false;
        };
    };

    try {
        CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
        ssValue >> mapProposals;
    } catch (std::exception& e) {
        PlumeLog("LocalDB::ReadProposalQueue() unserialize threw: %s.", e.what());
        return false;
    }

    return true;
};

bool LocalDB::WriteProposalQueue(std::map<uint256, CDataReservationProposal> mapProposals)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(2);
    ssKey << 'm'; // map
    ssKey << 'p'; // proposals

    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    ssValue.reserve(sizeof(mapProposals));
    ssValue << mapProposals;

    if (activeBatch)
    {
        activeBatch->Put(ssKey.str(), ssValue.str());
        return true;
    };

    leveldb::WriteOptions writeOptions;
    writeOptions.sync = true;
    leveldb::Status s = pdb->Put(writeOptions, ssKey.str(), ssValue.str());
    if (!s.ok())
    {
        PlumeLog("LocalDB write failure: %s", s.ToString().c_str());
        return false;
    };

    return true;
};

bool LocalDB::ReadProposalWaitingQueue(std::map<uint256, CDataReservationProposal>& mapProposals)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(2);
    ssKey << 'm'; // map
    ssKey << 'w'; // proposals

    std::string strValue;

    bool readFromDb = true;
    if (activeBatch)
    {
        // -- check activeBatch first
        bool deleted = false;
        readFromDb = ScanBatch(ssKey, &strValue, &deleted) == false;
        if (deleted)
            return false;
    };

    if (readFromDb)
    {
        leveldb::Status s = pdb->Get(leveldb::ReadOptions(), ssKey.str(), &strValue);
        if (!s.ok())
        {
            if (s.IsNotFound())
                return false;
            PlumeLog("LocalDB read failure: %s", s.ToString().c_str());
            return false;
        };
    };

    try {
        CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
        ssValue >> mapProposals;
    } catch (std::exception& e) {
        PlumeLog("LocalDB::ReadProposalWaitingQueue() unserialize threw: %s.", e.what());
        return false;
    }

    return true;
};

bool LocalDB::WriteProposalWaitingQueue(std::map<uint256, CDataReservationProposal> mapProposals)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(2);
    ssKey << 'm'; // map
    ssKey << 'w'; // proposals waiting

    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    ssValue.reserve(sizeof(mapProposals));
    ssValue << mapProposals;

    if (activeBatch)
    {
        activeBatch->Put(ssKey.str(), ssValue.str());
        return true;
    };

    leveldb::WriteOptions writeOptions;
    writeOptions.sync = true;
    leveldb::Status s = pdb->Put(writeOptions, ssKey.str(), ssValue.str());
    if (!s.ok())
    {
        PlumeLog("LocalDB write failure: %s", s.ToString().c_str());
        return false;
    };

    return true;
};

bool LocalDB::ReadReservationQueue(std::map<uint256, CDataReservationRequest>& mapRequests)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(2);
    ssKey << 'm'; // map
    ssKey << 'r'; // requests

    std::string strValue;

    bool readFromDb = true;
    if (activeBatch)
    {
        // -- check activeBatch first
        bool deleted = false;
        readFromDb = ScanBatch(ssKey, &strValue, &deleted) == false;
        if (deleted)
            return false;
    };

    if (readFromDb)
    {
        leveldb::Status s = pdb->Get(leveldb::ReadOptions(), ssKey.str(), &strValue);
        if (!s.ok())
        {
            if (s.IsNotFound())
                return false;
            PlumeLog("LocalDB read failure: %s", s.ToString().c_str());
            return false;
        };
    };

    try {
        CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
        ssValue >> mapRequests;
    } catch (std::exception& e) {
        PlumeLog("LocalDB::ReadReservationQueue() unserialize threw: %s.", e.what());
        return false;
    }

    return true;
};

bool LocalDB::WriteReservationQueue(std::map<uint256, CDataReservationRequest> mapRequests)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(2);
    ssKey << 'm'; // map
    ssKey << 'r'; // requests

    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    ssValue.reserve(sizeof(mapRequests));
    ssValue << mapRequests;

    if (activeBatch)
    {
        activeBatch->Put(ssKey.str(), ssValue.str());
        return true;
    };

    leveldb::WriteOptions writeOptions;
    writeOptions.sync = true;
    leveldb::Status s = pdb->Put(writeOptions, ssKey.str(), ssValue.str());
    if (!s.ok())
    {
        PlumeLog("LocalDB write failure: %s", s.ToString().c_str());
        return false;
    };

    return true;
};

bool LocalDB::ReadAcceptanceQueue(std::map<uint256, CDataProposalAcceptance>& mapAccepts)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(2);
    ssKey << 'm'; // map
    ssKey << 'a'; // accepts

    std::string strValue;

    bool readFromDb = true;
    if (activeBatch)
    {
        // -- check activeBatch first
        bool deleted = false;
        readFromDb = ScanBatch(ssKey, &strValue, &deleted) == false;
        if (deleted)
            return false;
    };

    if (readFromDb)
    {
        leveldb::Status s = pdb->Get(leveldb::ReadOptions(), ssKey.str(), &strValue);
        if (!s.ok())
        {
            if (s.IsNotFound())
                return false;
            PlumeLog("LocalDB read failure: %s", s.ToString().c_str());
            return false;
        };
    };

    try {
        CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
        ssValue >> mapAccepts;
    } catch (std::exception& e) {
        PlumeLog("LocalDB::ReadAcceptanceQueue() unserialize threw: %s.", e.what());
        return false;
    }

    return true;
};

bool LocalDB::WriteAcceptanceQueue(std::map<uint256, CDataProposalAcceptance> mapAccepts)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(2);
    ssKey << 'm'; // map
    ssKey << 'a'; // accepts

    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    ssValue.reserve(sizeof(mapAccepts));
    ssValue << mapAccepts;

    if (activeBatch)
    {
        activeBatch->Put(ssKey.str(), ssValue.str());
        return true;
    };

    leveldb::WriteOptions writeOptions;
    writeOptions.sync = true;
    leveldb::Status s = pdb->Put(writeOptions, ssKey.str(), ssValue.str());
    if (!s.ok())
    {
        PlumeLog("LocalDB write failure: %s", s.ToString().c_str());
        return false;
    };

    return true;
};

bool LocalDB::ReadPublicPlumeMap(std::map<uint256, CPlumeHeader>& mapPlumes)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(2);
    ssKey << 'm'; // map
    ssKey << '1'; // public plumes

    std::string strValue;

    bool readFromDb = true;
    if (activeBatch)
    {
        // -- check activeBatch first
        bool deleted = false;
        readFromDb = ScanBatch(ssKey, &strValue, &deleted) == false;
        if (deleted)
            return false;
    };

    if (readFromDb)
    {
        leveldb::Status s = pdb->Get(leveldb::ReadOptions(), ssKey.str(), &strValue);
        if (!s.ok())
        {
            if (s.IsNotFound())
                return false;
            PlumeLog("LocalDB read failure: %s", s.ToString().c_str());
            return false;
        };
    };

    try {
        CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
        ssValue >> mapPlumes;
    } catch (std::exception& e) {
        PlumeLog("LocalDB::ReadPublicPlumeMap() unserialize threw: %s.", e.what());
        return false;
    }

    return true;
};

bool LocalDB::WritePublicPlumeMap(std::map<uint256, CPlumeHeader> mapPlumes)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(2);
    ssKey << 'm'; // map
    ssKey << '1'; // public plumes

    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    ssValue.reserve(sizeof(mapPlumes));
    ssValue << mapPlumes;

    if (activeBatch)
    {
        activeBatch->Put(ssKey.str(), ssValue.str());
        return true;
    };

    leveldb::WriteOptions writeOptions;
    writeOptions.sync = true;
    leveldb::Status s = pdb->Put(writeOptions, ssKey.str(), ssValue.str());
    if (!s.ok())
    {
        PlumeLog("LocalDB write failure: %s", s.ToString().c_str());
        return false;
    };

    return true;
};

bool LocalDB::ReadMyPlumeMap(std::map<uint256, CPlumeHeader>& mapPlumes)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(2);
    ssKey << 'm'; // map
    ssKey << '2'; // my plumes

    std::string strValue;

    bool readFromDb = true;
    if (activeBatch)
    {
        // -- check activeBatch first
        bool deleted = false;
        readFromDb = ScanBatch(ssKey, &strValue, &deleted) == false;
        if (deleted)
            return false;
    };

    if (readFromDb)
    {
        leveldb::Status s = pdb->Get(leveldb::ReadOptions(), ssKey.str(), &strValue);
        if (!s.ok())
        {
            if (s.IsNotFound())
                return false;
            PlumeLog("LocalDB read failure: %s", s.ToString().c_str());
            return false;
        };
    };

    try {
        CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
        ssValue >> mapPlumes;
    } catch (std::exception& e) {
        PlumeLog("LocalDB::ReadMyPlumeMap() unserialize threw: %s.", e.what());
        return false;
    }

    return true;
};

bool LocalDB::WriteMyPlumeMap(std::map<uint256, CPlumeHeader> mapPlumes)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(2);
    ssKey << 'm'; // map
    ssKey << '2'; // my plumes

    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    ssValue.reserve(sizeof(mapPlumes));
    ssValue << mapPlumes;

    if (activeBatch)
    {
        activeBatch->Put(ssKey.str(), ssValue.str());
        return true;
    };

    leveldb::WriteOptions writeOptions;
    writeOptions.sync = true;
    leveldb::Status s = pdb->Put(writeOptions, ssKey.str(), ssValue.str());
    if (!s.ok())
    {
        PlumeLog("LocalDB write failure: %s", s.ToString().c_str());
        return false;
    };

    return true;
};

bool LocalDB::ReadServicedPlumeMap(std::map<uint256, CPlumeHeader>& mapPlumes)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(2);
    ssKey << 'm'; // map
    ssKey << '3'; // serviced plumes

    std::string strValue;

    bool readFromDb = true;
    if (activeBatch)
    {
        // -- check activeBatch first
        bool deleted = false;
        readFromDb = ScanBatch(ssKey, &strValue, &deleted) == false;
        if (deleted)
            return false;
    };

    if (readFromDb)
    {
        leveldb::Status s = pdb->Get(leveldb::ReadOptions(), ssKey.str(), &strValue);
        if (!s.ok())
        {
            if (s.IsNotFound())
                return false;
            PlumeLog("LocalDB read failure: %s", s.ToString().c_str());
            return false;
        };
    };

    try {
        CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
        ssValue >> mapPlumes;
    } catch (std::exception& e) {
        PlumeLog("LocalDB::ReadServicedPlumeMap() unserialize threw: %s.", e.what());
        return false;
    }

    return true;
};

bool LocalDB::WriteServicedPlumeMap(std::map<uint256, CPlumeHeader> mapPlumes)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(2);
    ssKey << 'm'; // map
    ssKey << '3'; // serviced plumes

    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    ssValue.reserve(sizeof(mapPlumes));
    ssValue << mapPlumes;

    if (activeBatch)
    {
        activeBatch->Put(ssKey.str(), ssValue.str());
        return true;
    };

    leveldb::WriteOptions writeOptions;
    writeOptions.sync = true;
    leveldb::Status s = pdb->Put(writeOptions, ssKey.str(), ssValue.str());
    if (!s.ok())
    {
        PlumeLog("LocalDB write failure: %s", s.ToString().c_str());
        return false;
    };

    return true;
};


bool LocalDB::WritePeerLastDhtUpdateMap(std::map<uint256, int64_t> mapPeerTimes)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(2);
    ssKey << 'd'; // map
    ssKey << 'p'; // my plumes

    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    ssValue.reserve(sizeof(mapPeerTimes));
    ssValue << mapPeerTimes;

    if (activeBatch)
    {
        activeBatch->Put(ssKey.str(), ssValue.str());
        return true;
    };

    leveldb::WriteOptions writeOptions;
    writeOptions.sync = true;
    leveldb::Status s = pdb->Put(writeOptions, ssKey.str(), ssValue.str());
    if (!s.ok())
    {
        PlumeLog("LocalDB write failure: %s", s.ToString().c_str());
        return false;
    };

    return true;
};

bool LocalDB::ReadPeerLastDhtUpdateMap(std::map<uint256, int64_t>& mapPeerTimes)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(2);
    ssKey << 'd'; // dht
    ssKey << 'p'; // peer update time

    std::string strValue;

    bool readFromDb = true;
    if (activeBatch)
    {
        // -- check activeBatch first
        bool deleted = false;
        readFromDb = ScanBatch(ssKey, &strValue, &deleted) == false;
        if (deleted)
            return false;
    };

    if (readFromDb)
    {
        leveldb::Status s = pdb->Get(leveldb::ReadOptions(), ssKey.str(), &strValue);
        if (!s.ok())
        {
            if (s.IsNotFound())
                return false;
            PlumeLog("LocalDB read failure: %s", s.ToString().c_str());
            return false;
        };
    };

    try {
        CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
        ssValue >> mapPeerTimes;
    } catch (std::exception& e) {
        PlumeLog("LocalDB::ReadPeerLastDhtUpdateMap() unserialize threw: %s.", e.what());
        return false;
    }

    return true;
};

bool LocalDB::WriteInfohashPeerList(uint256 infohash, std::vector<uint256> peers)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(sizeof(infohash) + 2);
    ssKey << 'd'; // dht
    ssKey << 'l'; // peer list
    ssKey << infohash;

    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    ssValue.reserve(sizeof(peers));
    ssValue << peers;

    if (activeBatch)
    {
        activeBatch->Put(ssKey.str(), ssValue.str());
        return true;
    };

    leveldb::WriteOptions writeOptions;
    writeOptions.sync = true;
    leveldb::Status s = pdb->Put(writeOptions, ssKey.str(), ssValue.str());
    if (!s.ok())
    {
        PlumeLog("LocalDB write failure: %s", s.ToString().c_str());
        return false;
    };

    return true;
};

bool LocalDB::ReadInfohashPeerList(uint256 infohash, std::vector<uint256>& peers)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(sizeof(infohash) + 2);
    ssKey << 'd'; // dht
    ssKey << 'l'; // peer list
    ssKey << infohash;

    std::string strValue;

    bool readFromDb = true;
    if (activeBatch)
    {
        // -- check activeBatch first
        bool deleted = false;
        readFromDb = ScanBatch(ssKey, &strValue, &deleted) == false;
        if (deleted)
            return false;
    };

    if (readFromDb)
    {
        leveldb::Status s = pdb->Get(leveldb::ReadOptions(), ssKey.str(), &strValue);
        if (!s.ok())
        {
            if (s.IsNotFound())
                return false;
            PlumeLog("LocalDB read failure: %s", s.ToString().c_str());
            return false;
        };
    };

    try {
        CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
        ssValue >> peers;
    } catch (std::exception& e) {
        PlumeLog("LocalDB::ReadInfohashPeerList() unserialize threw: %s.", e.what());
        return false;
    }

    return true;
};

bool LocalDB::WriteServicedPlumes(std::set<uint256> servicedPlumes)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(2);
    ssKey << 's'; // serviced
    ssKey << 'p'; // plumes

    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    ssValue.reserve(sizeof(servicedPlumes));
    ssValue << servicedPlumes;

    if (activeBatch)
    {
        activeBatch->Put(ssKey.str(), ssValue.str());
        return true;
    };

    leveldb::WriteOptions writeOptions;
    writeOptions.sync = true;
    leveldb::Status s = pdb->Put(writeOptions, ssKey.str(), ssValue.str());
    if (!s.ok())
    {
        PlumeLog("LocalDB write failure: %s", s.ToString().c_str());
        return false;
    };

    return true;
};

bool LocalDB::ReadServicedPlumes(std::set<uint256>& servicedPlumes)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(2);
    ssKey << 's'; // serviced
    ssKey << 'p'; // plumes

    std::string strValue;

    bool readFromDb = true;
    if (activeBatch)
    {
        // -- check activeBatch first
        bool deleted = false;
        readFromDb = ScanBatch(ssKey, &strValue, &deleted) == false;
        if (deleted)
            return false;
    };

    if (readFromDb)
    {
        leveldb::Status s = pdb->Get(leveldb::ReadOptions(), ssKey.str(), &strValue);
        if (!s.ok())
        {
            if (s.IsNotFound())
                return false;
            PlumeLog("LocalDB read failure: %s", s.ToString().c_str());
            return false;
        };
    };

    try {
        CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
        ssValue >> servicedPlumes;
    } catch (std::exception& e) {
        PlumeLog("LocalDB::ReadServicedPlumes() unserialize threw: %s.", e.what());
        return false;
    }

    return true;
};

bool LocalDB::WritePlumeInfohashes(uint256 plumeId, std::set<uint256> infohashes)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(sizeof(plumeId) + 2);
    ssKey << 'i'; // plume
    ssKey << 'i'; // infohashes
    ssKey << plumeId;

    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    ssValue.reserve(sizeof(infohashes));
    ssValue << infohashes;

    if (activeBatch)
    {
        activeBatch->Put(ssKey.str(), ssValue.str());
        return true;
    };

    leveldb::WriteOptions writeOptions;
    writeOptions.sync = true;
    leveldb::Status s = pdb->Put(writeOptions, ssKey.str(), ssValue.str());
    if (!s.ok())
    {
        PlumeLog("LocalDB write failure: %s", s.ToString().c_str());
        return false;
    };

    return true;
};

bool LocalDB::ReadPlumeInfohashes(uint256 plumeId, std::set<uint256>& infohashes)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(sizeof(plumeId) + 2);
    ssKey << 'i'; // plume
    ssKey << 'i'; // infohashes
    ssKey << plumeId;

    std::string strValue;

    bool readFromDb = true;
    if (activeBatch)
    {
        // -- check activeBatch first
        bool deleted = false;
        readFromDb = ScanBatch(ssKey, &strValue, &deleted) == false;
        if (deleted)
            return false;
    };

    if (readFromDb)
    {
        leveldb::Status s = pdb->Get(leveldb::ReadOptions(), ssKey.str(), &strValue);
        if (!s.ok())
        {
            if (s.IsNotFound())
                return false;
            PlumeLog("LocalDB read failure: %s", s.ToString().c_str());
            return false;
        };
    };

    try {
        CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
        ssValue >> infohashes;
    } catch (std::exception& e) {
        PlumeLog("LocalDB::ReadPlumeInfohashes() unserialize threw: %s.", e.what());
        return false;
    }

    return true;
};

bool LocalDB::ExistsPlumeInfohashes(uint256 plumeId)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(sizeof(plumeId)+2);
    ssKey << 'i'; // plume
    ssKey << 'i'; // infohashes
    ssKey << plumeId;
    std::string unused;

    if (activeBatch)
    {
        bool deleted;
        if (ScanBatch(ssKey, &unused, &deleted) && !deleted)
        {
            return true;
        };
    };

    leveldb::Status s = pdb->Get(leveldb::ReadOptions(), ssKey.str(), &unused);
    return s.IsNotFound() == false;
};

bool LocalDB::ExistsPlumePeer(uint256 peerid)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(sizeof(peerid)+2);
    ssKey << 'p'; // plume
    ssKey << 'p'; // peer
    ssKey << peerid;
    std::string unused;

    if (activeBatch)
    {
        bool deleted;
        if (ScanBatch(ssKey, &unused, &deleted) && !deleted)
        {
            return true;
        };
    };

    leveldb::Status s = pdb->Get(leveldb::ReadOptions(), ssKey.str(), &unused);
    return s.IsNotFound() == false;
};

bool LocalDB::ReadMyNeuralNetworks(std::map<uint256, CNeuralNetworkHeader>& mapNetworks)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(2);
    ssKey << 'm'; // map
    ssKey << 'n'; // my networks

    std::string strValue;

    bool readFromDb = true;
    if (activeBatch)
    {
        // -- check activeBatch first
        bool deleted = false;
        readFromDb = ScanBatch(ssKey, &strValue, &deleted) == false;
        if (deleted)
            return false;
    };

    if (readFromDb)
    {
        leveldb::Status s = pdb->Get(leveldb::ReadOptions(), ssKey.str(), &strValue);
        if (!s.ok())
        {
            if (s.IsNotFound())
                return false;
            PlumeLog("LocalDB read failure: %s", s.ToString().c_str());
            return false;
        };
    };

    try {
        CDataStream ssValue(strValue.data(), strValue.data() + strValue.size(), SER_DISK, CLIENT_VERSION);
        ssValue >> mapNetworks;
    } catch (std::exception& e) {
        PlumeLog("LocalDB::ReadMyNeuralNetworks() unserialize threw: %s.", e.what());
        return false;
    }

    return true;
};

bool LocalDB::WriteMyNeuralNetworks(std::map<uint256, CNeuralNetworkHeader> mapNetworks)
{
    if (!pdb)
        return false;

    CDataStream ssKey(SER_DISK, CLIENT_VERSION);
    ssKey.reserve(2);
    ssKey << 'm'; // map
    ssKey << 'n'; // my networks

    CDataStream ssValue(SER_DISK, CLIENT_VERSION);
    ssValue.reserve(sizeof(mapNetworks));
    ssValue << mapNetworks;

    if (activeBatch)
    {
        activeBatch->Put(ssKey.str(), ssValue.str());
        return true;
    };

    leveldb::WriteOptions writeOptions;
    writeOptions.sync = true;
    leveldb::Status s = pdb->Put(writeOptions, ssKey.str(), ssValue.str());
    if (!s.ok())
    {
        PlumeLog("LocalDB write failure: %s", s.ToString().c_str());
        return false;
    };

    return true;
};



// DHT
bool WriteDhtValue(uint256 infohash, std::string dhtvalue)
{
    LOCK(cs_localdb);
    LocalDB ldb;
    if (!ldb.Open("cw")
        || !ldb.TxnBegin())
        return false;

    if(ldb.WriteDhtValue(infohash, dhtvalue))
    {
        ldb.TxnCommit();
        return true;
    }
    else
    {
        return false;
    }
}

bool ReadDhtValue(uint256 infohash, std::string& dhtvalue)
{
    LocalDB ldb;
    if(!ldb.Open("r"))
        return false;

    if(ldb.ReadDhtValue(infohash, dhtvalue))
        return true;
    else
        return false;
}

bool HaveDhtValue(uint256 infohash)
{
    LocalDB ldb;
    if(!ldb.Open("r"))
        return false;

    if(ldb.ExistsDhtInfoHash(infohash))
        return true;
    else
        return false;
}

bool HavePlumeInfohashes(uint256 plumeId)
{
    LocalDB ldb;
    if(!ldb.Open("r"))
        return false;

    if(ldb.ExistsPlumeInfohashes(plumeId))
        return true;
    else
        return false;
}


// Peers
bool WritePlumePeer(uint256 infohash, CPlumePeer plumepeer)
{
    LOCK(cs_localdb);
    LocalDB ldb;
    if (!ldb.Open("cw")
        || !ldb.TxnBegin())
        return false;

    if(ldb.WritePlumePeer(infohash, plumepeer))
    {
        ldb.TxnCommit();
        return true;
    }
    else
    {
        return false;
    }
}

bool ReadPlumePeer(uint256 peerid, CPlumePeer& plumepeer)
{
    LocalDB ldb;
    if(!ldb.Open("r"))
        return false;

    if(ldb.ReadPlumePeer(peerid, plumepeer))
        return true;
    else
        return false;
}

bool HavePlumePeer(uint256 peerid)
{
    LocalDB ldb;
    if(!ldb.Open("r"))
        return false;

    if(ldb.ExistsPlumePeer(peerid))
        return true;
    else
        return false;
}

bool WriteProposalQueue(std::map<uint256, CDataReservationProposal> mapProposals)
{
    LOCK(cs_localdb);
    LocalDB ldb;
    if (!ldb.Open("cw")
        || !ldb.TxnBegin())
        return false;

    if(ldb.WriteProposalQueue(mapProposals))
    {
        ldb.TxnCommit();
        return true;
    }
    else
    {
        return false;
    }
}

bool ReadProposalQueue(std::map<uint256, CDataReservationProposal>& mapProposals)
{
    LocalDB ldb;
    if(!ldb.Open("r"))
        return false;

    if(ldb.ReadProposalQueue(mapProposals))
        return true;
    else
        return false;
}

bool WriteProposalWaitingQueue(std::map<uint256, CDataReservationProposal> mapProposals)
{
    LOCK(cs_localdb);
    LocalDB ldb;
    if (!ldb.Open("cw")
        || !ldb.TxnBegin())
        return false;

    if(ldb.WriteProposalWaitingQueue(mapProposals))
    {
        ldb.TxnCommit();
        return true;
    }
    else
    {
        return false;
    }
}

bool ReadProposalWaitingQueue(std::map<uint256, CDataReservationProposal>& mapProposals)
{
    LocalDB ldb;
    if(!ldb.Open("r"))
        return false;

    if(ldb.ReadProposalWaitingQueue(mapProposals))
        return true;
    else
        return false;
}

bool WriteReservationQueue(std::map<uint256, CDataReservationRequest> mapRequests)
{
    LOCK(cs_localdb);
    LocalDB ldb;
    if (!ldb.Open("cw")
        || !ldb.TxnBegin())
        return false;

    if(ldb.WriteReservationQueue(mapRequests))
    {
        ldb.TxnCommit();
        return true;
    }
    else
    {
        return false;
    }
}

bool ReadReservationQueue(std::map<uint256, CDataReservationRequest>& mapRequests)
{
    LocalDB ldb;
    if(!ldb.Open("r"))
        return false;

    if(ldb.ReadReservationQueue(mapRequests))
        return true;
    else
        return false;
}

bool WriteAcceptanceQueue(std::map<uint256, CDataProposalAcceptance> mapAccepts)
{
    LOCK(cs_localdb);
    LocalDB ldb;
    if (!ldb.Open("cw")
        || !ldb.TxnBegin())
        return false;

    if(ldb.WriteAcceptanceQueue(mapAccepts))
    {
        ldb.TxnCommit();
        return true;
    }
    else
    {
        return false;
    }
}

bool ReadAcceptanceQueue(std::map<uint256, CDataProposalAcceptance>& mapAccepts)
{
    LocalDB ldb;
    if(!ldb.Open("r"))
        return false;

    if(ldb.ReadAcceptanceQueue(mapAccepts))
        return true;
    else
        return false;
}

bool WritePublicPlumeMap(std::map<uint256, CPlumeHeader> mapPlumes)
{
    LOCK(cs_localdb);
    LocalDB ldb;
    if (!ldb.Open("cw")
        || !ldb.TxnBegin())
        return false;

    if(ldb.WritePublicPlumeMap(mapPlumes))
    {
        ldb.TxnCommit();
        return true;
    }
    else
    {
        return false;
    }
}

bool ReadPublicPlumeMap(std::map<uint256, CPlumeHeader>& mapPlumes)
{
    LocalDB ldb;
    if(!ldb.Open("r"))
        return false;

    if(ldb.ReadPublicPlumeMap(mapPlumes))
        return true;
    else
        return false;
}

bool WriteMyPlumeMap(std::map<uint256, CPlumeHeader> mapPlumes)
{
    LOCK(cs_localdb);
    LocalDB ldb;
    if (!ldb.Open("cw")
        || !ldb.TxnBegin())
        return false;

    if(ldb.WriteMyPlumeMap(mapPlumes))
    {
        ldb.TxnCommit();
        return true;
    }
    else
    {
        return false;
    }
}

bool ReadMyPlumeMap(std::map<uint256, CPlumeHeader>& mapPlumes)
{
    LocalDB ldb;
    if(!ldb.Open("r"))
        return false;

    if(ldb.ReadMyPlumeMap(mapPlumes))
        return true;
    else
        return false;
}

bool WriteServicedPlumeMap(std::map<uint256, CPlumeHeader> mapPlumes)
{
    LOCK(cs_localdb);
    LocalDB ldb;
    if (!ldb.Open("cw")
        || !ldb.TxnBegin())
        return false;

    if(ldb.WriteServicedPlumeMap(mapPlumes))
    {
        ldb.TxnCommit();
        return true;
    }
    else
    {
        return false;
    }
}

bool ReadServicedPlumeMap(std::map<uint256, CPlumeHeader>& mapPlumes)
{
    LocalDB ldb;
    if(!ldb.Open("r"))
        return false;

    if(ldb.ReadServicedPlumeMap(mapPlumes))
        return true;
    else
        return false;
}

bool WriteServicedPlumes(std::set<uint256> servicedPlumes)
{
    LOCK(cs_localdb);
    LocalDB ldb;
    if (!ldb.Open("cw")
        || !ldb.TxnBegin())
        return false;

    if(ldb.WriteServicedPlumes(servicedPlumes))
    {
        ldb.TxnCommit();
        return true;
    }
    else
    {
        return false;
    }
}

bool ReadServicedPlumes(std::set<uint256>& servicedPlumes)
{
    LocalDB ldb;
    if(!ldb.Open("r"))
        return false;

    if(ldb.ReadServicedPlumes(servicedPlumes))
        return true;
    else
        return false;
}

bool WritePlumeInfohashes(uint256 plumeId, std::set<uint256> infohashes)
{
    LOCK(cs_localdb);
    LocalDB ldb;
    if (!ldb.Open("cw")
        || !ldb.TxnBegin())
        return false;

    if(ldb.WritePlumeInfohashes(plumeId, infohashes))
    {
        ldb.TxnCommit();
        return true;
    }
    else
    {
        return false;
    }
}

bool ReadPlumeInfohashes(uint256 plumeId, std::set<uint256>& infohashes)
{
    LocalDB ldb;
    if(!ldb.Open("r"))
        return false;

    if(ldb.ReadPlumeInfohashes(plumeId, infohashes))
        return true;
    else
        return false;
}

bool ReadPeerLastDhtUpdateMap(std::map<uint256, int64_t>& mapPeerTimes)
{
    LocalDB ldb;
    if(!ldb.Open("r"))
        return false;

    if(ldb.ReadPeerLastDhtUpdateMap(mapPeerTimes))
        return true;
    else
        return false;
}

bool TryOpenLocalDB()
{
    LOCK(cs_localdb);
    LocalDB ldb;
    if(!ldb.Open("cr")) // create if doesn't exist, read mode
        return false;

    return true;
}


bool WriteMyNeuralNetworks(std::map<uint256, CNeuralNetworkHeader> mapNetworks)
{
    LOCK(cs_localdb);
    LocalDB ldb;
    if (!ldb.Open("cw")
        || !ldb.TxnBegin())
        return false;

    if(ldb.WriteMyNeuralNetworks(mapNetworks))
    {
        ldb.TxnCommit();
        return true;
    }
    else
    {
        return false;
    }
}

bool ReadMyNeuralNetworks(std::map<uint256, CNeuralNetworkHeader>& mapNetworks)
{
    LocalDB ldb;
    if(!ldb.Open("r"))
        return false;

    if(ldb.ReadMyNeuralNetworks(mapNetworks))
        return true;
    else
        return false;
}
