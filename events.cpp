// START A3HEADER
//
// This source file is part of the Atlantis PBM game program.
// Copyright (C) 2020 Valdis Zobēla
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
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

#include "gameio.h"
#include "events.h"
#include "gamedata.h"
#include "graphs.h"
#include "object.h"

#include <map>
#include <queue>
#include <algorithm>

const std::string oneOf(const std::vector<std::string> &list) {
    int i = getrandom(list.size());
    return list.at(i);
}

const std::string oneOf(const std::string &a, const std::string &b) {
    return getrandom(2) ? a : b;
}

std::string townType(const int type) {
    switch (type) {
        case TOWN_VILLAGE: return "village";
        case TOWN_TOWN:    return "town";
        case TOWN_CITY:    return "city";
        default:           return "unknown";
    }
}

FactBase::~FactBase() {

}

void BattleSide::AssignUnit(Unit* unit) {
	this->factionName = unit->faction->name->Str();
	this->factionNum = unit->faction->num;
	this->unitName = unit->name->Str();
	this->unitNum = unit->num;
}

void BattleSide::AssignArmy(Army* army) {
	this->total = army->count;

	for (int i = 0; i < army->count; i++) {
		auto soldier = army->soldiers[i];

        bool lost = soldier->hits == 0;
        if (lost) this->lost++;

        ItemType& item = ItemDefs[soldier->race];

        if (item.flags & ItemType::MANPRODUCE) {
            this->fmi++;
            if (lost) this->fmiLost++;
            continue;
        }
        
        if (item.type & IT_UNDEAD) {
            this->undead++;
            if (lost) this->undeadLost++;
            continue;
        }

        if (item.type & IT_MONSTER && !(item.type & IT_UNDEAD)) {
            this->monsters++;
            if (lost) this->monstersLost++;
            continue;
        }

        auto unit = soldier->unit;
        if (unit == NULL) {
            continue;
        }

        auto type = unit->type;
        if (type == U_MAGE || type == U_GUARDMAGE || type == U_APPRENTICE) {
            this->mages++;
            if (lost) this->magesLost++;
        }
    }
}

const std::string EventLocation::GetTerrainName(const bool plural) {
    TerrainType &terrain = TerrainDefs[this->terrainType];
    auto terrainName = plural ? terrain.plural : terrain.name;
    return terrainName;
}

void populateSettlementLandmark(std::vector<Landmark> &landmarks, ARegion *reg, const int distance) {
    if (!reg->town) {
        return;
    }

    std::string name = reg->town->name->Str();
    std::string title = townType(reg->town->TownType()) + " of " + name;

    landmarks.push_back({
        .type = events::LandmarkType::SETTLEMENT,
        .name = name,
        .title = title,
        .distance = distance,
        .weight = 10
    });
}

void populateRegionLandmark(std::vector<Landmark> &landmarks, ARegion *source, ARegion *reg, const int distance) {
    TerrainType& terrain = TerrainDefs[reg->type];
    int alias = terrain.similar_type;

    std::string name = reg->name->Str();
    std::string sourceName = source->name->Str();
    if (name == sourceName) {
        return;
    }

    std::string title = std::string(terrain.plural) + " of " + name;
    int weight = 1;

    events::LandmarkType type = events::LandmarkType::UNKNOWN;
    if (alias == R_MOUNTAIN) {
        type = events::LandmarkType::MOUNTAIN;
    }
    else if (alias == R_FOREST || alias == R_JUNGLE) {
        type = events::LandmarkType::FOREST;
    }
    else if (alias == R_VOLCANO) {
        type = events::LandmarkType::VOLCANO;
        weight = 2;
    }
    else if (alias == R_OCEAN) {
        type = endsWith(name, "River") ? events::LandmarkType::RIVER : events::LandmarkType::OCEAN;
    }
    else if (endsWith(name, "River")) {
        type = events::LandmarkType::FORD;
        weight = 2;
    }

    if (type == events::LandmarkType::UNKNOWN) {
        return;
    }
    
    landmarks.push_back({
        .type = type,
        .name = name,
        .title = title,
        .distance = distance,
        .weight = weight
    });
}

void populateForitifcationLandmark(std::vector<Landmark> &landmarks, ARegion *reg, const int distance) {
    int protect = 0;
    Object *building = NULL;

    forlist (&reg->objects) {
        auto obj = (Object *) elem;
        ObjectType& type = ObjectDefs[obj->type];

		if (type.flags & ObjectType::GROUP) {
			continue;
		}

		if (obj->IsFleet()) {
			continue;
		}

        if (protect >= type.protect) {
            continue;
        }

        protect = type.protect;
        building = obj;
    }

    if (!building) {
        return;
    }

    std::string name = building->name->Str();
    std::string title = std::string(ObjectDefs[building->type].name) + " " + name;

    landmarks.push_back({
        .type = events::LandmarkType::FORTIFICATION,
        .name = name,
        .title = title,
        .distance = distance,
        .weight = 5
    });
}

