#include "init.h"
#include "RummyConfig.h"
#include <iostream>
#include <fstream>
#include <map>
#include <memory>
#include "json/json.h"
using std::cout;
using std::endl;
using std::shared_ptr;
using std::map;

const char *scriptPrompt(int argc, char **argv)
{
    if (argc < 2)
    {
        cout << "Rummy" << Rummy_VERSION_MAJOR << "." << Rummy_VERSION_MINOR << "."
             << Rummy_VERSION_MAJOR << "." << Rummy_VERSION_MAJOR << endl;
        cout << "Usage: " << argv[0] << " PathToScript(default ./config/script.json)" << endl;
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
        cout << "script file format error" << endl;
        return false;
    }

    magic.suit_ = static_cast<E_SUIT>(root["magiccard"]["suit"].asInt());
    magic.rank_ = root["magiccard"]["rank"].asInt();
    cout << "magic " << magic.suit_ << ":" << magic.rank_ << endl;

    using PCard = shared_ptr<Card>;
    int size = root["hand"][0]["card"].size();
    for (auto i = 0; i < size; i++)
    {
        int suit = root["hand"][0]["card"][i]["suit"].asInt();
        int rank = root["hand"][0]["card"][i]["rank"].asInt();
        PCard card = std::make_shared<Card>(static_cast<E_SUIT>(suit), rank, rank == magic.rank_);
        hand.insert(std::make_pair(card->id_, card));
    }

    for (auto item : hand)
    {
        cout << "card id " << item.second->id_ << " suit " << item.second->suit_ << " rank " << item.second->rank_ << endl;
    }
    return true;
}