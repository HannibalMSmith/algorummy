#include "gamerummy.h"
#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <stdio.h>
#include <string.h>
#include <assert.h>
using std::cout;
using std::endl;
using std::map;
using std::shared_ptr;
using std::string;
using std::vector;

int Card::idx = 0;
int CardGroup::idx = 0;

Card::Card() : suit_(e_spade), rank_(0), id_(genId()), joker_(false)
{
    special_ = false;
    magic_ = suit_ == e_joker;
}

Card::Card(E_SUIT suit, int rank, int goldrank) : suit_(suit), rank_(rank), id_(genId()), joker_(suit_ == e_joker)
{
    special_ = rank_ == goldrank;
    magic_ = suit_ == e_joker || special_;
}

void CardGroup::removeCard(const PCard &card)
{
    cardlist_.erase(std::find(cardlist_.begin(), cardlist_.end(), card));
}

void CardGroup::removeGroup(const CardGroup &rhs)
{
    std::for_each(rhs.cardlist_.begin(), rhs.cardlist_.end(),
                  [this](const PCard &card) {
                      removeCard(card);
                  });
}

void CardGroup::reset()
{
    id_ = genId();
    cardlist_.clear();
}

void Mgr::printCardGroup(const CardGroup &group)
{
    if (group.cardlist_.size() == 0)
    {
        return;
    }
    cout << "分组编号: " << group.id_ << " 卡牌列表" << endl;
    std::for_each(group.cardlist_.begin(), group.cardlist_.end(),
                  [](const PCard &card) {
                      string name = getCardString(*card);
                      cout << name << endl;
                  });
}

string Mgr::getCardString(const Card &card)
{
    string name;
    switch (card.suit_)
    {
    case e_spade:
        name = "黑桃";
        break;
    case e_heart:
        name = "红桃";
        break;
    case e_diamond:
        name = "方块";
        break;
    case e_club:
        name = "梅花";
        break;
    case e_joker:
        if (card.rank_ == 0)
        {
            name = "大王";
        }
        else
        {
            name = "小王";
        }
        break;
    default:
        name = "老千";
        break;
    }

    if (!card.joker_)
    {
        char rank[50] = {};
        switch (card.rank_)
        {
        case 13:
            strcpy(rank, "k");
            break;
        case 12:
            strcpy(rank, "Q");
            break;
        case 11:
            strcpy(rank, "J");
            break;
        case 1:
            strcpy(rank, "A");
            break;
        default:
            sprintf(rank, "%d", card.rank_);
            break;
        }
        name.append(rank);
    }
    return std::move(name);
}

int Mgr::match(const map<int, PCard> &hand, vector<CardGroup> &grouplist)
{
    grouplist.clear();
    vector<CardGroup> matchlist;
    vector<CardGroup> candidates;
    buildRun(hand, matchlist, candidates);
    buildMeldFromGroup(matchlist, candidates);

    return 0;
}

int Mgr::buildRun(const map<int, PCard> &hand, vector<CardGroup> &matchlist, vector<CardGroup> &candidates)
{
    matchlist.clear();
    candidates.clear();
    std::vector<CardGroup> suitlist;
    suitlist.resize(c_suit);
    candidates.resize(c_suit);
    std::for_each(hand.begin(), hand.end(),
                  [&suitlist, &candidates, this](std::pair<const int, PCard> item) {
                      if (item.second->rank_ == special_)
                      {
                          candidates[e_joker].cardlist_.push_back(item.second);
                      }
                      else
                      {
                          suitlist[item.second->suit_].cardlist_.push_back(item.second);
                      }
                  });

    std::for_each(suitlist.begin(), suitlist.end(),
                  [&matchlist, &candidates](CardGroup &group) {
                      std::sort(group.cardlist_.begin(), group.cardlist_.end(), [](const PCard &a, const PCard &b) { return a->rank_ > b->rank_; });
                    buildRunFromGroup(group, matchlist, candidates);
                  });

    cout << "同花组" << endl;
    std::for_each(matchlist.begin(), matchlist.end(), Mgr::printCardGroup);

    return 0;
}

