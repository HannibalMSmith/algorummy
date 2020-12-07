#include "gamerummy.h"
#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <stdio.h>
#include <string.h>
using std::cout;
using std::endl;
using std::for_each;
using std::map;
using std::shared_ptr;
using std::string;
using std::vector;

int Card::idx = 0;
int CardGroup::idx = 0;

Card::Card() : suit_(e_joker), rank_(0), id_(genId()), joker_(suit_ == e_joker)
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

void CardGroup::removeCard(int id)
{
    cardlist_.erase(std::find_if(cardlist_.begin(), cardlist_.end(), [id](const PCard &target) { return target->id_ == id; }));
}

void CardGroup::removeGroup(const CardGroup &rhs)
{
    for_each(rhs.cardlist_.begin(), rhs.cardlist_.end(),
             [this](const PCard &card) {
                 removeCard(card);
             });
}

void CardGroup::reset()
{
    id_ = genId();
    cardlist_.clear();
}

bool CardGroup::restarMatch(CardGroup &matched, std::vector<PCard>::iterator &start)
{
    start = cardlist_.begin();
    if (start != cardlist_.end())
    {
        matched.cardlist_.push_back(*start);
        return true;
    }
    else
    {
        return false;
    }
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
             [&suitlist,this](std::pair<const int, PCard> item) {
                 if (!(item.second->rank_ == special_))
                 {
                     suitlist[item.second->suit_ - 1].cardlist_.push_back(item.second);
                 }
             });

    for_each(suitlist.begin(), suitlist.end(),
             [&matchlist, &candidates](CardGroup &group) {
                 std::sort(group.cardlist_.begin(), group.cardlist_.end(), [](const PCard &a, const PCard &b) { return a->rank_ > b->rank_; });
                 pickRunFromGroup(group, matchlist, candidates);
             });

    cout << "同花组" << endl;
    for_each(matchlist.begin(), matchlist.end(), Mgr::printCardGroup);
    return 0;
}

int Mgr::pickRunFromGroup(const CardGroup &group, vector<CardGroup> &matchlist, vector<CardGroup> &candidates)
{
    candidates.clear();
    CardGroup groupCopy(group);
    if (groupCopy.cardlist_.size() < c_minimum)
    {
        candidates.push_back(group);
        return 0;
    }

    printCardGroup(groupCopy);
    cout << "pickRunFromGroupEnd" << endl;

    CardGroup unmatched;
    CardGroup tmpMatched;
    auto last = groupCopy.cardlist_.begin();
    while (last != groupCopy.cardlist_.end())
    {
        if ((*last)->magic_)
        {
            ++last;
        }
        else
        {
            tmpMatched.cardlist_.push_back(*last);
            break;
        }
    };

    for (auto it = last; it != groupCopy.cardlist_.end(); ++it)
    {
        cout << "last " << (*last)->suit_ << ":" << (*last)->rank_ << " it " << (*it)->suit_ << ":" << (*it)->rank_ << std::boolalpha << " " << (*it)->magic_ << endl;
        if ((*it)->magic_)
        {
            last = it;
            unmatched.cardlist_.push_back(*it);
            continue;
        }
        else if (!((*it)->magic_) && (*it)->rank_ == (*last)->rank_)
        {
            unmatched.cardlist_.push_back(*it);
            continue;
        }
        else if (!((*it)->magic_) && ((*last)->rank_ - (*it)->rank_ == 1))
        {
            tmpMatched.cardlist_.push_back(*it);
            last = it;
        }
        else
        {
            unmatched.cardlist_.insert(unmatched.cardlist_.end(), tmpMatched.cardlist_.begin(), tmpMatched.cardlist_.end());
            tmpMatched.reset();
            last = it;
            continue;
        }
        if (tmpMatched.cardlist_.size() == c_minimum - 1)
        {
            if ((*(tmpMatched.cardlist_.begin()))->rank_ == 13 && (*(tmpMatched.cardlist_.begin() + 1))->rank_ == 12)
            {
                auto ace = std::find_if(groupCopy.cardlist_.begin(), groupCopy.cardlist_.end(), [](const PCard &card) { return card->rank_ == 1; });
                if (ace != groupCopy.cardlist_.end())
                {
                    tmpMatched.cardlist_.insert(tmpMatched.cardlist_.begin(), (*ace));
                    matchlist.push_back(tmpMatched);

                    cout << "******************add ace run:" << endl;
                    cout << "******************groupCopy size:" << groupCopy.cardlist_.size() << endl;
                    printCardGroup(tmpMatched);

                    groupCopy.removeGroup(tmpMatched);
                    tmpMatched.reset();
                    cout << "******************add ace end:" << endl;
                    cout << "******************groupCopy size:" << groupCopy.cardlist_.size() << endl;
                    it = groupCopy.cardlist_.begin();
                    cout << "******************groupCopy begin: " << (*it)->suit_ << ":" << (*it)->rank_ << endl;

                    it = groupCopy.cardlist_.begin();
                    if (it != groupCopy.cardlist_.end())
                    {

                        tmpMatched.cardlist_.push_back(*(it));
                        last = it;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
        else if (tmpMatched.cardlist_.size() == c_minimum)
        {

            cout << "******************add run！" << endl;
            cout << "******************groupCopy size:" << groupCopy.cardlist_.size() << endl;
            printCardGroup(tmpMatched);

            matchlist.push_back(tmpMatched);
            groupCopy.removeGroup(tmpMatched);
            tmpMatched.reset();

            if (groupCopy.restarMatch(tmpMatched, it))
            {
                last = it;

                cout << "******************add ace end:" << endl;
                cout << "******************groupCopy size:" << groupCopy.cardlist_.size() << endl;
            }
            else
            {
                break;
            }
        }
    }
    candidates.push_back(unmatched);

    return 0;
}

void Mgr::printCardGroup(const CardGroup &group)
{
    cout << "分组编号: " << group.id_ << " 卡牌列表" << endl;
    for_each(group.cardlist_.begin(), group.cardlist_.end(),
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
    case e_club:
        name = "梅花";
        break;
    case e_diamond:
        name = "方块";
        break;
    case e_heart:
        name = "红桃";
        break;
    case e_spade:
        name = "黑桃";
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
    default:
        name = "错误";
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

int Mgr::pickHighestMeld(std::vector<CardGroup> &matchlist, std::vector<CardGroup> &candidates)
{

    return 0;
}