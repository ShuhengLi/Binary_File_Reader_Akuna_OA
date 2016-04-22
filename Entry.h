#pragma once
class Entry {
public:
	Header h;
	uint64_t price;
	uint32_t qty;
	char instrument[11];
	uint8_t side;
	uint64_t client_assigned_id;
	uint8_t time_in_force;
	char trader_tag[4];
	uint8_t firm_id;
	std::string firm;
	char termination_string[9];
};