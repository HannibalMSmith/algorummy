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
    Card magic;
    if(!init(script.c_str(), hand, magic))
        return 1;

    Mgr mgr;
    std::vector<CardGroup> groupList;
    mgr.match(hand, groupList);

    return 0;
}