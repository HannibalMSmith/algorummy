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
    using std::shared_ptr;
    using std::map;
    using std::vector;
    using std::string;

    string script = scriptPrompt(argc, argv);

    using PCard = shared_ptr<Card>;
    map<int, PCard> hand;
    Card special;
    if(!init(script.c_str(), hand, special))
        return 1;

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
    cout<<"理牌结果#################"<<endl;
    std::for_each(runList.begin(), runList.end(), [](const CardGroup &group)
    {
        GameRummy::printCardGroup(group);
    });
    return 0;
}