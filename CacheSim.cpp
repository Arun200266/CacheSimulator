/**
 * @ Author: Manish, Alan, Arun
 * @ Create Time: 24-03-2021 22:05:03
 * @ Modified by: Manish
 * @ Modified time: 24-03-2021 22:53:32
 * @ Description: Cache Simulator
 */

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <vector>
#include <cmath>
#include <map>
using namespace std;

string HextoBin(string str)
{
    string b;
    for (char ch : str)
    {
        switch (ch)
        {
        case '0':
            b += "0000";
            break;
        case '1':
            b += "0001";
            break;
        case '2':
            b += "0010";
            break;
        case '3':
            b += "0011";
            break;
        case '4':
            b += "0100";
            break;
        case '5':
            b += "0101";
            break;
        case '6':
            b += "0110";
            break;
        case '7':
            b += "0111";
            break;
        case '8':
            b += "1000";
            break;
        case '9':
            b += "1001";
            break;
        case 'a':
            b += "1010";
            break;
        case 'b':
            b += "1011";
            break;
        case 'c':
            b += "1100";
            break;
        case 'd':
            b += "1101";
            break;
        case 'e':
            b += "1110";
            break;
        case 'f':
            b += "1111";
            break;
        default:
            break;
        }
    }
    return b;
}

int BintoDec(string str)
{
    int n = 0;
    for (char x : str)
    {
        n <<= 1;
        n += x - '0';
    }
    return n;
}

string DecToBin(int n, int k)
{
    string s;
    while (k--)
    {
        char b = (n & 1) + '0';
        s.insert(0, 1, b);
        n >>= 1;
    }
    return s;
}

struct cacheBlock
{
    string tag;
    int valid, dirty;
    cacheBlock()
    {
        tag = "";
        valid = 0;
        dirty = 0;
    }
};

struct Stats
{
    int numReadAccess = 0;
    int numWriteAccess = 0;
    int numCacheMiss = 0;
    int numCompMiss = 0;
    int numCapMiss = 0;
    int numConMiss = 0;
    int numReadMiss = 0;
    int numWriteMiss = 0;
    int numCacheAccess = 0;
    int numDirtyEvict = 0;

    void PrintStats(fstream &) const;
};

void Stats::PrintStats(fstream &f) const
{
    f << "Statistics:\n\n";
    f << "Number of Cache Accesses       : " << numCacheAccess << endl;
    f << "Number of Read Accesses        : " << numReadAccess << endl;
    f << "Number of Write Accesses       : " << numWriteAccess << endl;
    f << "Number of Cache Misses         : " << numCacheMiss << endl;
    f << "Number of Compulsory Misses    : " << numCompMiss << endl;
    f << "Number of Capacity Misses      : " << numCapMiss << endl;
    f << "Number of Conflict Misses      : " << numConMiss << endl;
    f << "Number of Read Misses          : " << numReadMiss << endl;
    f << "Number of Write Misses         : " << numWriteMiss << endl;
    f << "Number of Dirty Blocks Evicted : " << numDirtyEvict << endl;
}
class cache
{
    int cacheSize;
    int blockSize;
    int associativity;
    int repPolicy;

    int numBlock;
    int numSet;

    Stats CacheStats;

    vector<list<cacheBlock>> C;
    map<string, bool> traceAddr;

    vector<vector<bool>> info; // For Pseudo LRU

    void LRU(string, bool); // #Proud
    void Random(string, bool);
    void PseudoLRU(string, bool);

    string getTag(string);
    int getSetNo(string);
    string getmemAddress(string);

public:
    cache(int, int, int, int);
    void readwrite(string);
    void PrintDetails(string);
    void ReadFromFile(string);
};