bool compareLandmarks(const Landmark &first, const Landmark &second) {
    return first.weight == second.weight
        ? first.distance < second.distance
        : first.weight < second.weight;
}

const EventLocation EventLocation::Create(ARegion* region) {
    EventLocation loc;

    loc.x = region->xloc;
    loc.y = region->yloc;
    loc.z = region->zloc;
    loc.terrainType = region->type;
    loc.province = region->name->Str();
    
    if (region->town) {
        loc.settlement = region->town->name->Str();
        loc.settlementType = region->town->TownType();
    }

    loc.landmarks = { };

    auto items = breadthFirstSearch(region, 4);
    for (auto &kv : items) {
        auto reg = kv.second.key;
        auto distance = kv.second.distance;

        populateSettlementLandmark(loc.landmarks, reg, distance);
        populateRegionLandmark(loc.landmarks, region, reg, distance);
        populateForitifcationLandmark(loc.landmarks, reg, distance);
    }

    std::sort(std::begin(loc.landmarks), std::end(loc.landmarks), compareLandmarks);

    return loc;
}

const Landmark* EventLocation::GetSignificantLandmark() {
    if (this->landmarks.empty()) {
        return NULL;
    }

    return &(this->landmarks.at(0));
}


/////-----


Events::Events() {
}

Events::~Events() {
    for (auto &fact : this->facts) {
        delete fact;
    }

    this->facts.clear();
}

void Events::AddFact(FactBase *fact) {
    this->facts.push_back(fact);
}

bool compareEvents(const Event &first, const Event &second) {
    // return true if first should go before second
    return first.score > second.score;
}

std::list<string> wrapText(std::string input, std::size_t width) {
    std::size_t curpos = 0;
    std::size_t nextpos = 0;

    std::list<std::string> lines;
    std::string substr = input.substr(curpos, width + 1);

    while (substr.length() == width + 1 && (nextpos = substr.rfind(' ')) != input.npos) {
        lines.push_back(input.substr(curpos, nextpos));
        
        curpos += nextpos + 1;
        substr = input.substr(curpos, width + 1);
    }

    if (curpos != input.length()) {
        lines.push_back(input.substr(curpos, input.npos));
    }

    return lines;
}

std::string makeLine(std::size_t width, bool odd, std::string text)  {
    std::string line = (odd ? "( " : " )");

    std::size_t left = (width - text.size()) / 2;

    while (line.size() < left) line += ' ';
    line += text;
    while (line.size() < (width - 2)) line += ' ';

    line += (odd ? " )" : "(");
    line += "\n";

    return line;
}

std::string Events::Write(std::string worldName, std::string month, int year) {
    std::list<Event> events;

    for (auto &fact : this->facts) {
        fact->GetEvents(events);
    }

    std::map<EventCategory, std::vector<Event>> categories;
    for (auto &event : events) {
        if (categories.find(event.category) == categories.end()) {
            std::vector<Event> list = {};
            categories.insert(std::pair<EventCategory, std::vector<Event>>(event.category, list));
        }

        categories[event.category].push_back(event);
    }

    std::string text =  "   _.-=-._.-=-._.-=-._.-=-._.-=-._.-=-._.-=-._.-=-._.-=-._.-=-._.-=-._.-=-._\n";
                text += ".----      - ---     --     ---   -----   - --       ----  ----   -     ----.\n";
                text += " )                                                                         (\n";

    std::list<std::string> lines;
    for (auto &cat : categories) {
        auto list = cat.second;

        std::sort(list.begin(), list.end(), compareEvents);
        list.resize(std::min((int) list.size(), 10));

        int n = std::min((int) list.size(), getrandom(3) + 3);
        while (n-- > 0) {
            int i = getrandom(list.size());

            if (lines.size() > 0) {
                lines.push_back("");
                lines.push_back(".:*~*:._.:*~*:._.:*~*:.");
                lines.push_back("");
            }

            auto tmp = wrapText(list[i].text, 65);
            for (auto &line : tmp) {
                lines.push_back(line);
            }

            list.erase(list.begin() + i);
        }
    }

    bool noNews = lines.size() == 0;

    lines.push_front("");
    lines.push_front(month + ", Year " + std::to_string(year));
    lines.push_front(worldName + " Events");

    if (noNews) {
        lines.push_back("--== Nothing mentionable happened in the world this month ==--");
    }

    if (lines.size() % 2) {
        lines.push_back("");
    }

    int n = 3;
    for (auto &line : lines) {
        text += makeLine(77, (n++) % 2, line);
    }

    text += "(__       _       _       _       _       _       _       _       _       __)\n";
    text += "    '-._.-' (___ _) '-._.-' '-._.-' )     ( '-._.-' '-._.-' (__ _ ) '-._.-'\n";
    text += "            ( _ __)                (_     _)                (_ ___)\n";
    text += "            (__  _)                 '-._.-'                 (___ _)\n";
    text += "            '-._.-'                                         '-._.-'\n";

    return text;
}
