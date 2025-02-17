// START A3HEADER
//
// This source file is part of the Atlantis PBM game program.
// Copyright (C) 1995-1999 Geoff Dunbar
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program, in the file license.txt. If not, write
// to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
// See the Atlantis Project web page for details:
// http://www.prankster.com/project
//
// END A3HEADER

#include "quests.h"
#include "object.h"
#include <iterator>

QuestList quests;

Quest::Quest()
{
	type = -1;
	target = -1;
	objective.type = -1;
	objective.num = 0;
	building = -1;
	regionnum = -1;
	regionname = "-";
}

Quest::~Quest()
{
	Item *i;

	forlist(&rewards) {
		i = (Item *) elem;
		rewards.Remove(i);
		delete i;
	}
}

AString Quest::GetRewardsStr()
{
	Item *i;
	AString quest_rewards;
	int first_i = 1;

	quest_rewards = "Quest rewards: ";

	if (rewards.Num() == 0) {
		return quest_rewards + "none.";
	}

	forlist (&rewards) {
		i = (Item *) elem;
		if (first_i) {
			first_i = 0;
		} else {
			quest_rewards += ", ";
		}
		quest_rewards += ItemString(i->type, i->num);
	}
	quest_rewards += ".";

	return quest_rewards;
}

int QuestList::ReadQuests(istream& f)
{
	int count, dests, rewards;
	Quest *quest;
	AString name;
	Item *item;

	quests.DeleteAll();

	f >> count;
	if (count < 0)
		return 0;
	while (count-- > 0) {
		quest = new Quest();
		f >> quest->type;
		switch (quest->type) {
			case Quest::SLAY:
				f >> quest->target;
				break;
			case Quest::HARVEST:
				quest->objective.Readin(f);
				f >> quest->regionnum;
				break;
			case Quest::BUILD:
				f >> ws >> name;
				quest->building = LookupObject(&name);
				f >> ws >> quest->regionname;
				break;
			case Quest::VISIT:
				f >> ws >> name;
				quest->building = LookupObject(&name);
				f >> dests;
				while (dests-- > 0) {
					f >> ws  >> name;
					quest->destinations.insert(name.Str());
				}
				break;
			case Quest::DEMOLISH:
				f >> quest->target;
				f >> quest->regionnum;
				break;
			default:
				f >> quest->target;
				quest->objective.Readin(f);
				f >> ws >> name;
				quest->building = LookupObject(&name);
				f >> quest->regionnum;
				f >> ws >> quest->regionname;
				f >> dests;
				while (dests-- > 0) {
					f >> ws >> name;
					quest->destinations.insert(name.Str());
				}
				break;
		}
		f >> rewards;
		while (rewards-- > 0) {
			item = new Item();
			item->Readin(f);
			if (-1 == item->type)
				return 0;
			quest->rewards.Add(item);
		}
		quests.Add(quest);
	}

    return 1;
}

void QuestList::WriteQuests(ostream& f)
{
	Quest *q;
	Item *i;
	set<string>::iterator it;

	f << quests.Num() << '\n';
	forlist(this) {
		q = (Quest *) elem;
		f << q->type << '\n';
		switch(q->type) {
			case Quest::SLAY:
				f << q->target << '\n';
				break;
			case Quest::HARVEST:
				q->objective.Writeout(f);
				f << q->regionnum << '\n';
				break;
			case Quest::BUILD:
				f << (q->building == -1 ? "NO_OBJECT" : ObjectDefs[q->building].name) << '\n';
				f << q->regionname << '\n';
				break;
			case Quest::VISIT:
				f << (q->building == -1 ? "NO_OBJECT" : ObjectDefs[q->building].name) << '\n';
				f << q->destinations.size() << '\n';
				for (it = q->destinations.begin(); it != q->destinations.end();	it++) {
					f << *it << '\n';
				}
				break;
			case Quest::DEMOLISH:
				f << q->target << '\n';
				f << q->regionnum << '\n';
				break;
			default:
				f << q->target << '\n';
				q->objective.Writeout(f);
				f << (q->building == -1 ? "NO_OBJECT" : ObjectDefs[q->building].name) << '\n';
				f << q->regionnum << '\n';
				f << q->regionname << '\n';
				f << q->destinations.size() << '\n';
				for (it = q->destinations.begin(); it != q->destinations.end();	it++) {
					f << *it << '\n';
				}
				break;
		}
		f << q->rewards.Num() << '\n';
		forlist(&q->rewards) {
			i = (Item *) elem;
			i->Writeout(f);
		}
	}
	f << 0 << '\n';
}

