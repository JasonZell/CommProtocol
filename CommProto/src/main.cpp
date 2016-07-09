#include <stdio.h>
#include <CommProto/comms.h>
#include <CommProto/architecture/os/os_threads.h>
#include <CommProto/architecture/os/include_defines.h>
#include <CommProto/hash/NgcpHash.h>
#include <CommProto/console/Console.h>
#include <CommProto/network/CommSocket.h>
#include <CommProto/serialization/Marshal.h>
#include <CommProto/serialization/ObjectStream.h>
#include <stdlib.h>

#include <iostream>
using namespace std;
using namespace Comnet::Serialization;


int main(int c, char** args) {



	ObjectStream newObjectStream = ObjectStream();
	
	double value1A = 64.2;
	int value2A = 12;
	string_t strData = "Hello World!!!";
	uint8_t valueA = 1;
	std::wstring wStrA = L"Hello Wide???";

	std::wcout << wStrA << endl;
	newObjectStream << wStrA;


	//newObjectStream << valueA << strData << value2A << value1A;
	
	double value1B;
	int value2B; 
	uint8_t valueB;
	std::wstring wStrB;


	newObjectStream >> wStrB;
	wcout << wStrB << endl;
	

	//newObjectStream >> value1B >> value2B>>strDataB >> valueB;// >> value2B >> value1B;


	
	//cout << (int)valueB << endl;
	//cout << value1B << endl;
	//cout << value2B << endl;

		
	

	/*
  printf("Hello World!\n");
  Comms newComms1 = Comms(1);
  Comms newComms2 = Comms(2);
  newComms1.initConnection(UDP_LINK, "1337", "127.0.0.1");
  newComms2.initConnection(UDP_LINK, "1338", "127.0.0.1");
  

  newComms1.addAddress(2, "127.0.0.1", 1338);
  newComms2.addAddress(1, "127.0.0.1", 1337);


	uint8_t buf[1024] = { 1 };
	newComms1.send(2, 1, buf, 10);	    

	newComms2.run();
	newComms1.run();

	
	uint8_t sourceID;
	uint16_t messageID;
  uint32_t messageLength;
	uint8_t buffer[10];
	while (true)
	{

    
    Sleep(1000);
  }
  */
  return 0;
}
