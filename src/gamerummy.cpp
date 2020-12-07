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

int Mgr::match(const map<int, PCard> &hand, vector<CardGroup> &grouplist)
{
    grouplist.clear();
    vector<CardGroup> matchlist;
    vector<CardGroup> candidates;
    pickRun(hand, matchlist, candidates);
    return 0;
}

int Mgr::pickRun(const map<int, PCard> &hand, vector<CardGroup> &matchlist, vector<CardGroup> &candidates)
{
    matchlist.clear();
    candidates.clear();
    std::vector<CardGroup> suitlist;
    suitlist.resize(c_suit);
    for_each(hand.begin(), hand.end(),
             [&suitlist, &matchlist](std::pair<const int, PCard> item) {
                 suitlist[item.second->suit_ - 1].cardlist_.push_back(item.second);
             });

    
    for_each(suitlist.begin(), suitlist.end(),
             [&matchlist, &candidates](CardGroup &group) {
                 std::sort(group.cardlist_.begin(), group.cardlist_.end(), [](const PCard &a, const PCard &b) { return a->rank_ > b->rank_; });
                 pickRunFromGroup(group, matchlist, candidates);
             });
    
    cout<<"同花组"<<endl;
    for_each(matchlist.begin(), matchlist.end(), Mgr::printCardGroup);
    return 0;
}

int Mgr::pickRunFromGroup(const CardGroup &group, vector<CardGroup> &matchlist, vector<CardGroup> &candidates)
{
    
    if (group.cardlist_.size() < 3)
    {
        candidates.push_back(group);
        return 0;
    }


    //todo fix the error
    CardGroup tmpGroup;
    auto last = group.cardlist_.begin();
    for (auto it = group.cardlist_.begin(); it != group.cardlist_.end(); ++it)
    {
        tmpGroup.cardlist_.push_back(*last);
        if ((*it)->magic_ || (*it)->rank_ == (*last)->rank_)
        {
            continue;
        }
        else if ((*last)->rank_ - (*it)->rank_ == 1 && !((*it)->magic_))
        {
            tmpGroup.cardlist_.push_back(*it);
            last = it;
        }
        
        if (tmpGroup.cardlist_.size() == C_minimum)
        {
            matchlist.push_back(tmpGroup);
            tmpGroup.reset();
            if (it+1 != group.cardlist_.end())
            {
                last = it+1;
                tmpGroup.cardlist_.push_back(*last);
            }
        }
        else if((*it)->rank_ == 1 && (*tmpGroup.cardlist_.begin())->rank_ == 13 &&  (*(tmpGroup.cardlist_.begin()+1))->rank_==12)
        {
            tmpGroup.cardlist_.insert(tmpGroup.cardlist_.begin(), (*it));
            if (tmpGroup.cardlist_.size() > C_minimum)
            {
                PCard card = *(tmpGroup.cardlist_.end()-1);
                CardGroup tmp;
                tmp.cardlist_.push_back(card);
                candidates.push_back(tmp);
                tmpGroup.cardlist_.pop_back();
            }
            
            matchlist.push_back(tmpGroup);
            tmpGroup.reset(); 
        }
    }

    return 0;
}

void Mgr::printCardGroup(const CardGroup &group)
{
    cout<<"group: "<<group.id_<<" info"<<endl;
    for_each(group.cardlist_.begin(), group.cardlist_.end(),
             [](const PCard card) {
                 cout << "suit:" << card->suit_ << " rank:" << card->rank_ << endl;
             });

}