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

int Card::getGoal()
{
    int goal = 0;
    switch (rank_)
    {
    case 1:
    case 11:
    case 12:
    case 13:
        goal = 10;
        break;
    default:
        goal = rank_;
        break;
    }
    return goal;
}

void CardGroup::removeCard(const PCard &card)
{
    cardlist_.erase(std::find(cardlist_.begin(), cardlist_.end(), card));
}

void CardGroup::removeCard(int id)
{
    cardlist_.erase(std::find_if(cardlist_.begin(), cardlist_.end(), [id](const PCard &card) { return card->id_ == id; }));
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

int CardGroup::getGoal()
{
    //std::accumulate(cardlist_.begin(), cardlist_.end(), cardlist_.begin(), [](const PCard &a, const PCard &b){return a->getGoal()+b->getGoal()});
    int goal = 0;
    for (auto it : cardlist_)
    {
        goal += it->getGoal();
    }
    return goal;
}

int CardGroup::expandGroup(const PCard &card)
{
    int suit = cardlist_[0]->suit_;
    if (suit != card->suit_)
    {
        return 0;
    }

    if (card->rank_+1 == (*cardlist_.begin())->rank_)
    {
        cardlist_.insert(cardlist_.begin(), card);
        return 1;
    }

    if (card->rank_-1 == (*cardlist_.begin())->rank_)
    {
        cardlist_.insert(cardlist_.end(), card);
        return 1;
    }
    return 0;
    

}

void GameRummy::printCardGroup(const CardGroup &group)
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

string GameRummy::getCardString(const Card &card)
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

int GameRummy::match(const std::map<int, PCard> &hand, std::vector<CardGroup> &runList, std::vector<CardGroup> &meldList, std::vector<CardGroup> &setList, std::vector<CardGroup> &unMatchedList)
{
    runList.clear();
    meldList.clear();
    setList.clear();
    vector<CardGroup> candidates;
    buildRun(hand, runList, candidates);
    buildMeldAndSet(candidates, meldList, setList);

    return 0;
}

//按花色倒序分组
int GameRummy::buildRun(const map<int, PCard> &hand, vector<CardGroup> &runList, vector<CardGroup> &candidates)
{
    runList.clear();
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
                  [&runList, &candidates](CardGroup &group) {
                      std::sort(group.cardlist_.begin(), group.cardlist_.end(), [](const PCard &a, const PCard &b) { return a->rank_ > b->rank_; });
                      buildRunFromGroup(group, runList, candidates);
                  });

    // cout << "同花组" << endl;
    // std::for_each(runList.begin(), runList.end(), GameRummy::printCardGroup);

    return 0;
}

//选取最大同花顺
int GameRummy::buildRunFromGroup(CardGroup &group, vector<CardGroup> &runList, vector<CardGroup> &candidates)
{
    int matched = 0;
    int groupSize = group.cardlist_.size();
    if (groupSize < c_minimum && groupSize > 0)
    {
        int idx = group.cardlist_[0]->suit_;
        assert(idx >= 0 && idx <= 4);
        candidates[idx].cardlist_.insert(candidates[idx].cardlist_.end(),
                                         group.cardlist_.begin(), group.cardlist_.end());
        return matched;
    }

    CardGroup tmpMatched;
    auto it = group.cardlist_.begin();
    tmpMatched.cardlist_.push_back(*it);
    PCard last = NULL;
    for (; it < group.cardlist_.end(); ++it)
    {
        if (last == NULL)
        {
            last = *it;
            continue;
        }

        if ((*it)->rank_ == last->rank_)
        {
            continue;
        }
        else if (last->rank_ - (*it)->rank_ == 1)
        {
            tmpMatched.cardlist_.push_back(*it);
            last = *it;
        }
        else
        {
            if (tmpMatched.cardlist_.size() >= c_minimum)
            {
                runList.push_back(tmpMatched);
            }
            else
            {
                int idx = tmpMatched.cardlist_[0]->suit_;
                candidates[idx].cardlist_.insert(candidates[idx].cardlist_.end(),
                                                 tmpMatched.cardlist_.begin(), tmpMatched.cardlist_.end());
            }
            group.removeGroup(tmpMatched);
            tmpMatched.reset();
            it = group.cardlist_.begin();
            if (it != group.cardlist_.end())
            {
                tmpMatched.cardlist_.push_back(*it);
            }
            last = *it;
            continue;
        }

        if (tmpMatched.cardlist_.size() == c_minimum - 1)
        {
            if ((*(tmpMatched.cardlist_.begin()))->rank_ == 13 && (*(tmpMatched.cardlist_.begin() + 1))->rank_ == 12)
            {
                auto ace = std::find_if(group.cardlist_.begin(), group.cardlist_.end(), [](const PCard &card) { return card->rank_ == 1; });
                if (ace != group.cardlist_.end())
                {
                    tmpMatched.cardlist_.insert(tmpMatched.cardlist_.begin(), (*ace));
                    group.removeCard(*ace);
                    int id = (*it)->id_;
                    it = std::find_if(group.cardlist_.begin(), group.cardlist_.end(), [id](const PCard &card) { return card->id_ == id; });
                    last = *it;
                }
            }
        }
    }

    if (tmpMatched.cardlist_.size() > 0)
    {
        int idx = tmpMatched.cardlist_[0]->suit_;
        candidates[idx].cardlist_.insert(candidates[idx].cardlist_.end(),
                                         group.cardlist_.begin(), group.cardlist_.end());
    }
    return matched;
}

