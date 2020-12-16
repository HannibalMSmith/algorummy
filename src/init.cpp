#include "init.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <map>
#include <memory>
#include <vector>
#include <random>
#include <algorithm>
#include "json/json.h"

using std::cout;
using std::endl;
using std::map;
using std::shared_ptr;
using std::vector;

const char *scriptPrompt(int argc, char **argv, int &numCount)
{
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " PathToScript(default ./data/script.json)" << endl;
    }
    if (argc >= 2)
    {
        numCount = atoi(argv[1]);
    }
    const char *scriptfile = "./data/script.json";
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

void randomInit(std::map<int, std::shared_ptr<Card>> &hand, Card &special)
{
    hand.clear();
    vector<int> ranks{-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    vector<int> suits{0, 1, 2, 3, 4};
    std::random_device r;
    std::default_random_engine e(r());
    std::shuffle(ranks.begin(), ranks.end(), e);
    special.rank_ = ranks[0];
    std::shuffle(suits.begin(), suits.end(), e);
    special.suit_ = static_cast<E_SUIT>(suits[0]);
    if (special.suit_ == e_joker)
    {
        special.rank_ = 0;
    }

    int pack = 2;
    vector<shared_ptr<Card>> cards;
    for (size_t i = 0; i < pack; i++)
    {
        for (auto suit = 1; suit <= 4; ++suit)
        {
            for (auto rank = 1; rank <= 13; ++rank)
            {
                auto card = std::make_shared<Card>(static_cast<E_SUIT>(suit), rank, special.rank_);
                cards.push_back(card);
            }
        }
        auto redjoker = std::make_shared<Card>(e_joker, 0, special.rank_);
        cards.push_back(redjoker);
        auto blackjoker = std::make_shared<Card>(e_joker, -1, special.rank_);
        cards.push_back(blackjoker);
    }
    std::shuffle(cards.begin(), cards.end(), e);
    
    int numCards = 13;
    for (auto i = 0; i < numCards; i++)
    {
        hand[i] = cards[i];
    }
}
