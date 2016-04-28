#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<cstdint>
#include<map>
#include <algorithm>
#include"Header.h"
#include"Entry.h"
#include"Acknowledgement.h"
#include"Fill.h"

//using namespace std;
//Read files function declearation ==================
void read_header(std::ifstream& input);
void read_entry(std::ifstream &input,Header h,uint16_t);
void read_acknowledgement(std::ifstream &input,Header h);
void read_fill(std::ifstream &input,Header h);
void read_contra(std::ifstream & input, uint16_t no_of_contras, Fill &F);

//comput TraderTags
std::string ActTraderTag();
std::string LiqTraderTag();
void Volum_instr();
bool myfun(Entry, Entry);

//Dispaly files function declearation(debug)====================
void display_header();
void display_entry();
void display_acknowledgement();
void display_fills();
void display_contra(std::vector<contra>);

//vectors for saving input dates=========================
std::vector<Header> headers;
std::vector<Entry> entrys;
std::vector<Acknowledgement> acknowledgements;
std::vector<Fill> fills;
std::map<std::string, uint32_t> Trader_fills;

int main(int argc, char *argv[]) {

	if (argc < 2) {
		std::cout<< "No input files in command, please type the correct command with input file directory" << std::endl;
		return 0;
	}

	std::ifstream input(argv[1], std::ios::binary);

	while (!input.eof()) {
		read_header(input);
	}
	headers.pop_back();

	int total_packets = headers.size();
	int order_entry_msg_count = entrys.size();
	int order_ack_msg_count = acknowledgements.size();
	int order_fill_msg_count = fills.size();
	std::string most_active_trader_tag = ActTraderTag();
	std::string most_liquidity_trader_tag = LiqTraderTag();

	std::cout<< total_packets << ", " << order_entry_msg_count << ", " << order_ack_msg_count << ", " << order_fill_msg_count << ", "<< most_active_trader_tag<<", "<< most_liquidity_trader_tag ;

	Volum_instr();
	getchar();
	return 0;
}

//Parsing Part==================================================================
void read_header(std::ifstream &input)
{
	Header H;

	if (!input.is_open()) {// Always check to see if file opening succeeded?Ан?0
		std::cout<< "Could not open file\n";
		exit(0);
	}
	else {
		input.read((char*)&H.marker, sizeof(H.marker));
		input.read((char*)&H.msg_type, sizeof(H.msg_type));
		input.read((char*)&H.sequence_id, sizeof(H.sequence_id));
		input.read((char*)&H.timestamp, sizeof(H.timestamp));
		input.read((char*)&H.msg_direction, sizeof(H.msg_direction));
		input.read((char*)&H.msg_len, sizeof(H.msg_len));
	}

	headers.push_back(H); // save it in std::vector headers
	switch (H.msg_type)
	{
		//3rd parament is Entry.firm length =  the total msg length - all other informations length(8+4+10+1+8+1+3+1+8) = 44
		case 1:  read_entry(input,H,(H.msg_len - 44));
			break;
		case 2: read_acknowledgement(input, H);
			break;
		case 3: read_fill(input, H);
			break;

	}
	return;
}

void read_entry(std::ifstream & input,Header H,uint16_t firm_len)
{
	if (firm_len > 256 || firm_len < 0) {
		std::cout<< "Firm length is invalid, please check input data" << std::endl;
		exit(0);
	}
	Entry E;
	if (!input.is_open())// Always check to see if file opening succeeded
		std::cout<< "Could not open file\n";
	else {
		input.read((char*)&E.price, sizeof(E.price));
		input.read((char*)&E.qty, sizeof(E.qty));
		input.read((char*)&E.instrument, 10);
		E.instrument[10] = '\0';//avoid garbled at the end of the std::string
		input.read((char*)&E.side, sizeof(E.side));
		input.read((char*)&E.client_assigned_id, sizeof(E.client_assigned_id));
		input.read((char*)&E.time_in_force, sizeof(E.time_in_force));
		input.read((char*)&E.trader_tag, 3);
		E.trader_tag[3] = '\n';//avoid garbled at the end of the std::string
		input.read((char*)&E.firm_id, sizeof(E.firm_id));
		
		//save firm_len numbers chars to firm
		for (uint16_t i = 0; i < firm_len; i++) {
			char c;
			input.read((char*)&c,1);
			E.firm.push_back(c);
		}
		input.read((char*)&E.termination_string, 8);
		E.termination_string[8] = '\n';
	}
	E.h = H;
	entrys.push_back(E);
	}

