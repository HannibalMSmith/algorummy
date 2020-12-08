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

    std::vector<CardGroup> groupList;
    Mgr mgr;
    mgr.special_ = special.rank_;
    mgr.match(hand, groupList);
    
    cout<<"理牌结果#################"<<endl;
    std::for_each(groupList.begin(), groupList.end(), [](const CardGroup &group)
    {
        Mgr::printCardGroup(group);
    });
    
    return 0;
}