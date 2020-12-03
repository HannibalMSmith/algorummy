#include "gamerummy.h"
#include <algorithm>
#include <functional>
#include <iostream>
using std::map;
using std::vector;
using std::for_each;
using std::cout;
using std::endl;


int Card::idx = 0;
int CardGroup::idx = 0;



void CardGroup::delCard(const PCard &card)
{
    //cardlist_.erase(std::find(cardlist_.begin(), cardlist_.end(),  [](const PCard &a, const PCard &b){return a->id_ == b->id_;}));
    cardlist_.erase(std::find(cardlist_.begin(), cardlist_.end(), card));
}

void Mgr::match(const map<int, PCard> &hand, vector<CardGroup> &grouplist)
{
    grouplist.clear();
    vector<CardGroup> matchlist;
    vector<CardGroup> candidates;
    tryPickRun(hand, matchlist, candidates);
}

//按花色倒序分组提取同花顺（排除金）
void Mgr::tryPickRun(const map<int, PCard> &hand, vector<CardGroup> &matchlist, vector<CardGroup> &candidates)
{
    matchlist.clear();
    candidates.clear();
    std::vector<CardGroup> coloredgrouplist;
    coloredgrouplist.resize(c_suitcout);
    for_each(hand.begin(), hand.end(),
    [&coloredgrouplist, &matchlist](std::pair<const int, PCard> item)
    {
        coloredgrouplist[item.second->suit_-1].cardlist_.push_back(item.second);
    });

    for_each(coloredgrouplist.begin(), coloredgrouplist.end(),
    [](const CardGroup &group)
    {
        cout <<"tryPickRun group:" << group.id_ << endl;
       for_each(group.cardlist_.begin(), group.cardlist_.end(),
       [](const PCard card)
       {
           cout <<"suit:" << card->suit_ << " rank:" << card->rank_ << endl;
       });     
    }
    );


    for_each(coloredgrouplist.begin(), coloredgrouplist.end(),
    [&matchlist](CardGroup &group)
    {
        std::sort(group.cardlist_.begin(), group.cardlist_.end(), [](const PCard &a, const PCard &b){return a >b;});
        pickRunFromGroup(group, matchlist);
    }
    );

    for_each(matchlist.begin(), matchlist.end(),
    [](const CardGroup &group)
    {   
        cout << "groupid:" << group.id_ << " " << endl;
        for_each(group.cardlist_.begin(), group.cardlist_.end(), [](const PCard &card ){cout << "suit: "<<card->suit_ << " rank: " << card->rank_<< endl;});
        cout << endl;
    }
    );

}

void Mgr::pickRunFromGroup(CardGroup &group, vector<CardGroup> &matchlist)
{
    if (group.cardlist_.size() < 3)
    {
        return;
    }
    CardGroup tmpGroup;
    auto it = group.cardlist_.begin();
    while ((*it)->bmagic_)
    {
        ++it;
    }
    tmpGroup.cardlist_.push_back(*it);
    for (; it != group.cardlist_.end() -1; ++it)
    {
        if((!(*it)->bmagic_) && (!(*(it+1))->bmagic_) && (*it)->rank_ - (*(it+1))->rank_ == 1 )
        {
            tmpGroup.cardlist_.push_back(*(it+1));
            if(tmpGroup.cardlist_.size() ==3)
            {
                matchlist.push_back(tmpGroup);
                for_each(tmpGroup.cardlist_.begin(), tmpGroup.cardlist_.end(), [&group](const PCard &card){group.delCard(card);});
                tmpGroup.cardlist_.clear();
                if (it == group.cardlist_.end() -1)
                {
                    break;
                }
                it = ++it;
                tmpGroup.cardlist_.push_back(*it);
            } 
        }     
    }

}