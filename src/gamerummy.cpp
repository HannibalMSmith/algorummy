#include "gamerummy.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <numeric>

using std::cout;
using std::endl;
using std::map;
using std::pair;
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
    return std::accumulate(cardlist_.begin(), cardlist_.end(), 0,
                           [](int goal, const PCard &card) { return goal + card->rank_; });
}

bool CardGroup::haveRank(int rank, PCard &card)
{
}

bool CardGroup::expandToRun(const PCard &card)
{
    assert(card != NULL);
    if (card->suit_ != cardlist_[0]->suit_)
    {
        return false;
    }
    if (cardlist_[0]->rank_ == 13 && cardlist_[1]->rank_ == 12 && card->rank_ == 1)
    {
        cardlist_.insert(cardlist_.begin(), card);
        return true;
    }
    else if (cardlist_[0]->rank_ + 1 == card->rank_ && cardlist_[0]->rank_ - cardlist_[1]->rank_ == 1)
    {
        cardlist_.insert(cardlist_.begin(), card);
        return true;
    }
    else if (cardlist_[0]->rank_ - 1 == card->rank_ && cardlist_[0]->rank_ - cardlist_[1]->rank_ == 2)
    {
        cardlist_.insert(cardlist_.begin() + 1, card);
        return true;
    }
    else if ((*(cardlist_.end() - 1))->rank_ - 1 == card->rank_ &&
             (*(cardlist_.end() - 2))->rank_ - 1 == (*(cardlist_.end() - 1))->rank_)
    {
        cardlist_.push_back(card);
        return true;
    }
    else
    {
        return false;
    }
}

bool CardGroup::expandToMeld(const PCard &card)
{
    assert(card != NULL);
    if (card->special_)
    {
        if (expandToRun(card))
        {
            return true;
        }
    }
    if (card->magic_ || card->special_)
    {
        assert(cardlist_.size() >= c_minimum - 1);
        if (cardlist_.size() > c_minimum - 1)
        {
            if (cardlist_[0]->rank_ - cardlist_[1]->rank_ == 0)
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
            else
            {
                cardlist_.push_back(card);
                return true;
            }
        }
        else if (cardlist_.size() == c_minimum - 1)
        {
            if (cardlist_[0]->rank_ - cardlist_[1]->rank_ == 1)
            {
                cardlist_.push_back(card);
                return true;
            }
            else if (cardlist_[0]->rank_ - cardlist_[1]->rank_ == 2)
            {
                cardlist_.insert(cardlist_.begin() + 1, card);
                return true;
            }
        }
    }
    return false;
}

