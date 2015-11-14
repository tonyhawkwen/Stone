#include <iostream>
#include <string.h>
#include <cstddef>
#include "../Service/Protocal.h"

using namespace Stone;

int main() {
	unsigned char test[41] = {
		0xEB, 0x90, 0x04, 0x29, 0xC8, 0xCC,
		0x24, 0x07, 0x00, 0x4B, 0x12, 0x00,
		0xDA, 0xBA, 0x69, 0x05, 0x40, 0x04,
		0x00, 0x4B, 0x12, 0x00, 0x02, 0x02,
		0x07, 0x54, 0x65, 0x6D, 0x70, 0x3A,
		0x18, 0x18, 0x03, 0x05, 0x52, 0x68,
		0x3A, 0x18, 0x2C, 0x82, 0x16
	};

	GatewayProto proto((char*)test, 41);
	std::cout << "type:" << proto.GetType() << std::endl;
	std::cout << "device:" << proto.GetDevice() << std::endl;
	char mac[9];
	proto.GetGatewayMac(mac);
	mac[8] = 0;
	std::cout << "mac:" << mac << std::endl;
	char data[40] = {0,};
	unsigned int data_size = proto.GetData(data);
	std::cout << "data size:" << data_size << " data:" << data << std::endl;
	const std::vector<tlv>& values = proto.GetTLV();
	for(auto itr = values.begin(); itr != values.end(); ++itr)
	{
		std::cout << "type:" << itr->type
			<< " length:" << itr->length
			<< " value:" << itr->value << std::endl;
	}
	return 0;
}