void GameRummy::removeSetFromCandidates(std::vector<CardGroup> &candidates)
{
    std::for_each(candidates.begin(), candidates.end(), [](CardGroup &group) {
        int idx = group.idxSetMember_;
        if (idx != c_idxerror)
        {
            group.cardlist_.erase(group.cardlist_.begin() + idx);
            group.idxSetMember_ = c_idxerror;
        }
    });
}

//从候选组选出最高得分的顺子或者套牌
int GameRummy::buildMeldAndSet(std::vector<CardGroup> &candidates, std::vector<CardGroup> &meldList, std::vector<CardGroup> &setList)
{
    std::for_each(candidates.begin(), candidates.end(), [](CardGroup &group) {
        std::sort(group.cardlist_.begin(), group.cardlist_.end(), [](const PCard &a, const PCard &b) { return a->rank_ > b->rank_; });
    });
    std::sort(candidates[0].cardlist_.begin(), candidates[0].cardlist_.end(), [](const PCard &a, const PCard &b) { return a->rank_ < b->rank_; });
    std::vector<CardGroup> candidatesCopy(candidates);
        
    cout << "候选组选拔" << endl;
    std::for_each(candidates.begin(), candidates.end(), GameRummy::printCardGroup);

    while (true)
    {
        std::vector<int> targetGoal;
        targetGoal.resize(c_suit);
        CardGroup set;
        CardGroup meld;
        int setGoal = 0;
        int meldGoal = 0;
        for (size_t i = 1; i < c_suit; i++)
        {
            setGoal = buildSetFromTop(candidatesCopy, candidatesCopy[i], set);
            meldGoal = buildMeldFromTop(candidates, candidatesCopy[i], meld);
            if (meldGoal > setGoal)
            {
                targetGoal[i] = meldGoal;
            }
            else
            {
                targetGoal[i] = setGoal;
            }
        }

        cout<<"#####################"<<endl;
        cout<<"最大匹配卡牌："<<endl;
        printCardGroup(set);
        printCardGroup(meld);
        cout<<"#####################"<<endl;
        

        auto itMax = std::max_element(targetGoal.begin(), targetGoal.end());
        if (*itMax == 0)
        {
            for (int i = 1; i < c_suit; i++)
            {
                candidatesCopy[i].cardlist_.erase(candidatesCopy[i].cardlist_.begin());
            }
            if (candidatesCopy[1].cardlist_.size() == 0 && candidatesCopy[2].cardlist_.size() == 0 &&
                candidatesCopy[3].cardlist_.size() == 0 && candidatesCopy[4].cardlist_.size() == 0)
            {
                break;
            }
        }
        int idxGroup = itMax - targetGoal.begin();
        if (set.cardlist_.size() > c_minimum)
        {
            if (meldGoal > 0)
            {
                int rank = set.cardlist_[0]->rank_;
                auto itCard = std::find_if(meld.cardlist_.begin(), meld.cardlist_.end(), [rank](const PCard &card) { return card->rank_ == rank; });
                set.removeCard((*itCard)->id_);
                setList.push_back(set);

                candidates[idxGroup].removeGroup(meld);
                candidates[idxGroup].idxSetMember_ = c_idxerror;
                candidatesCopy[idxGroup].removeGroup(meld);
                candidatesCopy[idxGroup].idxSetMember_ = c_idxerror;
                meld.cardlist_.push_back(candidates[0].cardlist_[0]);
                candidates[0].cardlist_.erase(candidates[0].cardlist_.begin());
                meldList.push_back(meld);
                removeSetFromCandidates(candidates);
                removeSetFromCandidates(candidatesCopy);
            }
            else
            {
                setList.push_back(set);
                removeSetFromCandidates(candidates);
                removeSetFromCandidates(candidatesCopy);
            }
        }
        else if (set.cardlist_.size() == c_minimum)
        {
            setList.push_back(set);
            removeSetFromCandidates(candidates);
            removeSetFromCandidates(candidatesCopy);
        }
        else if (set.cardlist_.size() == c_minimum - 1)
        {
            set.cardlist_.push_back(candidates[0].cardlist_[0]);
            candidates[0].cardlist_.erase(candidates[0].cardlist_.begin());
            setList.push_back(set);
            removeSetFromCandidates(candidates);
            removeSetFromCandidates(candidatesCopy);
        }
        else
        {
            candidates[idxGroup].removeGroup(meld);
            candidatesCopy[idxGroup].removeGroup(meld);
            meld.cardlist_.push_back(candidates[0].cardlist_[0]);
            candidates[0].cardlist_.erase(candidates[0].cardlist_.begin());
            meldList.push_back(meld);
        }
    }

    return 0;
}

