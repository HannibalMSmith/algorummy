#include "gamerummy.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <algorithm>
#include <functional>
#include <iostream>
#include <string>

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
    //std::accumulate(cardlist_.begin(), cardlist_.end(), cardlist_.begin(), [](const PCard &a, const PCard &b){return a->getGoal()+b->getGoal()});
    int goal = 0;
    for (auto it : cardlist_)
    {
        goal += it->rank_;
    }
    return goal;
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
                //套牌
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
                //顺子
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

int GameRummy::match(const map<int, PCard> &hand, vector<CardGroup> &runList, vector<CardGroup> &meldList,
                     vector<CardGroup> &setList, vector<CardGroup> &unmatchedList)
{
    runList.clear();
    meldList.clear();
    setList.clear();
    unmatchedList.clear();
    vector<CardGroup> candidates;
    buildRun(hand, runList, candidates);
    buildMeldAndSet(candidates, runList, meldList, setList);
    buildCandidates(runList, meldList, setList, candidates);
    arrangeUnmatchList(candidates, unmatchedList);
    return 0;
}

//按花色倒序分组
int GameRummy::buildRun(const map<int, PCard> &hand, vector<CardGroup> &runList, vector<CardGroup> &candidates)
{
    vector<CardGroup> suitList(c_suit);
    candidates.resize(c_suit);
    std::for_each(hand.begin(), hand.end(),
                  [&suitList, &candidates, this](pair<const int, PCard> item) {
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

#ifdef DEBUGMORE
    {
        cout << "初选同花组***********************" << endl;
        std::for_each(runList.begin(), runList.end(), std::bind(GameRummy::printCardGroup, std::placeholders::_1, c_idxerror));
        cout << "初选同花组***********************" << endl;
    }
#endif

    return 0;
}

//选取最大同花顺
int GameRummy::buildRunFromGroup(CardGroup &group, vector<CardGroup> &runList, vector<CardGroup> &candidates)
{
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
#ifdef DEBUGMORE
            cout << "last: "
                 << "null"
                 << ":"
                 << "null";
            cout << " it: " << (*it)->suit_ << ":" << (*it)->rank_ << endl;
#endif
            last = *it;
            continue;
        }

#ifdef DEBUGMORE
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

void GameRummy::buildRunOrMeld(vector<CardGroup> &candidates, vector<CardGroup> &runList,
                               vector<CardGroup> &meldList, CardGroup &group, int idxInCandidates)
{
    assert(group.cardlist_.size() == 2 && candidates[0].cardlist_.size() > 0);
    CardGroup newGroup;
    PCard special;
    candidates[idxInCandidates].removeGroup(group);
    if (tryBuildRunWithSpecial(candidates[0], group, newGroup, special))
    {
        runList.push_back(newGroup);
        candidates[0].removeCard(special->id_);
    }
    else
    {
        group.cardlist_.push_back(candidates[0].cardlist_[0]);
        meldList.push_back(group);
        candidates[0].removeCard(candidates[0].cardlist_[0]);
    }
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
    for (auto it = groupCopy.cardlist_.begin(); it <= groupCopy.cardlist_.end()-2; ++it)
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

bool GameRummy::findCardInBothMeldandSet(vector<CardGroup> &candidates, vector<CardGroup> &meldList, int rank, PCard &card, int &idxCandidates)
{
    vector<CardGroup> tempMeldList(c_suit);
    vector<int> meldGoal(c_suit);
    for (auto i = 1; i < c_suit; ++i)
    {
        meldGoal[i] = buildMeldWithRank(candidates, candidates[i], rank);
    }
    auto itMax = std::max_element(meldGoal.begin() + 1, meldGoal.end());
    if (*itMax == 0)
    {
        return false;
    }
    int idxMax = itMax - meldGoal.begin();
    auto itCardInBoth = std::find_if(candidates[idxMax].cardlist_.begin(), candidates[idxMax].cardlist_.end(),
                                     [rank](const PCard &card) { return card->rank_ == rank; });
    card = *itCardInBoth;
    idxCandidates = idxMax;
    return true;
}

//从候选组选出最高得分的顺子或者套牌
int GameRummy::buildMeldAndSet(vector<CardGroup> &candidates, vector<CardGroup> &runList,
                               vector<CardGroup> &meldList, vector<CardGroup> &setList)
{
    std::for_each(candidates.begin(), candidates.end(), [](CardGroup &group) {
        std::sort(group.cardlist_.begin(), group.cardlist_.end(),
                  [](const PCard &a, const PCard &b) { return a->rank_ > b->rank_; });
    });
    std::sort(candidates[0].cardlist_.begin(), candidates[0].cardlist_.end(),
              [](const PCard &a, const PCard &b) { return a->rank_ < b->rank_; });

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
#ifdef DEBUGMORE
        {
            cout << "分组最高分：" << *itMax << " 位置：" << idxGroup << " 顺子最高分：" << *itMaxMeld << " 位置：" << idxMeld << endl;
        }
#endif
        if (*itMax == 0)
        {
            vector<int> topItem(c_suit);
            for (auto i = 1; i < c_suit; i++)
            {
                if (candidates[i].cardlist_.size() == 0)
                {
                    topItem[i] = 0;
                }
                else
                {
                    topItem[i] = candidates[i].cardlist_[0]->rank_;
                }
            }
            auto itTopMax = std::max_element(topItem.begin()+1, topItem.end());
            if ((*itTopMax) == 0)
            {
                break;
            }
            else
            {
                int idxTop = itTopMax - topItem.begin();
                tempList[idxTop].cardlist_.push_back(candidates[idxTop].cardlist_[0]);
                candidates[idxTop].cardlist_.erase(candidates[idxTop].cardlist_.begin());
                continue;
            }
        }

        if (*itMaxMeld == *itMax)
        {
            buildRunOrMeld(candidates, runList, meldList, tempMeldList[idxMeld], idxMeld);
        }
        else
        {
            //fix me with better bookkeep
            markSet(candidates, candidates[idxGroup]);
            int setSize = tempSetList[idxGroup].cardlist_.size();
            if (setSize > c_minimum)
            {
                PCard cardInBoth = NULL;
                int rankSet = tempSetList[idxGroup].cardlist_[0]->rank_;
                int idxCandidates = c_idxerror;
                if (findCardInBothMeldandSet(candidates, meldList, rankSet, cardInBoth, idxCandidates))
                {
                    tempSetList[idxGroup].removeCard(cardInBoth->id_);
                    setList.push_back(tempSetList[idxGroup]);
                    candidates[idxCandidates].idxSetMember_ = c_idxerror;
                    removeSetFromCandidates(candidates);
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
                setList.push_back(tempSetList[idxGroup]);
                candidates[0].removeCard(candidates[0].cardlist_[0]);
                removeSetFromCandidates(candidates);
            }
        }
    }
    std::copy(tempList.begin()+1, tempList.end(), candidates.begin()+1);
    return 0;
}

//选出同花色潜在顺子
int GameRummy::buildMeldFromTop(vector<CardGroup> &candidates, CardGroup &group, CardGroup &meld)
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
        group.removeCard(group.cardlist_[0]);
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
int GameRummy::buildSetFromTop(vector<CardGroup> &candidates, CardGroup &group, CardGroup &set)
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

void GameRummy::markSet(vector<CardGroup> &candidates, CardGroup &group)
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

    auto itMax = std::max_element(goalList.begin()+1, goalList.end());
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
            if(!expandToMeldAndDelSource(setList, candidates[0], 0) &&
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
            if(!expandToMeldAndDelSource(setList, candidates[0], 0) &&
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