int QuestList::CheckQuestKillTarget(Unit *u, ItemList *reward, AString *quest_rewards)
{
	Quest *q;
	Item *i;

	forlist(this) {
		q = (Quest *) elem;
		if (q->type == Quest::SLAY && q->target == u->num) {
			// This dead thing was the target of a quest!
			forlist (&q->rewards) {
				i = (Item *) elem;
				reward->SetNum(i->type, reward->GetNum(i->type) + i->num);
			}
			*quest_rewards = q->GetRewardsStr();
			this->Remove(q);
			delete q;
			return 1;
		}
	}

	return 0;
}

int QuestList::CheckQuestHarvestTarget(ARegion *r,
		int item, int harvested, int max,
		Unit *u, AString *quest_rewards)
{
	Quest *q;
	Item *i;

	forlist(this) {
		q = (Quest *) elem;
		if (q->type == Quest::HARVEST &&
				q->regionnum == r->num &&
				q->objective.type == item) {
			
			if (getrandom(max) < harvested) {
				forlist (&q->rewards) {
					i = (Item *) elem;
					u->items.SetNum(i->type, u->items.GetNum(i->type) + i->num);
					u->faction->DiscoverItem(i->type, 0, 1);
				}
				*quest_rewards = q->GetRewardsStr();
				this->Remove(q);
				delete q;
				return 1;
			}
		}
	}

	return 0;
}

int QuestList::CheckQuestBuildTarget(ARegion *r, int building,
		Unit *u, AString *quest_rewards)
{
	Quest *q;
	Item *i;

	forlist(this) {
		q = (Quest *) elem;
		if (q->type == Quest::BUILD &&
				q->building == building &&
				q->regionname == *r->name) {

			forlist (&q->rewards) {
				i = (Item *) elem;
				u->items.SetNum(i->type, u->items.GetNum(i->type) + i->num);
				u->faction->DiscoverItem(i->type, 0, 1);
			}
			*quest_rewards = q->GetRewardsStr();
			this->Remove(q);
			delete q;
			return 1;
		}
	}

	return 0;
}

int QuestList::CheckQuestVisitTarget(ARegion *r, Unit *u, AString *quest_rewards)
{
	Quest *q;
	Object *o;
	Item *i;
	set<string> intersection;
	set<string>::iterator it;

	forlist(this) {
		q = (Quest *) elem;
		if (q->type != Quest::VISIT)
			continue;
		if (!q->destinations.count(r->name->Str()))
			continue;
		forlist(&r->objects) {
			o = (Object *) elem;
			if (o->type == q->building) {
				u->visited.insert(r->name->Str());
				intersection.clear();
				set_intersection(
					q->destinations.begin(),
					q->destinations.end(),
					u->visited.begin(),
					u->visited.end(),
					inserter(intersection,
						intersection.begin()),
					less<string>()
				);
				if (intersection.size() == q->destinations.size()) {
					// This unit has visited the
					// required buildings in all those
					// regions, so they completed a quest
					forlist (&q->rewards) {
						i = (Item *) elem;
						u->items.SetNum(i->type, u->items.GetNum(i->type) + i->num);
						u->faction->DiscoverItem(i->type, 0, 1);
					}
					*quest_rewards = q->GetRewardsStr();
					this->Remove(q);
					delete q;
					return 1;
				}
			}
		}
	}

	return 0;
}

int QuestList::CheckQuestDemolishTarget(ARegion *r, int building,
		Unit *u, AString *quest_rewards)
{
	Quest *q;
	Item *i;

	forlist(this) {
		q = (Quest *) elem;
		if (q->type == Quest::DEMOLISH &&
				q->regionnum == r->num &&
				q->target == building) {
			forlist (&q->rewards) {
				i = (Item *) elem;
				u->items.SetNum(i->type, u->items.GetNum(i->type) + i->num);
				u->faction->DiscoverItem(i->type, 0, 1);
			}
			*quest_rewards = q->GetRewardsStr();
			this->Remove(q);
			delete q;
			return 1;
		}
	}

	return 0;
}

