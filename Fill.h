#pragma once
struct contra {
	uint8_t firm_id;
	char trade_tag[4];
	uint32_t qty;

};

class Fill {
public:
	Header h;
	uint32_t order_id;
	uint64_t price;
	uint32_t qty;
	uint8_t no_of_contras;
	std::vector<contra> contras;
	char termination_string[9];
};