//选出同花色潜在顺子
int GameRummy::buildMeldFromTop(std::vector<CardGroup> &candidates, CardGroup &group, CardGroup &potential)
{
    if (group.cardlist_.size() < c_minimum - 1 || candidates[0].cardlist_.size() == 0)
    {
        return 0;
    }
    potential.reset();
    auto it = group.cardlist_.begin();
    if ((*it)->rank_ - (*(it + 1))->rank_ == 1 || (*it)->rank_ - (*(it + 1))->rank_ == 2)
    {
        potential.cardlist_.push_back(*it);
        potential.cardlist_.push_back(*(it + 1));
        return potential.getGoal();
    }
    else if ((*it)->rank_ - (*(it + 1))->rank_ == 0)
    {
        ++it;
        return buildMeldFromTop(candidates, group, potential);
    }
    else
    {
        return 0;
    }
}

//选出潜在套牌
int GameRummy::buildSetFromTop(std::vector<CardGroup> &candidates, CardGroup &group, CardGroup &set)
{
    if (group.cardlist_.size() == 0)
    {
        return 0;
    }
    int rank = group.cardlist_[0]->rank_;
    CardGroup tempSet;
    std::for_each(candidates.begin(), candidates.end(),
                  [&tempSet, rank](CardGroup &group) {
                      auto itCard = std::find_if(group.cardlist_.begin(), group.cardlist_.end(), [rank](PCard &card) { return card->rank_ == rank; });
                      if (itCard != group.cardlist_.end())
                      {
                          tempSet.cardlist_.push_back(*itCard);
                          group.idxSetMember_ = itCard - group.cardlist_.begin();
                      }
                  });

    int nMember = tempSet.cardlist_.size();
    if (nMember < c_minimum - 1)
    {
        std::for_each(candidates.begin(), candidates.end(), [](CardGroup &val) { val.idxSetMember_ = c_idxerror; });
        return 0;
    }
    if (nMember == c_minimum - 1)
    {
        if (candidates[0].cardlist_.size() == 0)
        {
            std::for_each(candidates.begin(), candidates.end(), [](CardGroup &val) { val.idxSetMember_ = c_idxerror; });
            return 0;
        }
        else
        {
            return set.getGoal();
        }
    }
    if (nMember > c_minimum)
    {
        std::for_each(candidates.begin(), candidates.end(), [](CardGroup &group) { group.optional_ = true; });
    }
    set = std::move(tempSet);
    return set.getGoal();
}

int GameRummy::buildCandidates(std::vector<CardGroup> &runList, std::vector<CardGroup> &meldList, std::vector<CardGroup> &setList,
                               std::vector<CardGroup> &candidates, std::vector<CardGroup> &unMatchedList)
{
    std::sort(candidates[0].cardlist_.begin(), candidates[0].cardlist_.end(), [](const PCard &a, const PCard &b){return a > b;});
    if (candidates[0].cardlist_.size() == 0)
    {
        return 0;
    }
    else
    {
        if (candidates[0].cardlist_[0]->rank_ != 0)
        {
            PCard special = candidates[0].cardlist_[0];
            std::for_each(runList.begin(), runList.end(), [&candidates, &special](CardGroup &group){
                if(group.expandGroup(special))
                {
                    candidates[0].cardlist_.erase(candidates[0].cardlist_.begin());
                }
            });
        }
        CardGroup unMatchedGroup;
        std::for_each(unMatchedList.begin(), unMatchedList.end(), [&unMatchedGroup](const CardGroup &group){
            unMatchedGroup.cardlist_.insert(unMatchedGroup.cardlist_.begin(), group.cardlist_.begin(), group.cardlist_.end());
        });
        std::sort(unMatchedGroup.cardlist_.begin(), unMatchedGroup.cardlist_.end(), []( const PCard &a, const PCard &b){return a->rank_ > b->rank_;});

        if (candidates[0].cardlist_.size() == 3)
        {
            //todo special data
           unMatchedList.clear();
           unMatchedList.push_back(candidates[0]);
           unMatchedList.push_back(unMatchedGroup);
        }
        else if(candidates[0].cardlist_.size() == 2)
        {
            unMatchedList.clear();
            CardGroup newGroup;
            newGroup.cardlist_.push_back(unMatchedGroup.cardlist_[0]);
            unMatchedList.push_back(newGroup);
            unMatchedGroup.cardlist_.erase(unMatchedGroup.cardlist_.begin());
            unMatchedList.push_back(unMatchedGroup);

        }
    }

}
