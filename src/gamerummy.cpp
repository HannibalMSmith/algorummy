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

Card::Card() : suit_(e_spade), rank_(0), id_(genId()), joker_(suit_ == e_joker)
{
    special_ = false;
    magic_ = suit_ == e_joker;
}

Card::Card(E_SUIT suit, int rank, int goldrank) : suit_(suit), rank_(rank), id_(genId()), joker_(suit_ == e_joker)
{
    special_ = rank_ == goldrank;
    magic_ = suit_ == e_joker;
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

bool CardGroup::expandToRun(const PCard &card)
{
    if (card->suit_ != cardlist_[0]->suit_)
    {
        return false;
    }
    if (cardlist_[1]->rank_ - cardlist_[0]->rank_ == 1)
    {

    }
    else if (cardlist_[1]->rank_ - cardlist_[0]->rank_ == 1)
    {

    }
    else
    {
        assert("headache");
    }
}

bool CardGroup::expandToGroup(const PCard &card)
{
    if (card->special_)
    {
        if (expandToRun(card))
        {
            return true;
        }
    }
    if (card->magic_)
    {
        assert(cardlist_.size() >= c_minimum - 1);
        if (cardlist_[1]->rank_ - cardlist_[0]->rank_ == 0)
        {
            if (cardlist_.size() == c_minimum)
            {
                cardlist_.push_back(card);
                return true;
            }
            else
            {
                return false;
            }
        }
        else if (cardlist_[1]->rank_ - cardlist_[0]->rank_ == 1)
        {
            cardlist_.push_back(card);
            return true;
        }
        else if (cardlist_[1]->rank_ - cardlist_[0]->rank_ == 2)
        {
            cardlist_.insert(cardlist_.begin() + 1, card);
            return true;
        }
    }
    return false;
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

int GameRummy::match(const std::map<int, PCard> &hand, std::vector<CardGroup> &runList, std::vector<CardGroup> &meldList,
                     std::vector<CardGroup> &setList, std::vector<CardGroup> &unMatchedList)
{
    runList.clear();
    meldList.clear();
    setList.clear();
    unMatchedList.clear();
    vector<CardGroup> candidates;
    buildRun(hand, runList, candidates);
    buildMeldAndSet(candidates, meldList, setList);
    buildCandidates(runList, meldList, setList, candidates);
    unMatchedList = candidates;
    return 0;
}

//按花色倒序分组
int GameRummy::buildRun(const map<int, PCard> &hand, vector<CardGroup> &runList, vector<CardGroup> &candidates)
{
    std::vector<CardGroup> suitList;
    suitList.resize(c_suit);
    candidates.resize(c_suit);
    std::for_each(hand.begin(), hand.end(),
                  [&suitList, &candidates, this](std::pair<const int, PCard> item) {
                      if (item.second->special_ || item.second->magic_)
                      {
                          candidates[e_joker].cardlist_.push_back(item.second);
                      }
                      else
                      {
                          suitList[item.second->suit_].cardlist_.push_back(item.second);
                      }
                  });

    std::for_each(suitList.begin(), suitList.end(),
                  [&runList, &candidates](CardGroup &group) {
                      std::sort(group.cardlist_.begin(), group.cardlist_.end(), [](const PCard &a, const PCard &b) { return a->rank_ > b->rank_; });
                      buildRunFromGroup(group, runList, candidates);
                  });

#ifdef Debug
    {
        cout << "同花组***********************" << endl;
        std::for_each(runList.begin(), runList.end(), GameRummy::printCardGroup);
        cout << "同花组***********************" << endl;
    }
#endif

    return 0;
}

//选取最大同花顺
int GameRummy::buildRunFromGroup(CardGroup &group, vector<CardGroup> &runList, vector<CardGroup> &candidates)
{
    int matched = 0;
    int groupSize = group.cardlist_.size();
    if (groupSize == 0)
    {
        return 0;
    }
    else if (groupSize < c_minimum && groupSize > 0)
    {
        int idx = group.cardlist_[0]->suit_;
        assert(idx >= 0 && idx <= 4);
        candidates[idx].cardlist_.insert(candidates[idx].cardlist_.end(),
                                         group.cardlist_.begin(), group.cardlist_.end());
        return 0;
    }

    CardGroup tmpMatched;
    auto it = group.cardlist_.begin();
    tmpMatched.cardlist_.push_back(*it);
    PCard last = NULL;
    for (; it < group.cardlist_.end(); ++it)
    {
        if (last == NULL)
        {
#ifdef DebugMore
            cout << "last: "<< "null"<< ":" << "null";
            cout << " it: " << (*it)->suit_ << ":" << (*it)->rank_ << endl;
#endif
            last = *it;
            continue;
        }

#ifdef DebugMore
        cout << "last: " << last->suit_ << ":" << last->rank_;
        cout << " it: " << (*it)->suit_ << ":" << (*it)->rank_ << endl;
#endif
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
            else if (tmpMatched.cardlist_.size() == c_minimum - 1)
            {
                do
                {
                    if ((*(tmpMatched.cardlist_.begin()))->rank_ == 13 && (*(tmpMatched.cardlist_.begin() + 1))->rank_ == 12)
                    {
                        auto ace = std::find_if(group.cardlist_.begin(), group.cardlist_.end(), [](const PCard &card) { return card->rank_ == 1; });
                        if (ace != group.cardlist_.end())
                        {
                            tmpMatched.cardlist_.insert(tmpMatched.cardlist_.begin(), (*ace));
                            runList.push_back(tmpMatched);
                            break;
                        }
                    }
                    int idx = tmpMatched.cardlist_[0]->suit_;
                    candidates[idx].cardlist_.insert(candidates[idx].cardlist_.end(),
                                                     tmpMatched.cardlist_.begin(), tmpMatched.cardlist_.end());
                } while (false);
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
        }
    }

    if (tmpMatched.cardlist_.size() >= c_minimum)
    {
        runList.push_back(tmpMatched);
        group.removeGroup(tmpMatched);
        if (group.cardlist_.size() >= c_minimum)
        {
            buildRunFromGroup(group, runList, candidates);
        }
        else
        {
            int idx = tmpMatched.cardlist_[0]->suit_;
            candidates[idx].cardlist_.insert(candidates[idx].cardlist_.end(),
                                             group.cardlist_.begin(), group.cardlist_.end());
        }
    }
    else if (tmpMatched.cardlist_.size() > 0)
    {
        int idx = tmpMatched.cardlist_[0]->suit_;
        candidates[idx].cardlist_.insert(candidates[idx].cardlist_.end(),
                                         group.cardlist_.begin(), group.cardlist_.end());
    }
    return 1;
}

void GameRummy::removeSetFromCandidates(std::vector<CardGroup> &candidates)
{
    int i = 0;
    std::for_each(candidates.begin(), candidates.end(),
                  [&i](CardGroup &group) {
                      int idx = group.idxSetMember_;
                      if (idx != c_idxerror)
                      {
                          group.cardlist_.erase(group.cardlist_.begin() + idx);
                          group.idxSetMember_ = c_idxerror;
                      }
                  });
}

//从候选组选出最高得分的顺子或者套牌
int GameRummy::buildMeldAndSet(std::vector<CardGroup> &candidates, std::vector<CardGroup> &meldList,
                               std::vector<CardGroup> &setList)
{
    std::for_each(candidates.begin(), candidates.end(), [](CardGroup &group) {
        std::sort(group.cardlist_.begin(), group.cardlist_.end(),
                  [](const PCard &a, const PCard &b) { return a->rank_ > b->rank_; });
    });
    std::sort(candidates[0].cardlist_.begin(), candidates[0].cardlist_.end(),
              [](const PCard &a, const PCard &b) { return a->rank_ < b->rank_; });

    while (true)
    {

#ifdef Debug
        {
            cout << "候选组#######################" << endl;
            std::for_each(candidates.begin(), candidates.end(), GameRummy::printCardGroup);
            cout << "候选组#######################" << endl;
        }
#endif
        vector<CardGroup> tempSetList(c_suit);
        vector<CardGroup> tempMeldList(c_suit);
        vector<int> targetGoal(c_suit);
        vector<int> meldGoal(c_suit);
        for (auto idx = 1; idx < c_suit; ++idx)
        {
            int setGoal = buildSetFromTop(candidates, candidates[idx], tempSetList[idx]);
            meldGoal[idx] = buildMeldFromTop(candidates, candidates[idx], tempMeldList[idx]);
            if (meldGoal[idx] > setGoal)
            {
                targetGoal[idx] = meldGoal[idx];
            }
            else
            {
                targetGoal[idx] = setGoal;
            }
        }
        auto itMax = std::max_element(targetGoal.begin(), targetGoal.end());
        int idxGroup = itMax - targetGoal.begin();
        auto itMaxMeld = std::max_element(meldGoal.begin(), meldGoal.end());
        int idxMeld = itMaxMeld - meldGoal.begin();
 #ifdef DebugMore
        {
            cout << "分组最高分："<<*itMax<<" 位置："<<idxGroup<<" 顺子最高分："<<*itMaxMeld<<" 位置："<<idxMeld<<endl;
        }
#endif             
        if (*itMax == 0)
        {
            break;
        }

        if (*itMaxMeld == *itMax)
        {
            assert(idxGroup == idxMeld);
            candidates[idxGroup].removeGroup(tempMeldList[idxGroup]);
            PCard special;
            if (tryBuildRunWithSpecial(candidates[0], tempMeldList[idxGroup], special))
            {
                tempMeldList[idxGroup].cardlist_.push_back(special);
                meldList.push_back(tempMeldList[idxGroup]);
                candidates[0].removeCard(special);
            }
            else
            {
                tempMeldList[idxGroup].cardlist_.push_back(candidates[0].cardlist_[0]);
                meldList.push_back(tempMeldList[idxGroup]);
                candidates[0].cardlist_.erase(candidates[0].cardlist_.begin());
            }
        }
        else
        {
            //fix me with better bookkeep
            markSet(candidates, candidates[idxGroup]);
            int setSize = tempSetList[idxGroup].cardlist_.size();
            if (setSize > c_minimum)
            {
                //套牌与潜在顺子重合
                if (tempMeldList[idxMeld].getGoal() > 0)
                {
                    int rank = tempSetList[idxGroup].cardlist_[0]->rank_;
                    auto itCard = std::find_if(tempMeldList[idxMeld].cardlist_.begin(), tempMeldList[idxMeld].cardlist_.end(),
                                               [rank](const PCard &card) { return card->rank_ == rank; });                                  
                    tempSetList[idxGroup].removeCard((*itCard)->id_);
                    candidates[idxMeld].idxSetMember_ = c_idxerror;
                    setList.push_back(tempSetList[idxGroup]);
                    removeSetFromCandidates(candidates);
                    
                    candidates[idxMeld].removeGroup(tempMeldList[idxMeld]);
                    PCard special;
                    if (tryBuildRunWithSpecial(candidates[0], tempMeldList[idxMeld], special))
                    {
                        tempMeldList[idxMeld].cardlist_.push_back(special);
                        meldList.push_back(tempMeldList[idxMeld]);
                        candidates[0].removeCard(special);
                    }
                    else
                    {
                        tempMeldList[idxMeld].cardlist_.push_back(candidates[0].cardlist_[0]);
                        meldList.push_back(tempMeldList[idxMeld]);
                        candidates[0].cardlist_.erase(candidates[0].cardlist_.begin());
                    }
                }
                else
                {
                    setList.push_back(tempSetList[idxGroup]);
                    removeSetFromCandidates(candidates);
                }
            }
            else if (setSize == c_minimum)
            {
                setList.push_back(tempSetList[idxGroup]);
                removeSetFromCandidates(candidates);
            }
            else if (setSize == c_minimum - 1)
            {
                tempSetList[idxGroup].cardlist_.push_back(candidates[0].cardlist_[0]);
                candidates[0].cardlist_.erase(candidates[0].cardlist_.begin());
                setList.push_back(tempSetList[idxGroup]);
                removeSetFromCandidates(candidates);
            }
        }
    }
    return 0;
}

//选出同花色潜在顺子
int GameRummy::buildMeldFromTop(std::vector<CardGroup> &candidates, CardGroup &group, CardGroup &meld)
{
    if (group.cardlist_.size() < c_minimum - 1 || candidates[0].cardlist_.size() == 0)
    {
        return 0;
    }
    meld.reset();
    auto it = group.cardlist_.begin();
    if ((*it)->rank_ - (*(it + 1))->rank_ == 1 || (*it)->rank_ - (*(it + 1))->rank_ == 2)
    {
        meld.cardlist_.push_back(*it);
        meld.cardlist_.push_back(*(it + 1));
        return meld.getGoal();
    }
    else if ((*it)->rank_ - (*(it + 1))->rank_ == 0)
    {
        ++it;
        const PCard sameRank = (*(group.cardlist_.begin()));
        group.cardlist_.erase(group.cardlist_.begin());
        buildMeldFromTop(candidates, group, meld);
        group.cardlist_.insert(group.cardlist_.begin(), sameRank);
        return meld.getGoal();
    }
    else
    {
        return 0;
    }
}

//选出潜在套牌
int GameRummy::buildSetFromTop(std::vector<CardGroup> &candidates, CardGroup &group, CardGroup &set)
{
    set.reset();
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
                      }
                  });

    int nMember = tempSet.cardlist_.size();
    if (nMember < c_minimum - 1)
    {
        return 0;
    }
    else if (nMember == c_minimum - 1)
    {
        if (candidates[0].cardlist_.size() == 0)
        {
            return 0;
        }
        else
        {
            set = std::move(tempSet);
            return set.getGoal();
        }
    }
    else
    {
        set = std::move(tempSet);
        return set.getGoal();
    }
}

