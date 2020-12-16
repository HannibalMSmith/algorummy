#include <iostream>
#include <memory>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include "gamerummy.h"
#include "init.h"

int main(int argc, char **argv)
{
    using std::cout;
    using std::endl;
    using std::map;
    using std::shared_ptr;
    using std::string;
    using std::vector;
    using PCard = shared_ptr<Card>;

    
    Card special;
    map<int, PCard> hand;
    int numCount = 0;
    string script = scriptPrompt(argc, argv, numCount);
    if (numCount == 0)
    {
        if (!init(script.c_str(), hand, special))
            return 1;
    }

    int index = 0;
    do
    {
        if (numCount != 0)
        {
            randomInit(hand, special);
        }
        
        cout << "####################金牌：" << GameRummy::getCardString(special) << endl;
        cout << "#################初始手牌: " << hand.size() << endl;
        for (auto item : hand)
        {
            cout << GameRummy::getCardString(*(item.second));
            if (item.second->rank_ == special.rank_)
            {
                cout << " 金";
            }
            cout << endl;
        }

        std::vector<CardGroup> runList;
        std::vector<CardGroup> meldList;
        std::vector<CardGroup> setList;
        std::vector<CardGroup> unMatchedList;
        GameRummy solution;
        solution.special_ = special.rank_;
        solution.match(hand, runList, meldList, setList, unMatchedList);
        runList.insert(runList.end(), meldList.begin(), meldList.end());
        runList.insert(runList.end(), setList.begin(), setList.end());
        runList.insert(runList.end(), unMatchedList.begin(), unMatchedList.end());

        int count = 0;
        std::for_each(runList.begin(), runList.end(), [&count](const CardGroup &item) {
            count += item.cardlist_.size();
        });

        cout << "#################理牌结果：" << count << endl;
        std::for_each(runList.begin(), runList.end(), [special](const CardGroup &group) {
            GameRummy::printCardGroup(group, special.rank_);
        });
        cout << "#################测试轮数：" << ++index << endl;
    } while (--numCount > 0);

    return 0;
}