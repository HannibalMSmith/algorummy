#include <iostream>
#include <fstream>
#include <memory>
#include <map>
#include <vector>
#include <algorithm>
#include "RummyConfig.h"
#include "json/json.h"
#include "gamerummy.h"

int main(int argc, char *argv[])
{
    using std::cout;
    using std::endl;
    using std::shared_ptr;
    using std::map;
    using std::vector;

    if (argc < 2)
    {   
        cout <<"Rummy match:" << Rummy_VERSION_MAJOR << "." << Rummy_VERSION_MINOR <<"."\
        << Rummy_VERSION_MAJOR << "." << Rummy_VERSION_MAJOR << endl;
        cout << "Usage: " << argv[0] << " PathToScript(default ./config/script.json)" << endl;
    }
    const char *scriptfile = "./config/script.json";
    if (argc > 1)
    {
        scriptfile = argv[1];
    }

    std::ifstream ifs;
    ifs.open(scriptfile);
    Json::CharReaderBuilder builder;
    builder["collectComments"] = true;
    Json::Value root;
    JSONCPP_STRING errs;
    if (!parseFromStream(builder, ifs, &root, &errs)) 
    {
         cout << "script file format error" << endl;
         return 1;
    }

    Card magic;
    magic.suit_ = static_cast<E_SUIT>(root["magiccard"]["suit"].asInt());
    magic.rank_ = root["magiccard"]["rank"].asInt();
    cout << "magic suit " << magic.suit_ << " rank "<< magic.rank_ <<endl;

    using PCard = shared_ptr<Card>;
    map<int, PCard> hand;
    int size = root["hand"][0]["card"].size();
    for (auto i = 0; i < size; i++)
    {
        int suit = root["hand"][0]["card"][i]["suit"].asInt();
        int rank = root["hand"][0]["card"][i]["rank"].asInt();
        PCard card = std::make_shared<Card>(static_cast<E_SUIT>(suit), rank, rank == magic.rank_);
        hand.insert(std::make_pair(card->id_,card));
    }

    for(auto item: hand)
    {
        cout <<"card id "<<item.second->id_ << " suit " << item.second->suit_ <<" rank " << item.second->rank_ << endl;
    }

    Mgr mgr;
    std::vector<CardGroup> groupList;
    mgr.match(hand, groupList);

    return 0;
}