#include "init.h"
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
    
    cout << "金牌：" << GameRummy::getCardString(magic) << endl;

    using PCard = shared_ptr<Card>;
    int size = root["hand"][0]["card"].size();
    for (auto i = 0; i < size; i++)
    {
        int suit = root["hand"][0]["card"][i]["suit"].asInt();
        int rank = root["hand"][0]["card"][i]["rank"].asInt();
        const PCard &card = std::make_shared<Card>(static_cast<E_SUIT>(suit), rank, magic.rank_);
        hand.insert(std::make_pair(card->id_, card));
    }

    cout<<"初始手牌"<<endl;
    for (auto item : hand)
    {
        cout<<GameRummy::getCardString(*(item.second));
        if (item.second->rank_ == magic.rank_)
        {
            cout<<" 金牌";
        }
        cout<<endl; 
    }
    return true;
}