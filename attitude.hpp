#pragma once

#ifndef __ATTITUDE_HPP
#define __ATTITUDE_HPP

#include <string>
#include <map>
#include <vector>
#include "enum_helpers.hpp"

enum class Attitude {
	HOSTILE,
	UNFRIENDLY,
	NEUTRAL,  // This is the default attitude if nothing else is specified.
	FRIENDLY,
	ALLY,
	NUM_ATTITUDES
};

// Intent is to extend this to include attitudes toward units, etc.
enum AttitudeFor {
	FACTION
};

struct AttitudeDescriptor {
	int identifier; // for now this will always be a faction, but eventually it could be a unit or something else
	AttitudeFor attitude_for;

    inline bool operator== (const AttitudeDescriptor& other) const {
        return identifier == other.identifier && attitude_for == other.attitude_for;
    };

    inline bool operator< (const AttitudeDescriptor& other) const {
        return identifier < other.identifier || (identifier == other.identifier && attitude_for < other.attitude_for);
    };
};

class FactionAttitudes {
public:
	FactionAttitudes() {};
	~FactionAttitudes() {};
	
	void set_attitude_toward_faction(int faction_id, Attitude attitude);
	void clear_attitude_toward_faction(int faction_id);
	Attitude get_attitude_toward_faction(int faction_id);
	void set_default_attitude(Attitude attitude);
	void clear_default_attitude();
	Attitude get_default_attitude();
    std::vector<AttitudeDescriptor>& get_attitudes_by_type(Attitude attitude);
    std::map<AttitudeDescriptor, Attitude>& get_attitudes();

private:
	Attitude default_attitude = Attitude::NEUTRAL;
	std::map<AttitudeDescriptor, Attitude> attitudes; // this is used to quickly query the attitude towards something.
	std::map<Attitude, std::vector<AttitudeDescriptor>> attitudes_by_type; // primarily used in reports.
};

const extern std::string AttitudeStrs[];

// Just to allow iteration.
static const Attitude AllAttitudes[to_underlying(Attitude::NUM_ATTITUDES)] = {
    Attitude::HOSTILE, Attitude::UNFRIENDLY, Attitude::NEUTRAL, Attitude::FRIENDLY, Attitude::ALLY
};

#endif