void GameRummy::printCardGroup(const CardGroup &group, int specialRank)
{
    if (group.cardlist_.size() == 0)
    {
        return;
    }
    cout << "分组编号: " << group.id_ << " 卡牌列表" << endl;
    std::for_each(group.cardlist_.begin(), group.cardlist_.end(),
                  [specialRank](const PCard &card) {
                      string name = getCardString(*card);
                      cout << name;
                      if (card->rank_ == specialRank && specialRank != c_idxerror)
                      {
                          cout << " 金";
                      }
                      cout << endl;
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
        else if (card.rank_ == -1)
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

int GameRummy::match(const map<int, PCard> &hand, vector<CardGroup> &runList, vector<CardGroup> &meldList,
                     vector<CardGroup> &setList, vector<CardGroup> &unmatchedList)
{
    runList.clear();
    meldList.clear();
    setList.clear();
    unmatchedList.clear();
    vector<CardGroup> candidates;
    partitionHand(hand, candidates);
    initRunList(candidates, runList);
    buildGroupByScore(candidates, runList, meldList, setList);
    arrangeUnmatched(candidates, unmatchedList);
    return 0;
}

int GameRummy::partitionHand(const std::map<int, PCard> &hand, std::vector<CardGroup> &candidates)
{
    candidates.resize(c_suit);
    std::for_each(hand.begin(), hand.end(),
                  [&candidates](const pair<const int, PCard> &item) {
                      if (item.second->special_ || item.second->magic_)
                      {
                          candidates[e_joker].cardlist_.push_back(item.second);
                      }
                      else
                      {
                          candidates[item.second->suit_].cardlist_.push_back(item.second);
                      }
                  });
    std::for_each(candidates.begin(), candidates.end(),
                  [](CardGroup &group) {
                      std::sort(group.cardlist_.begin(), group.cardlist_.end(), GameRummy::greater4CardOrder);
                  });
    std::sort(candidates[e_joker].cardlist_.begin(), candidates[e_joker].cardlist_.end(), GameRummy::less4CardOrder);
    return 0;
}

int GameRummy::initRunList(std::vector<CardGroup> &candidates, std::vector<CardGroup> &runList)
{
    std::for_each(candidates.begin() + 1, candidates.end(), [&runList](CardGroup &group) { buildRun(group, runList); });
#ifdef DEBUGMORE
    {
        cout << "初选同花组***********************" << endl;
        std::for_each(runList.begin(), runList.end(), std::bind(GameRummy::printCardGroup, std::placeholders::_1, c_idxerror));
        cout << "初选同花组***********************" << endl;
    }
#endif
    return 0;
}

int GameRummy::buildRun(CardGroup &group, std::vector<CardGroup> &runList)
{
    int groupSize = group.cardlist_.size();
    if (groupSize < c_minimum)
    {
        return 0;
    }
    CardGroup tmpMatched;
    auto last = group.cardlist_.begin();
    tmpMatched.cardlist_.push_back(*last);
    for (auto it = last + 1; it < group.cardlist_.end(); ++it)
    {
#ifdef DEBUGMORE
        cout << "last: " << (*last)->suit_ << ":" << (*last)->rank_;
        cout << " it: " << (*it)->suit_ << ":" << (*it)->rank_ << endl;
#endif
        if ((*last)->rank_ == (*it)->rank_)
        {
            continue;
        }
        else if ((*last)->rank_ - (*it)->rank_ == 1)
        {
            last = it;
            tmpMatched.cardlist_.push_back(*it);
            if (tmpMatched.cardlist_.size() == c_minimum)
            {
                runList.push_back(tmpMatched);
                group.removeGroup(tmpMatched);
                break;
            }
        }
        else
        {
            if (tmpMatched.cardlist_.size() == c_minimum - 1 &&
                tmpMatched.cardlist_[0]->rank_ == 13 && tmpMatched.cardlist_[1]->rank_ == 12)
            {
                PCard ace;
                group.haveRank(1, ace);
                if (ace != NULL)
                {
                    tmpMatched.cardlist_.insert(tmpMatched.cardlist_.begin(), ace);
                    runList.push_back(tmpMatched);
                    group.removeGroup(tmpMatched);
                    break;
                }
            }
            else
            {
                tmpMatched.cardlist_.clear();
                last = it;
                if (group.cardlist_.size() < c_minimum)
                {
                    break;
                }
                else
                {
                    continue;
                }
            }
        }
    }
    return runList.size();
}

int GameRummy::buildGroupByScore(std::vector<CardGroup> &candidates, std::vector<CardGroup> &runList, std::vector<CardGroup> &meldList, std::vector<CardGroup> &setList)
{
    vector<CardGroup> tempList(c_suit);
    while (true)
    {
#ifdef DEBUGMORE
        {
            cout << "候选组#######################" << endl;
            std::for_each(candidates.begin(), candidates.end(), std::bind(GameRummy::printCardGroup, std::placeholders::_1, c_idxerror));
            cout << "候选组#######################" << endl;
        }
#endif
        vector<CardGroup> tempRunList(c_suit);
        vector<CardGroup> tempSetList(c_suit);
        vector<CardGroup> tempMeldList(c_suit);
        vector<int> topGoal(c_suit);
        vector<int> runGoal(c_suit);
        vector<int> meldGoal(c_suit);
        vector<int> setGoal(c_suit);
        for (auto idx = 1; idx < c_suit; ++idx)
        {
            if (candidates[idx].cardlist_.size() == 0)
            {
                topGoal[idx] = 0;
            }
            else
            {
                topGoal[idx] = candidates[idx].cardlist_[0]->rank_;
            }
            runGoal[idx] = buildRunFromTop(candidates[idx], tempRunList[idx]);
            setGoal[idx] = buildSetFromTop(candidates, candidates[idx], tempSetList[idx]);
            meldGoal[idx] = buildMeldFromTop(candidates, candidates[idx], tempMeldList[idx]);
        }
        auto itMaxTop = std::max_element(topGoal.begin() + 1, topGoal.end());
        int idxMaxTop = itMaxTop - topGoal.begin();
        if (*itMaxTop == 0)
        {
            break;
        }
        if (runGoal[idxMaxTop] == 0 && setGoal[idxMaxTop] == 0)
        {
            auto rankTopMax = candidates[idxMaxTop].cardlist_[0]->rank_;
            if (expandTopMaxToRunlist(runGoal, rankTopMax, candidates, runList) > 0)
            {
                continue;
            }
            CardGroup set;
            if (buildMinorSet(candidates, idxMaxTop, set) == 0 && meldGoal[idxMaxTop] == 0)
            {
                int suit = candidates[idxMaxTop].cardlist_[0]->suit_;
                tempList[suit].cardlist_.push_back(candidates[idxMaxTop].cardlist_[0]);
                candidates[idxMaxTop].cardlist_.erase(candidates[idxMaxTop].cardlist_.begin());
                continue;
            }
        }

        auto itMaxRun = std::max_element(runGoal.begin() + 1, runGoal.end());
        int idxMaxRun = itMaxRun - runGoal.begin();
        auto itMaxSet = std::max_element(setGoal.begin() + 1, setGoal.end());
        int idxMaxSet = itMaxSet - setGoal.begin();
        auto itMaxMeld = std::max_element(meldGoal.begin() + 1, meldGoal.end());
        int idxMaxMeld = itMaxMeld - meldGoal.begin();

        if (*itMaxRun != 0 && *itMaxSet == 0)
        {
            runList.push_back(tempRunList[idxMaxRun]);
            candidates[idxMaxRun].removeGroup(tempRunList[idxMaxRun]);
        }
        else if (*itMaxRun != 0 && *itMaxSet != 0)
        {
            int rank = tempSetList[idxMaxSet].cardlist_[0]->rank_;
            PCard cardInBoth;
            tempRunList[idxMaxRun].haveRank(rank, cardInBoth);
            if (cardInBoth == NULL)
            {
                runList.push_back(tempRunList[idxMaxRun]);
                candidates[idxMaxRun].removeGroup(tempRunList[idxMaxRun]);
                continue;
            }
            else
            {
                if (tempSetList[idxMaxSet].cardlist_.size() > c_minimum)
                {
                    runList.push_back(tempRunList[idxMaxRun]);
                    candidates[idxMaxRun].removeGroup(tempRunList[idxMaxRun]);
                    continue;
                }
                else if (tempSetList[idxMaxSet].cardlist_.size() == c_minimum)
                {
                    if (runList.size() + meldList.size() >= 2 && *itMaxSet > *itMaxRun)
                    {
                        setList.push_back(tempSetList[idxMaxSet]);
                        markSet(candidates, tempSetList[idxMaxSet]);
                        removeSetFromCandidates(candidates);
                        continue;
                    }
                    else
                    {
                        runList.push_back(tempRunList[idxMaxRun]);
                        candidates[idxMaxRun].removeGroup(tempRunList[idxMaxRun]);
                        continue;
                    }
                }
                else
                {
                    assert("fatal:invalid set");
                }
            }
        }
        else if (*itMaxRun == 0 && *itMaxSet != 0)
        {
            if (tempSetList[idxMaxSet].cardlist_.size() > c_minimum)
            {
                if (*itMaxMeld != 0)
                {
                    int rank = tempSetList[idxMaxSet].cardlist_[0]->rank_;
                    PCard cardInBoth;
                    tempMeldList[idxMaxMeld].haveRank(rank, cardInBoth);
                    tempSetList[idxMaxSet].removeCard(cardInBoth->id_);
                    candidates[idxMaxMeld].idxSetMember_ = c_idxerror;
                }
                setList.push_back(tempSetList[idxMaxSet]);
                markSet(candidates, tempSetList[idxMaxSet]);
                removeSetFromCandidates(candidates);
                continue;
            }
            else if (tempSetList[idxMaxSet].cardlist_.size() == c_minimum)
            {
                if (*itMaxMeld == 0)
                {
                    setList.push_back(tempSetList[idxMaxSet]);
                    markSet(candidates, tempSetList[idxMaxSet]);
                    removeSetFromCandidates(candidates);
                    continue;
                }
                else
                {
                    PCard card;
                    int rank = tempSetList[idxMaxSet].cardlist_[0]->rank_;
                    tempMeldList[idxMaxMeld].haveRank(rank, card);
                    if (card != NULL)
                    {
                        if (runList.size() + meldList.size() >= 2)
                        {
                            setList.push_back(tempSetList[idxMaxSet]);
                            markSet(candidates, tempSetList[idxMaxSet]);
                            removeSetFromCandidates(candidates);
                            continue;
                        }
                        else
                        {
                            buildRunOrMeld(candidates, runList, meldList, tempMeldList[idxMaxMeld], idxMaxMeld);
                            continue;
                        }
                    }
                    else
                    {
                        if (*itMaxMeld > *itMaxSet)
                        {
                            buildRunOrMeld(candidates, runList, meldList, tempMeldList[idxMaxMeld], idxMaxMeld);
                            continue;
                        }
                        else
                        {
                            setList.push_back(tempSetList[idxMaxSet]);
                            markSet(candidates, tempSetList[idxMaxSet]);
                            removeSetFromCandidates(candidates);
                            continue;
                        }
                    }
                }
            }
        }
        else
        {
            for (auto idx = 1; idx < c_suit; ++idx)
            {
                if (candidates[idx].cardlist_.size() == 0)
                {
                    topGoal[idx] = 0;
                }
                else
                {
                    topGoal[idx] = candidates[idx].cardlist_[0]->rank_;
                }
            }
            itMaxTop = std::max_element(topGoal.begin() + 1, topGoal.end());
            idxMaxTop = itMaxTop - topGoal.begin();
            if (itMaxTop == topGoal.end())
            {
                break;
            }
            CardGroup minorSet;
            buildMinorSet(candidates, idxMaxTop, minorSet);
            if (minorSet.getGoal() > tempMeldList[idxMaxTop].getGoal() && 
                runList.size() + meldList.size() >= 2)
            {
                markSet(candidates, minorSet);
                minorSet.cardlist_.push_back(candidates[e_joker].cardlist_[0]);
                setList.push_back(minorSet);
                removeSetFromCandidates(candidates);
                candidates[e_joker].removeCard(candidates[e_joker].cardlist_[0]);
                continue;
            }
            else if(tempMeldList[idxMaxTop].getGoal() > minorSet.getGoal())
            {
                candidates[idxMaxTop].removeGroup(tempMeldList[idxMaxTop]);
                tempMeldList[idxMaxTop].cardlist_.push_back(candidates[e_joker].cardlist_[0]);
                setList.push_back(tempMeldList[idxMaxTop]);
                candidates[e_joker].removeCard(candidates[e_joker].cardlist_[0]);
                continue;
            }
            else if(minorSet.getGoal() == 0 && tempMeldList[idxMaxTop].getGoal() == 0)
            {
                tempList[idxMaxTop].cardlist_.push_back(candidates[idxMaxTop].cardlist_[0]);
                candidates[idxMaxTop].removeCard(candidates[idxMaxTop].cardlist_[0]);
            }

        }
    }
    std::copy(tempList.begin() + 1, tempList.end(), candidates.begin() + 1);
    return 0;
}

int GameRummy::buildRunFromTop(CardGroup &group, CardGroup &run)
{
    if (group.cardlist_.size() < c_minimum)
    {
        return 0;
    }
    CardGroup tmpMatched;
    auto last = group.cardlist_.begin();
    tmpMatched.cardlist_.push_back(*last);
    for (auto it = last + 1; it < group.cardlist_.end(); ++it)
    {
        if ((*last)->rank_ == (*it)->rank_)
        {
            continue;
        }
        else if ((*last)->rank_ - (*it)->rank_ == 1)
        {
            tmpMatched.cardlist_.push_back(*it);
            last = it;
            if (tmpMatched.cardlist_.size() == c_minimum)
            {
                break;
            }
        }
        else
        {
            tmpMatched.cardlist_.clear();
            break;
        }
    }
    if (tmpMatched.cardlist_.size() != c_minimum)
    {
        tmpMatched.cardlist_.clear();
        return 0;
    }
    run = std::move(tmpMatched);
    return run.getGoal();
}

int GameRummy::expandTopMaxToRunlist(const vector<int> &runGoal, int rank, std::vector<CardGroup> &candidates, std::vector<CardGroup> &runList)
{
    int count = 0;
    std::for_each(runList.begin(), runList.end(), [&candidates, runGoal, &count, rank](CardGroup &runGroup) {
        for (int i = 1; i < c_suit; ++i)
        {
            auto cardTop = candidates[i].cardlist_[0];
            if (cardTop->rank_ == rank && runGoal[i] != 0)
            {
                if (runGroup.expandToRun(cardTop))
                {
                    candidates[i].removeCard(cardTop);
                    count++;
                }
            }
        }
    });
    return count;
}

int GameRummy::buildMinorSet(std::vector<CardGroup> &candidates, int idx, CardGroup &set)
{
    set.reset();
    int rank = candidates[idx].cardlist_[0]->rank_;
    for (auto i = 1; i != c_suit; ++i)
    {
        if (i == idx || candidates[i].cardlist_.size() == 0)
        {
            continue;
        }
        PCard card;
        candidates[i].haveRank(rank, card);
        if (card != NULL)
        {
            set.cardlist_.push_back(card);
            set.cardlist_.push_back(candidates[idx].cardlist_[0]);
        }
    }
    return set.cardlist_.size();
}

int GameRummy::buildSetFromTop(vector<CardGroup> &candidates, CardGroup &group, CardGroup &set)
{
    if (group.cardlist_.size() == 0)
    {
        return 0;
    }
    int rank = group.cardlist_[0]->rank_;
    CardGroup tempSet;
    std::for_each(candidates.begin(), candidates.end(),
                  [&tempSet, rank](CardGroup &group) {
                      PCard card;
                      group.haveRank(rank, card);
                      if (card != NULL)
                      {
                          tempSet.cardlist_.push_back(card);
                      }
                  });
    int nMember = tempSet.cardlist_.size();
    if (nMember < c_minimum)
    {
        return 0;
    }
    else
    {
        set = std::move(tempSet);
        return set.getGoal();
    }
}
void GameRummy::markSet(vector<CardGroup> &candidates, CardGroup &group)
{
    int rank = group.cardlist_[0]->rank_;
    CardGroup tempSet;
    std::for_each(candidates.begin(), candidates.end(),
                  [&tempSet, rank](CardGroup &group) {
                      auto itCard = std::find_if(group.cardlist_.begin(), group.cardlist_.end(), [rank](const PCard &card) { return card->rank_ == rank; });
                      if (itCard != group.cardlist_.end())
                      {
                          tempSet.cardlist_.push_back(*itCard);
                          group.idxSetMember_ = itCard - group.cardlist_.begin();
                      }
                  });
}

void GameRummy::removeSetFromCandidates(vector<CardGroup> &candidates)
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

int GameRummy::buildMeldFromTop(vector<CardGroup> &candidates, CardGroup &group, CardGroup &meld)
{
    if (group.cardlist_.size() < c_minimum - 1 || candidates[0].cardlist_.size() == 0)
    {
        return 0;
    }
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
        const PCard duiplicate = (*(group.cardlist_.begin()));
        group.removeCard(group.cardlist_[0]);
        buildMeldFromTop(candidates, group, meld);
        group.cardlist_.insert(group.cardlist_.begin(), duiplicate);
        return meld.getGoal();
    }
    else
    {
        return 0;
    }
}

int GameRummy::buildMeldWithAceFromTop(vector<CardGroup> &candidates, CardGroup &group, CardGroup &meld)
{
    meld.reset();
    if (group.cardlist_.size() < c_minimum - 1 || candidates[0].cardlist_.size() == 0)
    {
        return 0;
    }
    if (group.cardlist_[0]->rank_ != 12 && (*(group.cardlist_.end() - 1))->rank_ != 1)
    {
        return 0;
    }
    meld.cardlist_.push_back(*(group.cardlist_.end() - 1));
    meld.cardlist_.push_back(group.cardlist_[0]);
    return meld.getGoal();
}

void GameRummy::buildRunOrMeld(vector<CardGroup> &candidates, vector<CardGroup> &runList,
                               vector<CardGroup> &meldList, CardGroup &group, int idxInCandidates)
{
    assert(group.cardlist_.size() == 2 && candidates[e_joker].cardlist_.size() > 0);
    CardGroup newGroup;
    PCard special;
    candidates[idxInCandidates].removeGroup(group);
    if (tryBuildRunWithSpecial(candidates[e_joker], group, newGroup, special))
    {
        runList.push_back(newGroup);
        candidates[e_joker].removeCard(special->id_);
    }
    else
    {
        group.cardlist_.push_back(candidates[e_joker].cardlist_[0]);
        meldList.push_back(group);
        candidates[e_joker].removeCard(candidates[0].cardlist_[0]);
    }
}

bool GameRummy::tryBuildRunWithSpecial(CardGroup &specialGroup, CardGroup &meldCandidate, CardGroup &newGroup, PCard &special)
{
    bool ret = false;
    std::for_each(specialGroup.cardlist_.begin(), specialGroup.cardlist_.end(),
                  [&meldCandidate, &special, &newGroup, &ret](const PCard &card) {
                      if (meldCandidate.expandToRun(card))
                      {
                          special = card;
                          newGroup = meldCandidate;
                          ret = true;
                      }
                  });
    return ret;
}

int GameRummy::arrangeUnmatched(std::vector<CardGroup> &unmatchedList, std::vector<CardGroup> &newList)
{
    return 0;
}

int GameRummy::buildMeldWithRank(vector<CardGroup> &candidates, CardGroup &group, int rank)
{
    if (group.cardlist_.size() < c_minimum - 1 || candidates[0].cardlist_.size() == 0)
    {
        return 0;
    }
    int numCheat = candidates[0].cardlist_.size();
    CardGroup groupCopy(group);
    CardGroup temp;
    auto target = groupCopy.cardlist_.end();
    for (auto it = groupCopy.cardlist_.begin(); it <= groupCopy.cardlist_.end() - 2; ++it)
    {
        if (numCheat <= 0)
        {
            break;
        }
        if ((*it)->rank_ - (*(it + 1))->rank_ == 0)
        {
            continue;
        }
        else if (((*it)->rank_ == 13 && (*(groupCopy.cardlist_.end() - 1))->rank_ == 1) ||
                 ((*it)->rank_ == 12 && (*(groupCopy.cardlist_.end() - 1))->rank_ == 1))
        {
            groupCopy.cardlist_.erase(groupCopy.cardlist_.end() - 1);
            if ((*it)->rank_ == rank)
            {
                temp.cardlist_.push_back((*it));
                temp.cardlist_.push_back((*(it + 1)));
                target = it;
                numCheat--;
                break;
            }
            else if ((*(it + 1))->rank_ == rank)
            {
                temp.cardlist_.push_back((*it));
                temp.cardlist_.push_back((*(it + 1)));
                target = it + 1;
                numCheat--;
                break;
            }
            else
            {
                ++it;
                numCheat--;
                continue;
            }
        }
        else if ((*it)->rank_ - (*(it + 1))->rank_ == 1 || (*it)->rank_ - (*(it + 1))->rank_ == 2)
        {
            if ((*it)->rank_ == rank)
            {
                temp.cardlist_.push_back((*it));
                temp.cardlist_.push_back((*(it + 1)));
                target = it;
                numCheat--;
                break;
            }
            else if ((*(it + 1))->rank_ == rank)
            {
                temp.cardlist_.push_back((*it));
                temp.cardlist_.push_back((*(it + 1)));
                target = it + 1;
                numCheat--;
                break;
            }
            else
            {
                ++it;
                numCheat--;
                continue;
            }
        }
        else
        {
            continue;
        }
    }
    if (temp.cardlist_.size() == 0)
    {
        return 0;
    }
    else
    {
        return temp.getGoal();
    }
}

int GameRummy::findCardInBothRunAndSet(std::vector<CardGroup> &tempRunList, int rankSet, PCard &cardInBoth, int &runInCandidates)
{
    vector<vector<PCard>::iterator> itList(c_suit);
    vector<int> goal(c_suit);
    for (auto i = 1; i < c_suit; i++)
    {
        itList[i] = std::find_if(tempRunList[i].cardlist_.begin(), tempRunList[i].cardlist_.end(), [rankSet](const PCard &card) { return card->rank_ == rankSet; });
        if (itList[i] == tempRunList[i].cardlist_.end())
        {
            goal[i] = 0;
        }
        else
        {
            goal[i] = tempRunList[i].getGoal();
        }
    }
    auto itMax = std::max_element(goal.begin(), goal.end());
    if (*itMax == 0)
    {
        return 0;
    }
    else
    {
        runInCandidates = itMax - goal.begin();
        cardInBoth = *(itList[runInCandidates]);
        return goal[runInCandidates];
    }
}

int GameRummy::findCardInBothMeldAndSet(vector<CardGroup> &candidates, vector<CardGroup> &meldList, int rank, PCard &card, int &meldInCandidates)
{
    vector<CardGroup> tempMeldList(c_suit);
    vector<int> meldGoal(c_suit);
    for (auto i = 1; i < c_suit; ++i)
    {
        meldGoal[i] = buildMeldWithRank(candidates, candidates[i], rank);
    }
    auto itMax = std::max_element(meldGoal.begin(), meldGoal.end());
    if (*itMax == 0)
    {
        return 0;
    }
    int idxMax = itMax - meldGoal.begin();
    auto itCardInBoth = std::find_if(candidates[idxMax].cardlist_.begin(), candidates[idxMax].cardlist_.end(),
                                     [rank](const PCard &card) { return card->rank_ == rank; });
    card = *itCardInBoth;
    meldInCandidates = idxMax;
    return meldGoal[idxMax];
}

int maxInThree(int a, int b, int c)
{
    int max = a;
    if (b > max)
    {
        max = b;
    }
    if (c > max)
    {
        max = c;
    }
    return max;
}

int GameRummy::getMaxCandidate(vector<CardGroup> &candidates, PCard &card)
{
    vector<int> goalList(c_suit);
    for (auto idx = 1; idx != c_suit; ++idx)
    {
        if (candidates[idx].cardlist_.size() == 0)
        {
            goalList[idx] = 0;
        }
        else
        {
            goalList[idx] = candidates[idx].cardlist_[0]->rank_;
        }
    }

    auto itMax = std::max_element(goalList.begin(), goalList.end());
    if (*itMax == 0)
    {
        card = NULL;
        return c_idxerror;
    }
    int idxMax = itMax - goalList.begin();
    card = candidates[idxMax].cardlist_[0];
    return idxMax;
}

bool GameRummy::expandToRunAndDelSource(vector<CardGroup> &runList, CardGroup &srcGroup)
{
    if (runList.size() == 0)
    {
        return false;
    }

    bool success = false;
    std::for_each(runList.begin(), runList.end(), [&success, &srcGroup](CardGroup &group) {
        for (PCard &card : group.cardlist_)
        {
            if (group.expandToRun(card))
            {
                srcGroup.removeCard(card);
                success = true;
                break;
            }
        }
    });
    return success;
}

bool GameRummy::expandToMeldAndDelSource(vector<CardGroup> &targetList, CardGroup &src, int idx)
{
    if (src.cardlist_.size() == 0)
    {
        return false;
    }
    bool success = false;
    for (auto &group : targetList)
    {
        if (group.expandToMeld(src.cardlist_[idx]))
        {
            success = true;
            break;
        }
    }
    if (success)
    {
        src.removeCard(src.cardlist_[0]);
    }
    return success;
}

int GameRummy::buildCandidates(vector<CardGroup> &runList, vector<CardGroup> &meldList, vector<CardGroup> &setList,
                               vector<CardGroup> &candidates)
{

    int nCheat = candidates[0].cardlist_.size();
    if (nCheat == 0)
    {
        return 0;
    }
    expandToRunAndDelSource(runList, candidates[0]);
    nCheat = candidates[0].cardlist_.size();
    if (nCheat == 0)
    {
        return 0;
    }
    do
    {
        PCard card = NULL;
        nCheat = candidates[0].cardlist_.size();
        int idx = getMaxCandidate(candidates, card);
        if (idx == c_idxerror)
        {
            if (!expandToMeldAndDelSource(setList, candidates[0], 0) &&
                !expandToMeldAndDelSource(meldList, candidates[0], 0))
            {
                break;
            }
        }
        nCheat = candidates[0].cardlist_.size();
        if (nCheat >= c_minimum - 1 && idx != c_idxerror)
        {
            CardGroup newMeld;
            newMeld.cardlist_.insert(newMeld.cardlist_.end(), candidates[0].cardlist_.begin(), candidates[0].cardlist_.begin() + 2);
            candidates[0].removeGroup(newMeld);
            newMeld.cardlist_.push_back(card);
            meldList.push_back(newMeld);
            candidates[idx].removeCard(card);
        }
        else if (nCheat == 1)
        {
            if (!expandToMeldAndDelSource(setList, candidates[0], 0) &&
                !expandToMeldAndDelSource(meldList, candidates[0], 0))
            {
                break;
            }
        }
        else
        {
            break;
        }
    } while (true);

    return 0;
}

void GameRummy::arrangeUnmatchList(std::vector<CardGroup> &unmatchedList, std::vector<CardGroup> &newList)
{
    vector<int> goalTop(c_suit);
    vector<CardGroup> tempList(c_suit);
    while (true)
    {
        for (auto i = 1; i < c_suit; ++i)
        {
            if (unmatchedList[i].cardlist_.size() == 0)
            {
                goalTop[i] = 0;
            }
            else
            {
                goalTop[i] = unmatchedList[i].cardlist_[0]->rank_;
            }
        }
        auto itMax = std::max_element(goalTop.begin(), goalTop.end());
        if ((*itMax) == 0)
        {
            break;
        }

        int idx = itMax - goalTop.begin();
        assert(unmatchedList[idx].cardlist_.size() != 0);
        if (unmatchedList[idx].cardlist_.size() == 1)
        {
            buildMinorSet(unmatchedList, idx, tempList, newList);
        }
        else
        {
            if (unmatchedList[idx].cardlist_[0]->rank_ == 13)
            {
                auto itAce = std::find_if(unmatchedList[idx].cardlist_.begin(), unmatchedList[idx].cardlist_.end(), [](const PCard &item) { return item->rank_ == 1; });
                if (itAce != unmatchedList[idx].cardlist_.end())
                {
                    CardGroup temp;
                    temp.cardlist_.push_back(*itAce);
                    temp.cardlist_.push_back(unmatchedList[idx].cardlist_[0]);
                    newList.push_back(std::move(temp));
                    unmatchedList[idx].cardlist_.erase(itAce);
                    unmatchedList[idx].cardlist_.erase(unmatchedList[idx].cardlist_.begin());
                    continue;
                }
            }
            if (unmatchedList[idx].cardlist_[0]->rank_ - unmatchedList[idx].cardlist_[1]->rank_ == 0)
            {
                tempList[idx].cardlist_.push_back(unmatchedList[idx].cardlist_[0]);
                unmatchedList[idx].cardlist_.erase(unmatchedList[idx].cardlist_.begin());
            }
            else if (unmatchedList[idx].cardlist_[0]->rank_ - unmatchedList[idx].cardlist_[1]->rank_ == 1)
            {
                CardGroup temp;
                temp.cardlist_.insert(temp.cardlist_.end(), unmatchedList[idx].cardlist_.begin(), unmatchedList[idx].cardlist_.begin() + 2);
                newList.push_back(std::move(temp));
                unmatchedList[idx].cardlist_.erase(unmatchedList[idx].cardlist_.begin(), unmatchedList[idx].cardlist_.begin() + 2);
            }
            else
            {
                buildMinorSet(unmatchedList, idx, tempList, newList);
            }
        }
    }
    std::for_each(tempList.begin(), tempList.end(), [&newList](const CardGroup &group) { newList.push_back(group); });
    newList.push_back(unmatchedList[0]);
}

bool GameRummy::buildMinorSet(vector<CardGroup> &unmatchedList, int idx, vector<CardGroup> &tempList, vector<CardGroup> &newList)
{
    int rankSet = unmatchedList[idx].cardlist_[0]->rank_;
    bool match = false;
    for (auto i = 1; i < c_suit; i++)
    {
        if (i == idx)
        {
            continue;
        }
        auto itSet = std::find_if(unmatchedList[i].cardlist_.begin(), unmatchedList[i].cardlist_.end(), [rankSet](const PCard &item) { return item->rank_ == rankSet; });
        if (itSet != unmatchedList[i].cardlist_.end())
        {
            CardGroup temp;
            temp.cardlist_.push_back(unmatchedList[i].cardlist_[0]);
            temp.cardlist_.push_back((*itSet));
            newList.push_back(std::move(temp));
            unmatchedList[idx].cardlist_.erase(unmatchedList[idx].cardlist_.begin());
            unmatchedList[i].cardlist_.erase(itSet);
            match = true;
            break;
        }
    }
    if (!match)
    {
        tempList[idx].cardlist_.push_back(unmatchedList[idx].cardlist_[0]);
        unmatchedList[idx].cardlist_.erase(unmatchedList[idx].cardlist_.begin());
    }
    return match;
}
