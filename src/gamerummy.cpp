#include "gamerummy.h"
#include <algorithm>
#include <functional>
#include <iostream>
using std::cout;
using std::endl;
using std::for_each;
using std::map;
using std::shared_ptr;
using std::vector;

int Card::idx = 0;
int CardGroup::idx = 0;

void CardGroup::delCard(const PCard &card)
{
    cardlist_.erase(std::find(cardlist_.begin(), cardlist_.end(), card));
}

void CardGroup::reset()
{
    id_ = genId();
    cardlist_.clear();
}

void Mgr::match(const map<int, PCard> &hand, vector<CardGroup> &grouplist)
{
    grouplist.clear();
    vector<CardGroup> matchlist;
    vector<CardGroup> candidates;
    tryPickRun(hand, matchlist, candidates);
}

void Mgr::tryPickRun(const map<int, PCard> &hand, vector<CardGroup> &matchlist, vector<CardGroup> &candidates)
{
    matchlist.clear();
    candidates.clear();
    std::vector<CardGroup> coloredgrouplist;
    coloredgrouplist.resize(c_suitcout);
    for_each(hand.begin(), hand.end(),
             [&coloredgrouplist, &matchlist](std::pair<const int, PCard> item) {
                 coloredgrouplist[item.second->suit_ - 1].cardlist_.push_back(item.second);
             });

    
    for_each(coloredgrouplist.begin(), coloredgrouplist.end(),
             [&matchlist](CardGroup &group) {
                 std::sort(group.cardlist_.begin(), group.cardlist_.end(), [](const PCard &a, const PCard &b) { return a->rank_ > b->rank_; });
                 pickRunFromGroup(group, matchlist);
             });
    
    cout<<"同花组"<<endl;
    for_each(matchlist.begin(), matchlist.end(), Mgr::printCardGroup);
}

void Mgr::pickRunFromGroup(const CardGroup &group, vector<CardGroup> &matchlist)
{
    
    if (group.cardlist_.size() < 3)
    {
        return;
    }

    auto last = group.cardlist_.begin();
    for (auto it = group.cardlist_.begin(); it != group.cardlist_.end(); ++it)
    {
        CardGroup tmpGroup;
        tmpGroup.cardlist_.push_back(*last);
        if ((*it)->bmagic_ || (*it)->rank_ == (*last)->rank_)
        {
            continue;
        }
        else if (!((*it)->bmagic_) && (*last)->rank_ - (*it)->rank_ == 1)
        {
            tmpGroup.cardlist_.push_back(*it);
            last = it;
        }
        
        if (tmpGroup.cardlist_.size() == 3)
        {
            matchlist.push_back(tmpGroup);
            tmpGroup.reset();
            if (it+1 != group.cardlist_.end())
            {
                last = it+1;
                tmpGroup.cardlist_.push_back(*last);
            }
        }
    }
}

void Mgr::printCardGroup(CardGroup &group)
{
    cout<<"group: "<<group.id_<<" info"<<endl;
    for_each(group.cardlist_.begin(), group.cardlist_.end(),
             [](const PCard card) {
                 cout << "suit:" << card->suit_ << " rank:" << card->rank_ << endl;
             });

}

