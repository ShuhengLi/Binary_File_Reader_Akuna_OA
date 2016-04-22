#pragma once
class Acknowledgement {
public:
	Header h;
	uint32_t order_id;
	uint64_t client_id;
	uint8_t order_status;
	uint8_t reject;
	char termination_string[9];
};