cache::cache(int cSz, int bSz, int ass, int rpP)
{
    cacheSize = cSz;
    blockSize = bSz;
    associativity = ass;
    repPolicy = rpP;

    numBlock = cacheSize / blockSize;

    if (associativity == 0)
        numSet = 1;
    else
        numSet = numBlock / associativity;

    C = vector<list<cacheBlock>>(numSet);

    int numBlocksPerSet = ((associativity == 0) ? numBlock : associativity);

    for (int i = 0; i < numSet; i++)
        for (int j = 0; j < numBlocksPerSet; j++)
            C[i].push_back(cacheBlock());

    info = vector<vector<bool>>(numSet, vector<bool>(numBlocksPerSet, false));
}

string cache::getTag(string s)
{
    int blockoffset = (int)log2(blockSize);
    int setbits = (int)log2(numSet);
    int tagbits = s.size() - setbits - blockoffset;
    return s.substr(0, tagbits);
}

int cache::getSetNo(string s)
{
    int blockoffset = (int)log2(blockSize);
    int setbits = (int)log2(numSet);
    int tagbits = s.size() - setbits - blockoffset;
    int setnum = BintoDec(s.substr(tagbits, setbits));
    return setnum;
}

string cache::getmemAddress(string s)
{
    int blockoffset = (int)log2(blockSize);
    return s.substr(0, s.size() - blockoffset);
}

void cache::readwrite(string bin)
{
    string s = bin.substr(1, bin.length() - 1);
    bool write = bin[0] - '0';
    string addr = getmemAddress(s);

    if (traceAddr.find(addr) == traceAddr.end())
    {
        traceAddr[addr] = true;
        CacheStats.numCompMiss++;
    }

    CacheStats.numCacheAccess++;
    (write ? CacheStats.numWriteAccess++ : CacheStats.numReadAccess++);

    if (repPolicy == 0)
        Random(s, write);
    if (repPolicy == 1)
        LRU(s, write);
    if (repPolicy == 2)
        PseudoLRU(s, write);
}

void cache::Random(string s, bool write)
{
    string tag = getTag(s);
    int set = getSetNo(s);
    bool flag = false;
    auto block = C[set].begin();

    for (block = C[set].begin(); block != C[set].end(); ++block)
        if (block->tag == tag) //cache hit
        {
            flag = true;
            break;
        }

    if (!flag)
    {
        for (block = C[set].begin(); block != C[set].end(); ++block)
            if (block->valid == 0) //filling empty block
            {
                flag = true;
                break;
            }

        if (!flag)
        {
            int x = rand() % associativity;

            //Picking Random Block
            block = C[set].begin();
            advance(block, x);

            //checking if dirty block and evicting
            if (block->dirty)
                CacheStats.numDirtyEvict++;

            CacheStats.numConMiss++;
            if (associativity == 0)
                CacheStats.numCapMiss++;
        }

        CacheStats.numCacheMiss++;
        (write ? CacheStats.numWriteMiss++ : CacheStats.numReadMiss++);
    }

    block->tag = tag;
    block->valid = 1;
    if (write)
        block->dirty = 1;
}

void cache::LRU(string s, bool write)
{
    string tag = getTag(s);
    int setNum = getSetNo(s);
    bool hit = false;
    for (auto block = C[setNum].begin(); block != C[setNum].end(); block++)
    {
        if (block->tag == tag)
        {
            hit = true;
            auto blockcopy = *block;
            C[setNum].erase(block);
            C[setNum].push_front(blockcopy);
            break;
        }
    }

    if (!hit)
    {
        cacheBlock newBlock;
        newBlock.valid = 1;
        newBlock.tag = tag;
        if (write)
            newBlock.dirty = 1;

        bool free = false;
        for (auto block = C[setNum].begin(); block != C[setNum].end(); block++)
        {
            if (block->valid == 0)
            {
                free = true;
                C[setNum].erase(block);
                C[setNum].push_front(newBlock);
                break;
            }
        }
        if (!free)
        {
            CacheStats.numConMiss++;
            if (associativity == 0)
                CacheStats.numCapMiss++;

            auto lastBlock = C[setNum].end();
            lastBlock--;
            if (lastBlock->dirty == 1)
                CacheStats.numDirtyEvict++;
            C[setNum].erase(lastBlock);
            C[setNum].push_front(newBlock);
        }
        CacheStats.numCacheMiss++;
        (write ? CacheStats.numWriteMiss++ : CacheStats.numReadMiss++);
    }
}