void read_acknowledgement(std::ifstream & input,Header H)
{
	Acknowledgement A;
	if (!input.is_open())// Always check to see if file opening succeeded?Ан?0
		std::cout<< "Could not open file\n";
	else {
		input.read((char*)&A.order_id, 4);
		input.read((char*)&A.client_id, 8);
		input.read((char*)&A.order_status, 1);
		input.read((char*)&A.reject, 1);
		input.read((char*)&A.termination_string, 8);
		A.termination_string[8] = '\n';
	}
	A.h = H;
	acknowledgements.push_back(A);
}

void read_fill(std::ifstream & input,Header H)
{
	Fill F;
	if (!input.is_open())// Always check to see if file opening succeeded?Ан?0
		std::cout<< "Could not open file\n";
	else {
		input.read((char*)&F.order_id, 4);
		input.read((char*)&F.price, 8);
		input.read((char*)&F.qty, 4);

		// compute original filled volume;
		std::string temp(entrys.back().trader_tag);
		temp = temp.substr(0,3);
		Trader_fills[temp] += F.qty;
		input.read((char*)&F.no_of_contras, 1);
		read_contra(input, F.no_of_contras, F);
		input.read((char*)&F.termination_string, 8);
		F.termination_string[8] = '\n';
	}
	F.h = H;
	fills.push_back(F);
}

void read_contra(std::ifstream & input,uint16_t no_of_contras, Fill &F) {
	for (uint16_t i = 0; i < no_of_contras; i++) {
		contra c;
		input.read((char*)&c.firm_id, 1);
		input.read((char*)&c.trade_tag, 3);
		c.trade_tag[3] = '\n';
		input.read((char*)&c.qty, 4);

		//compute contras filled volume;
		std::string temp(entrys.back().trader_tag);
		temp = temp.substr(0, 3);
		Trader_fills[temp] += c.qty;
		F.contras.push_back(c);
	}
	return;
}


//==============================Compute Part========================================================================
std::string ActTraderTag() {
	//find the max VALUE from map Trader_fills and return the KEY 
	using pair_type = decltype(Trader_fills)::value_type;
	auto max = max_element(
		Trader_fills.begin(), Trader_fills.end(),[](const pair_type & p1, const pair_type & p2) {return p1.second < p2.second;}
	);
	return max->first;
}

std::string LiqTraderTag()
{
	// Initialazation the map Trader_Liquid
	std::map<std::string, uint32_t> Trader_Liquid;
	for (size_t i = 0; i < entrys.size(); i++) {
		// if is good count
		if (entrys[i].side == 2) {
			std::string temp(entrys[i].trader_tag);
			temp = temp.substr(0, 3);
			Trader_Liquid[temp] += entrys[i].qty;
		}
		else {}
	}
	//find the max VALUE from map Trader_Liquid and return the KEY 
	using pair_type = decltype(Trader_Liquid)::value_type;
	auto max = max_element(
		Trader_Liquid.begin(), Trader_Liquid.end(), [](const pair_type & p1, const pair_type & p2) {return p1.second < p2.second; }
	);
	return max->first;
}

void Volum_instr()
{	
	std::map<std::string, uint32_t> Instruc_qty;
	for (size_t i = 0; i < entrys.size(); i++) {
		Instruc_qty[entrys[i].instrument] += entrys[i].qty;
	}

	for (auto it = Instruc_qty.begin(); it != Instruc_qty.end(); it++) {
		std::cout<< ", " << it->first << ": " << it->second/2;
	}
}