int Mgr::buildRunFromGroup(const CardGroup &group, vector<CardGroup> &matchlist, vector<CardGroup> &candidates)
{
    int matched = 0;
    CardGroup groupCopy(group);
    int groupSize = groupCopy.cardlist_.size();
    if (groupSize < c_minimum && groupSize > 0)
    {
        int idx = groupCopy.cardlist_[0]->suit_;
        assert(idx >= 0 && idx <= 4);
        candidates[idx].cardlist_.insert(candidates[idx].cardlist_.end(),
                                         groupCopy.cardlist_.begin(), groupCopy.cardlist_.end());
        return matched;
    }

    CardGroup tmpMatched;
    auto it = groupCopy.cardlist_.begin();
    tmpMatched.cardlist_.push_back(*it);
    PCard last = NULL;
    for (; it < groupCopy.cardlist_.end(); ++it)
    {
        if (last == NULL)
        {
            last = *it;
            // cout << "匹配开始" << endl;
            // cout << "last: " << "NULL" << ":" << "NULL";
            // cout << " it: " << (*it)->suit_ << ":" << (*it)->rank_ << endl;
            continue;
        }

        if ((*it)->rank_ == last->rank_)
        {
            // cout << "匹配重复" << endl;
            // cout << "last: " << last->suit_ << ":" << last->rank_;
            // cout << " it: " << (*it)->suit_ << ":" << (*it)->rank_ << endl;
            continue;
        }
        else if (last->rank_ - (*it)->rank_ == 1)
        {
            // cout << "匹配成功" << endl;
            // cout << "last: " << last->suit_ << ":" << last->rank_;
            // cout << " it: " << (*it)->suit_ << ":" << (*it)->rank_ << endl;
            tmpMatched.cardlist_.push_back(*it);
            last = *it;
        }
        else
        {
            // cout << "匹配失败" << endl;
            // cout << "last: " << last->suit_ << ":" << last->rank_;
            // cout << " it: " << (*it)->suit_ << ":" << (*it)->rank_ << endl;
            tmpMatched.reset();
            tmpMatched.cardlist_.push_back(*it);
            last = *it;
            continue;
        }

        if (tmpMatched.cardlist_.size() == c_minimum - 1)
        {
            if ((*(tmpMatched.cardlist_.begin()))->rank_ == 13 && (*(tmpMatched.cardlist_.begin() + 1))->rank_ == 12)
            {
                auto ace = std::find_if(groupCopy.cardlist_.begin(), groupCopy.cardlist_.end(), [](const PCard &card) { return card->rank_ == 1; });
                if (ace != groupCopy.cardlist_.end())
                {
                    matched++;
                    tmpMatched.cardlist_.insert(tmpMatched.cardlist_.begin(), (*ace));
                    matchlist.push_back(tmpMatched);
                    groupCopy.removeGroup(tmpMatched);
                    tmpMatched.reset();
                    it = groupCopy.cardlist_.begin();
                    if (it != groupCopy.cardlist_.end())
                    {
                        tmpMatched.cardlist_.push_back(*(it));
                        last = *it;
                    }
                }
            }
        }
        else if (tmpMatched.cardlist_.size() == c_minimum)
        {
            matched++;
            matchlist.push_back(tmpMatched);
            groupCopy.removeGroup(tmpMatched);
            tmpMatched.reset();
            it = groupCopy.cardlist_.begin();
            if (it != groupCopy.cardlist_.end())
            {
                tmpMatched.cardlist_.push_back(*it);
            }
            last = *it;
        }
    }

    if (tmpMatched.cardlist_.size() > 0)
    {
        int idx = tmpMatched.cardlist_[0]->suit_;
        candidates[idx].cardlist_.insert(candidates[idx].cardlist_.end(),
                                         groupCopy.cardlist_.begin(), groupCopy.cardlist_.end());
    }
    return matched;
}

int Mgr::buildMeldFromGroup(std::vector<CardGroup> &matchlist, std::vector<CardGroup> &candidates)
{
    std::for_each(candidates.begin(), candidates.end(), [](CardGroup &group) {
        std::sort(group.cardlist_.begin(), group.cardlist_.end(), [](const PCard &a, const PCard &b) {
            return a->rank_ > b->rank_;
        });
    });

    cout << "候选组选拔" << endl;
    std::for_each(candidates.begin(), candidates.end(), Mgr::printCardGroup);

    int targetGoal[c_suit] = {};
    for (int i = 1; i < c_suit; i++)
    {
        targetGoal[i] = 0;
    }

    return 0;
}