void cache::PseudoLRU(string s, bool write)
{
    string tag = getTag(s);
    int set = getSetNo(s);

    /* cache hit? */
    bool hit = false;
    auto block = C[set].begin();
    int i = 0;
    for (block = C[set].begin(); block != C[set].end(); block++)
    {
        i++;
        if (block->tag == tag)
        {
            hit = true;
            break;
        }
    }

    int n = associativity == 0 ? numBlock : associativity;

    /* CACHE HIT! */
    if (hit)
    {
        if (write)
            block->dirty = true;

        // update PLRU information.
        string s = DecToBin(i, log2(n)); // i is the location of the block in the set, s is i in k-bit binary; k = log2(n)
        int j = 1;
        for (auto c : s)
        {
            bool b = !(c - '0');
            info[set][j] = b;
            j = 2 * j + info[set][j];
        }
    }

    /* CACHE MISS! */
    else
    {
        // locate the victim block.
        int loc = 0, i = 1;
        while (i < n)
        {
            loc = 2 * loc + info[set][i];
            i = 2 * i + info[set][i];
        } // idx = block location in the set
        auto block = C[set].begin();
        advance(block, loc); // block points to the victim block

        // is the victim block non-empty? if yes, evict.
        if (block->valid)
        {
            CacheStats.numConMiss++;
            if (associativity == 0)
                CacheStats.numCapMiss++;
            if (block->dirty)
                CacheStats.numDirtyEvict++;
        }

        // read/write data into the block.
        block->tag = tag;
        block->valid = true;
        block->dirty = write ? true : false;

        // update PLRU information.
        i = 1;
        while (i < n)
        {
            info[set][i] = !info[set][i];
            i = 2 * i + info[set][i];
        }

        CacheStats.numCacheMiss++;
        (write ? CacheStats.numWriteMiss++ : CacheStats.numReadMiss++);
    }
}

void cache::ReadFromFile(string filename)
{
    fstream f;
    f.open(filename, ios::in);
    if (!f)
    {
        cerr << "Error - File not found!\n";
        exit(0);
    }
    string s, bin;
    if (f.is_open())
    {
        while (getline(f, s))
        {
            bin = HextoBin(s);
            readwrite(bin);
        }
    }
}

void cache::PrintDetails(string filename)
{
    fstream f;
    f.open(filename, ios::out);
    if (!f)
    {
        cerr << "Output File not created!!\n";
        exit(0);
    }
    f << "Cache Details:\n\n";
    f << "Cache Size         : " << cacheSize << endl;
    f << "Block Size         : " << blockSize << endl;
    f << "Type of Cache      : " << ((associativity == 0) ? "Fully Associative" : (associativity == 1) ? "Direct Mapped"
                                                                                                       : "Set-Associative")
      << endl;
    f << "Replacement Policy : " << ((repPolicy == 0) ? "Random" : (repPolicy == 1) ? "LRU"
                                                                                    : "Pseudo-LRU")
      << endl;
    f << endl
      << endl;
    CacheStats.PrintStats(f);
    f.close();
}

int main()
{
    int cacheSize;
    int blockSize;
    int associativity;
    int repPolicy;
    string inputFile;

    cout << "Enter the Cache size : ";
    cin >> cacheSize;
    cout << "Enter the Block Size : ";
    cin >> blockSize;
    cout << "Enter the Associativity : ";
    cin >> associativity;
    cout << "Enter the Replacement Policy : ";
    cin >> repPolicy;
    cout << endl;
    cache C(cacheSize, blockSize, associativity, repPolicy);
    cout << "Enter the Input File name : ";
    cin >> inputFile;
    C.ReadFromFile(inputFile);
    C.PrintDetails("output.txt");
    return 0;
}
