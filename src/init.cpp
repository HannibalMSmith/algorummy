#include "init.h"
#include <iostream>
#include <fstream>
#include <map>
#include <memory>
#include "json/json.h"
#include <stdlib.h>

using std::cout;
using std::endl;
using std::shared_ptr;
using std::map;

const char *scriptPrompt(int argc, char **argv, int &numCount)
{
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " PathToScript(default ./config/script.json)" << endl;
    }
    if(argc >= 2)
    {
        numCount = atoi(argv[1]);
    }
    const char *scriptfile = "./config/script.json";
    if (argc > 1)
    {
        scriptfile = argv[1];
    }
    return scriptfile;
}

bool init(const char *script, std::map<int, std::shared_ptr<Card>> &hand, Card &magic)
{
    hand.clear();

    std::ifstream ifs;
    ifs.open(script);
    Json::CharReaderBuilder builder;
    builder["collectComments"] = true;
    Json::Value root;
    JSONCPP_STRING errs;
    if (!parseFromStream(builder, ifs, &root, &errs))
    {
        cout << "Please check the json file format" << endl;
        return false;
    }

    magic.suit_ = static_cast<E_SUIT>(root["magiccard"]["suit"].asInt());
    magic.rank_ = root["magiccard"]["rank"].asInt();

    using PCard = shared_ptr<Card>;
    int size = root["hand"][0]["card"].size();
    for (auto i = 0; i < size; i++)
    {
        int suit = root["hand"][0]["card"][i]["suit"].asInt();
        int rank = root["hand"][0]["card"][i]["rank"].asInt();
        const PCard &card = std::make_shared<Card>(static_cast<E_SUIT>(suit), rank, magic.rank_);
        hand.insert(std::make_pair(card->id_, card));
    }
    
    return true;
}


void randomInit(std::map<int, std::shared_ptr<Card>> &hand, Card &magic)
{



}