bool myfun(Entry e1, Entry e2)
{
	return e1.instrument > e2.instrument;
}


//================================================================debug====================================================
void display_header()
{
	
	for (size_t i = 0; i < headers.size(); i++) {
		std::cout<< std::endl << "=========================Header [ " << i << " ] =====================" << std::endl;
		std::cout<< "marker : " << headers[i].marker << std::endl;
		std::cout<< "mag_type : " << int(headers[i].msg_type) << std::endl;
		std::cout<< " sequenmce_id :   " << headers[i].sequence_id << std::endl;
		std::cout<< "timestamp :  "  << headers[i].timestamp << std::endl;
		std::cout<< "msg_direction :" << int(headers[i].msg_direction) << std::endl;
		std::cout<< "msg_len :" << int(headers[i].msg_len) << std::endl<<std::endl;

	}	std::cout<< std::endl << "=====================header end=============" << std::endl;
	return;
}

void display_entry()
{	
	
	for (size_t i = 0; i <entrys.size(); i++) {
		std::cout<< std::endl << "=========================Entry [ " << i << " ] =====================" << std::endl;
		std::cout<< " Entry price : " <<entrys[i].price << std::endl;
		std::cout<< "Entry qty : " << entrys[i].qty << std::endl;
		std::cout<< " Entry Instrument : " <<entrys[i].instrument << std::endl;
		std::cout<< "Entry slide :  " <<entrys[i].side << std::endl;
		std::cout<< "Entry client id :  "  << entrys[i].client_assigned_id << std::endl;
		std::cout<< "Entry  time in :  " << int(entrys[i].time_in_force) << std::endl;
		std::cout<< "Entry  trader tag:  "  << entrys[i].trader_tag << std::endl;
		std::cout<< "Entry  firm id:  " << entrys[i].firm_id << std::endl;
		std::cout<< "Entry  firm: " << entrys[i].firm << std::endl;
		std::cout<< "Entry  termination:  " << entrys[i].termination_string << std::endl;
	}
	return;
}

void display_acknowledgement() {
	
	for (size_t i = 0; i < acknowledgements.size(); i++) {
		std::cout<< std::endl << "=========================acknowledgement [ " << i << " ] =====================" << std::endl;
		std::cout<< "Acknowledgement order_id:  " << acknowledgements[i].order_id << std::endl;
		std::cout<< "Acknowledgement client id:  " << acknowledgements[i].client_id << std::endl;
		std::cout<< "Acknowledgement order status:  " <<(int) acknowledgements[i].order_status << std::endl;
		std::cout<< "Acknowledgement reject:  " << int(acknowledgements[i].reject) << std::endl;
		std::cout<< "Acknowledgement termination std::string:  " << acknowledgements[i].termination_string << std::endl<<std::endl;
	}
}

void display_fills() {

	for (size_t i = 0; i < fills.size(); i++) {
		std::cout<< std::endl << "=========================fill [ "<<i<<" ] =====================" << std::endl;
		std::cout<< "Fill order_id:  " << fills[i].order_id << std::endl;
		std::cout<< "Fill price:  " << fills[i].price << std::endl;
		std::cout<< "Fill qty:  " << (int)fills[i].qty<< std::endl;
		std::cout<< "Fill Number of contras:  " << int(fills[i].no_of_contras) << std::endl;
		display_contra(fills[i].contras);
		std::cout<< "Fill termination std::string:  " << fills[i].termination_string << std::endl<<std::endl;
	}
}

void display_contra(std::vector<contra> t) {
	for (size_t i = 0; i < t.size(); i++) {
		std::cout<< std::endl << "=========================contras [ " << i << " ] =====================" << std::endl;
		std::cout<< "contra firm_id:  " << t[i].firm_id << std::endl;
		std::cout<< "contra trade tag:  " << t[i].trade_tag << std::endl;
		std::cout<< "contra order qty:  " << (int)t[i].qty << std::endl;
	}
}