void GameRummy::markSet(std::vector<CardGroup> &candidates, CardGroup &group)
{
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
}

bool GameRummy::tryBuildRunWithSpecial(CardGroup &specialGroup, CardGroup &meldCandidate, PCard &special)
{
    std::for_each(specialGroup.cardlist_.begin(), specialGroup.cardlist_.end(),
                  [&meldCandidate, &special](const PCard &card) {
                      if (meldCandidate.expandToRun(card))
                      {
                          special = card;
                          return true;
                      }
                  });
    return false;
}

int GameRummy::buildCandidates(std::vector<CardGroup> &runList, std::vector<CardGroup> &meldList, std::vector<CardGroup> &setList,
                               std::vector<CardGroup> &candidates)
{
    int nMagic = candidates[0].cardlist_.size();
    if (nMagic == 0)
    {
        return 0;
    }
    std::sort(candidates[0].cardlist_.begin(), candidates[0].cardlist_.end(), [](const PCard &a, const PCard &b) { return a > b; });
    vector<int> goalList(c_minimum);
    for (auto idx = 1; idx != c_minimum; ++idx)
    {
        if (candidates[idx].cardlist_.size() != 0)
        {
            goalList[idx] = candidates[idx].cardlist_[0]->rank_;
        }
        else
        {
            goalList[idx] = 0;
        }
    }

    auto itMax = std::max_element(goalList.begin() + 1, goalList.end());
    int idxMax = itMax - goalList.begin();
    if (nMagic == 1)
    {
        std::for_each(meldList.begin(), meldList.end(), [&candidates, &meldList](CardGroup &group) {
            group.expandToGroup(*(candidates[0].cardlist_.begin()));
        });
        std::for_each(setList.begin(), setList.end(), [&candidates, &meldList](CardGroup &group) {
            group.expandToGroup(*(candidates[0].cardlist_.begin()));
        });
    }
    else if (nMagic == 2)
    {
        CardGroup newGroup = candidates[0];
        newGroup.cardlist_.insert(newGroup.cardlist_.begin(), candidates[idxMax].cardlist_[0]);
        candidates[idxMax].cardlist_.erase(candidates[idxMax].cardlist_.begin());
    }
    else
    {
        //extreme condition should test more
        do
        {
            nMagic -= 2;
            goalList.erase(goalList.begin() + idxMax);
            candidates[0].cardlist_.erase(candidates[0].cardlist_.begin(), candidates[0].cardlist_.begin() + 2);
            if (nMagic >= 2)
            {
                itMax = std::max_element(goalList.begin() + 1, goalList.end());
                idxMax = itMax - goalList.begin();
                CardGroup newGroup = candidates[0];
                newGroup.cardlist_.insert(newGroup.cardlist_.begin(), candidates[idxMax].cardlist_[0]);
                candidates[idxMax].cardlist_.erase(candidates[idxMax].cardlist_.begin());
            }
            else if (nMagic == 1)
            {
                std::for_each(meldList.begin(), meldList.end(), [&candidates, &meldList](CardGroup &group) {
                    group.expandToGroup(*(candidates[0].cardlist_.begin()));
                });
                std::for_each(setList.begin(), setList.end(), [&candidates, &meldList](CardGroup &group) {
                    group.expandToGroup(*(candidates[0].cardlist_.begin()));
                });
            }

        } while (nMagic > 0);
    }

    return 0;
}
