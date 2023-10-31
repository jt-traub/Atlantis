#include "attitude.hpp"

#include <string>
#include <algorithm>

const std::string AttitudeStrs[] = {
	"Hostile",
	"Unfriendly",
	"Neutral",
	"Friendly",
	"Ally"
};

void FactionAttitudes::clear_attitude_toward_faction(int faction_id) {
    AttitudeDescriptor descriptor { faction_id, AttitudeFor::FACTION };

    if (attitudes.contains(descriptor)) {
        Attitude attitude = attitudes[descriptor];
        attitudes.erase(descriptor);
        attitudes_by_type[attitude].erase(
            std::remove(attitudes_by_type[attitude].begin(), attitudes_by_type[attitude].end(), descriptor),
            attitudes_by_type[attitude].end()
        );
    }
}

void FactionAttitudes::set_attitude_toward_faction(int faction_id, Attitude attitude) {
    AttitudeDescriptor descriptor { faction_id, AttitudeFor::FACTION };

    if (attitudes.contains(descriptor)) {
        Attitude old_attitude = attitudes[descriptor];
        attitudes[descriptor] = attitude;
        attitudes_by_type[old_attitude].erase(
            std::remove(attitudes_by_type[old_attitude].begin(), attitudes_by_type[old_attitude].end(), descriptor),
            attitudes_by_type[old_attitude].end()
        );
        attitudes_by_type[attitude].push_back(descriptor);
    } else {
        attitudes[descriptor] = attitude;
        attitudes_by_type[attitude].push_back(descriptor);
    }
}

Attitude FactionAttitudes::get_attitude_toward_faction(int faction_id) {
    AttitudeDescriptor descriptor { faction_id, AttitudeFor::FACTION };

    if (attitudes.contains(descriptor)) {
        return attitudes[descriptor];
    } else {
        return default_attitude;
    }
}

void FactionAttitudes::set_default_attitude(Attitude attitude) {
    default_attitude = attitude;
}

void FactionAttitudes::clear_default_attitude() {
    default_attitude = Attitude::NEUTRAL;
}

Attitude FactionAttitudes::get_default_attitude() {
    return default_attitude;
}

std::vector<AttitudeDescriptor>& FactionAttitudes::get_attitudes_by_type(Attitude attitude) {
    return attitudes_by_type[attitude];
}

std::map<AttitudeDescriptor, Attitude>& FactionAttitudes::get_attitudes() {
    return attitudes;
}
