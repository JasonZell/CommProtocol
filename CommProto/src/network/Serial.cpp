/* 
  Implementation of x-platform serial connections.

  Copyright (C) 2016  Michael Wallace, Mario Garcia.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <CommProto/network/Serial.h>
;


namespace Comnet {
namespace Network {


/***********************************************/
/******************* Private *******************/
/***********************************************/
#if COM_TARGET_OS == COM_OS_WINDOWS


// Defines the Serial keyword for the preprocessor, allows us to focus on only one 
// set of serial functions in an operating system.
#define WINDOWS_SERIAL

/**
  Initializes windows serial port.
*/
bool initWindows(Serial& serial, const char* comPort, uint32_t baudrate)
{
  serial_t& hSerial = serial.getSerialPort();

  char comPortCat[80];
  strcpy(comPortCat, (char*)comPort);

  if (comPort[0] != '\\') {
      // "\\\\.\\"  allows windows ports above 9
	  char temp[80];
	  strcpy(temp, comPortCat);
	  strcpy(comPortCat, "\\\\.\\");
	  strcat(comPortCat, temp);
  } 


#ifdef UNICODE	//convert multybyte string to LPCWSTR
  uint16_t length = 0;
  str_length(comPortCat, length);
  wchar_t* str = new wchar_t[length * 2];
  MultiByteToWideChar(CP_ACP, 0, comPortCat, -1, str, length * 2);
  LPCWSTR comport = str;
#else
  //open up serial port  
  const char* comport = comPortCat;
#endif
  hSerial.h_serial = CreateFile(comport,
				GENERIC_READ | GENERIC_WRITE,
				0,
				0,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				0);

#ifdef UNICODE //clean up multybyte string to LPCWSTR
  delete str;
#endif
  
  if (hSerial.h_serial == INVALID_HANDLE_VALUE)
    {
      if (GetLastError() == ERROR_FILE_NOT_FOUND)
	{
	  //serial port does not exist.
	  printf( "comport not found\n");
	  return false;
	}
      // some other error occured.
      printf( "Unknown error\n");
      return false;
    }
  else
    {
      
      printf( "Serial port created\n");
    }
  
  
  //setup parameters
  
  DCB dcbSerialParams = { 0 };
  
  dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
  
  if (!GetCommState(hSerial.h_serial, &dcbSerialParams))
    {
      //error getting state
      printf( "Error getting state\n");
      return false;
    }
  
  dcbSerialParams.BaudRate = (DWORD)baudrate;
  dcbSerialParams.ByteSize = 8;
  dcbSerialParams.StopBits = ONESTOPBIT;
  dcbSerialParams.Parity = NOPARITY;
  
  if (!SetCommState(hSerial.h_serial, &dcbSerialParams))
    {
      //error setting serial port state
      printf( "Error setting serial port state\n");
      return false;
    }
  
  
  printf( "Completed setting serial port state\n");
  

  //time out code not needed?
  
  COMMTIMEOUTS timeouts = { 0 };
  
  timeouts.ReadIntervalTimeout = 5;
  timeouts.ReadTotalTimeoutConstant = 5;
  timeouts.ReadTotalTimeoutMultiplier = 1;
  timeouts.WriteTotalTimeoutConstant = 50;
  timeouts.WriteTotalTimeoutMultiplier = 10;
  
  if (!SetCommTimeouts(hSerial.h_serial, &timeouts))
    {
      //error occureeed
      printf( "Setting up times outs failed\n");
      return false;
    }
  
  printf( "Completed setting up time outs\n");
  hSerial.serial_s = SERIAL_CONNECTED;
  return true;
}


inline bool
windowsSend(Serial& serial, uint8_t destID, uint8_t* txData, int32_t txLength) {
  unsigned long sentData = 0;//windows want LPDWORD == unsignled long != uint32_t || uint16_t
  serial_t& hSerial = serial.getSerialPort();
  
  if (!WriteFile(hSerial.h_serial, txData, txLength, &sentData, NULL)) {
    //error reading file
    printf("Failed to write serial\n");
    return false;
  }
  else {
#ifdef SERIAL_DEBUG
    printf("**  Sent\t Length: %d, Sent: %d, destID: %d **\n", txLength, sentData, destID);
#endif	
    return true;
  }		

  return false;
}


inline bool
windowsRead(Serial& serial, uint8_t* rx_data, uint32_t* rx_len) {
  serial_t& hSerial = serial.getSerialPort();
  unsigned long recvData = 0;//windows wants LPDWORD == unsignled long; LPWORD != uint32_t || uint16_t
  if (!ReadFile(hSerial.h_serial, rx_data, MAX_BUFFER_SIZE, &recvData, NULL)) {
    //error reading file
    printf("Failed to read serial\n"); // This may print too much data might need to change. 
                                       // Im not sure if time out will return false for ReadFile
    return false;
  } else {
#ifdef SERIAL_DEBUG
    printf("**  Recieved\t Length: %d  **\n", recvData);
#endif	
    if (recvData > 0) {
      *rx_len = recvData;
      return true;
    }
  }
  
  return false;//no error, no data recieved, connection timmed out, will retry next time.
}

#else 

#define UNIX_SERIAL 


inline bool
initUnixSerial(Serial& serial, const char* port, uint32_t baudrate) {
  bool result = false;
  serial_t& hSerial = serial.getSerialPort();

  printf("port: %s\n connecting...\n", port);
  hSerial.fd = open(port, (O_RDWR | O_NOCTTY));

  if (hSerial.fd == -1) {
    printf("port failed to open: err number %d\n", errno);
  } else {
    struct termios options;
    fcntl(hSerial.fd, F_SETFL, 0);
    tcgetattr(hSerial.fd, &options);
    memset(&options, 0, sizeof(options));
    speed_t speed = B115200;
    switch (baudrate) {
      case 0:      { speed = B0;      break; }
      case 50:     { speed = B50;     break; }
      case 75:     { speed = B75;     break; } 
      case 110:    { speed = B110;    break; }
      case 134:    { speed = B134;    break; }
      case 150:    { speed = B150;    break; }
      case 200:    { speed = B200;    break; }
      case 300:    { speed = B300;    break; }
      case 600:    { speed = B600;    break; }
      case 1200:   { speed = B1200;   break; }
      case 1800:   { speed = B1800;   break; }
      case 2400:   { speed = B2400;   break; }
      case 4800:   { speed = B4800;   break; }
      case 9600:   { speed = B9600;   break; }
      case 19200:  { speed = B19200;  break; }
      case 38400:  { speed = B38400;  break; }
      case 57600:  { speed = B57600;  break; }
      case 115200: { speed = B115200; break; }
    }
    
    cfsetispeed(&options, speed);
    cfsetospeed(&options, speed);

    // Since we are going with 8 bit size, ignoring parity...
    options.c_cflag |= (CLOCAL | CREAD | CS8);
    options.c_iflag = (IGNBRK | IGNPAR);
    options.c_oflag = 0;
    // timeout properties
    options.c_cc[VMIN] = 1;
    tcflush(hSerial.fd, TCIFLUSH);
    tcsetattr(hSerial.fd, TCSANOW, &options);

    result = true;
    printf("Connected\n");
  }

  return result;
}


inline bool
unixSend(Serial& serial, uint8_t destID, uint8_t* txData, int32_t txLength) {
  bool result = false;
  serial_t& hSerial = serial.getSerialPort();

  int32_t bytesWritten = write(hSerial.fd, txData, txLength);
  if (bytesWritten < 0) {
    printf("write() has failed to send!\n");
  } else {
#ifdef SERIAL_DEBUG
      printf("**  Sent\t Length: %d, Sent: %d, destID: %d **\n", txLength, bytesWritten, destID);
#endif	
    result = true;
  }
  
  return result;
}


inline bool
unixRead(Serial& serial, uint8_t* rx_data, uint32_t* rx_len) {
  bool result = false;
  serial_t& hSerial = serial.getSerialPort();
#ifdef SERIAL_DEBUG
  printf("\n\nReading serial\n\n");
#endif
  int32_t bytesRead = read(hSerial.fd, rx_data, 256);
  if (bytesRead < 0) {
    printf("Failed to read from package. erro number %d\n", errno);
  } else {
    *rx_len = bytesRead;
#ifdef SERIAL_DEBUG
    printf("**  Recieved\t Length: %d  **\n", bytesRead);
#endif	    
    result = true;
  }

  return result;
}

#endif // COM_TARGET_OS == COM_OS_WINDOWS


inline bool
openPort(Serial& serial, const char* comPort, uint32_t baudrate) {
#if defined WINDOWS_SERIAL
  return initWindows(serial, comPort, baudrate);
#elif defined UNIX_SERIAL
  return initUnixSerial(serial, comPort, baudrate);
#endif
}

inline bool
sendToPort(Serial& serial, uint8_t destID, uint8_t* txData, uint32_t txLength) {
  serial_t& hSerial = serial.getSerialPort();
  if (hSerial.serial_s == SERIAL_CONNECTED) {
#if defined WINDOWS_SERIAL
    return windowsSend(serial, destID, txData, txLength);
#elif defined UNIX_SERIAL
    return unixSend(serial, destID, txData, txLength);
#endif
  }

  return false;
}


inline bool
readFromPort(Serial& serial, uint8_t* rx_data, uint32_t* rx_len) {
  serial_t& hSerial = serial.getSerialPort();
  if (hSerial.serial_s == SERIAL_CONNECTED) {
#if defined WINDOWS_SERIAL
    return windowsRead(serial, rx_data, rx_len);
#elif defined UNIX_SERIAL
    return unixRead(serial, rx_data, rx_len);
#endif
  }
  return false;
}


inline bool 
closePortHelper(Serial& serial) {
  serial_t& hSerial = serial.getSerialPort();
#if defined WINDOWS_SERIAL
  ClosePort(hSerial.h_serial);
#elif defined UNIX_SERIAL
  ClosePort(hSerial.fd);
#endif 
  return true;
}


/***********************************************/
/******************* Public  *******************/
/***********************************************/
Serial::Serial():CommsLink()
{		
  strcpy(terminal_sequence, "*&*");
  connectionEstablished = false;
  hSerial.serial_s = SERIAL_OPEN;
  parserPosition = 0;
  lastRecievedLength = 0;
}


Serial::~Serial() {  }


bool Serial::initConnection(const char* port, const char* address, uint32_t baudrate)
{ 
  
  //check os here
  connectionEstablished = openPort(*this, port, baudrate);
  hSerial.serial_s = SERIAL_CONNECTED;
  printf("Port is now: %d\n", hSerial.fd);
  return connectionEstablished;
}


bool Serial::send(uint8_t destID, uint8_t* txData, int32_t txLength)
{ 
  printf("Port send is: %d\n", hSerial.fd); 
  /*
  unsigned int crc = crc32(txData, txLength);//calculate crc32
  unsigned char a = (crc >> 24) & 0xff;//leftmost
  unsigned char b = (crc >> 16) & 0xff;//next byte
  unsigned char c = (crc >> 8) & 0xff;//next byte
  unsigned char d = (crc)& 0xff;//right most

#ifdef LITTLE_ENDIAN_COMNET
  //swap outter two
  unsigned char e = a;
  a = d;
  d = a;
  //swap middle two
  e = b;
  b = c;
  c = b;
#endif

  //add crc32
  txData[txLength++] = a;
  txData[txLength++] = b;
  txData[txLength++] = c;
  txData[txLength++] = d;
  */
  memset(serialBufferSend, 0, sizeof(serialBufferSend));
  memcpy(serialBufferSend, terminal_sequence, TERMINAL_SEQUENCE_SIZE);
  memcpy((serialBufferSend + TERMINAL_SEQUENCE_SIZE), txData, txLength);
  memcpy((serialBufferSend + TERMINAL_SEQUENCE_SIZE + txLength), terminal_sequence, TERMINAL_SEQUENCE_SIZE);  
  return sendToPort(*this, destID, serialBufferSend, txLength + (2 * TERMINAL_SEQUENCE_SIZE));
}



bool Serial::recv(uint8_t* rx_data, uint32_t* rx_len) {
	
	bool valid = true;
	printf("Parser Postion %d\n", parserPosition);
	printf("Last recieved Length %d\n", lastRecievedLength);
	if (parserPosition == 0 || parserPosition >= lastRecievedLength -1){
		
		printf("Port recv is: %d\n", hSerial.fd);
		parserPosition = 0;		
		valid = readFromPort(*this, serialBufferRecv, rx_len);

		lastRecievedLength = *rx_len;
		/*
		if (valid){
		unsigned char a = rx_data[--(*rx_len)];
		unsigned char b = rx_data[--(*rx_len)];
		unsigned char c = rx_data[--(*rx_len)];
		unsigned char d = rx_data[--(*rx_len)];
		#ifdef LITTLE_ENDIAN_COMNET
		//swap outter two
		unsigned char e = a;
		a = d;
		d = a;
		//swap middle two
		e = b;
		b = c;
		c = b;
		#endif
		//store bytes into crcRecv
		unsigned int crcRecv = 0;
		((char*)&crcRecv)[0] = a;
		((char*)&crcRecv)[1] = b;
		((char*)&crcRecv)[2] = c;
		((char*)&crcRecv)[3] = d;

		//get new crc
		unsigned int crc = crc32(rx_data, *rx_len);

		//compare and return results
		return crc == crcRecv;
		}
		*/
	}

	bool parsed = false;
	uint32_t messageLength = 0;
	while (!parsed && lastRecievedLength > 0){
		
		//check for sequence
		printf("ParserPosition: %d\n", parserPosition);
		if ((char)serialBufferRecv[parserPosition] == '*' && (char)serialBufferRecv[parserPosition + 1] == '&' && (char)serialBufferRecv[parserPosition + 2] == '*'){
			parserPosition += TERMINAL_SEQUENCE_SIZE;
			printf("ParserPosition: %d\n", parserPosition);
			bool done = false;
			while (!done && messageLength < 516){
				rx_data[messageLength++] = serialBufferRecv[parserPosition++];
				char a = serialBufferRecv[parserPosition];
				char b = serialBufferRecv[parserPosition + 1];
				char c = serialBufferRecv[parserPosition + 2];
				if (a == '*' && b == '&' && c == '*')done = true;
				
			}
			printf("%c\n", (char)serialBufferRecv[parserPosition+ 1]);
			printf("%c\n", (char)serialBufferRecv[parserPosition + 2]);
			printf("%c\n", (char)serialBufferRecv[parserPosition + 3]);
			printf("Message Length %d\n", messageLength);
			*rx_len = messageLength;
			parserPosition += TERMINAL_SEQUENCE_SIZE;
			printf("ParserPosition: %d\n", parserPosition);
			parsed = true;
		}
		else
		{
			parserPosition++;
			if (parserPosition > lastRecievedLength)parsed = true;
		}
	}
	
  return valid;
}


serial_status Serial::getStatus() {
  return hSerial.serial_s;
}


bool Serial::closePort() {
  return closePortHelper(*this);
}


serial_t& Serial::getSerialPort() {
  return hSerial;
}

unsigned int Serial::crc32(unsigned char *message, int length) {
	int i, j;
	unsigned int byte, crc, mask;
	static unsigned int table[256];

	/* Set up the table, if necessary. */

	if (table[1] == 0) {
		for (byte = 0; byte <= 255; byte++) {
			crc = byte;
			for (j = 7; j >= 0; j--) {    // Do eight times.
				mask = -(crc & 1);
				crc = (crc >> 1) ^ (0xEDB88320 & mask);
			}
			table[byte] = crc;
		}
	}

	/* Through with table setup, now calculate the CRC. */
	i = 0;
	crc = 0xFFFFFFFF;
	while (length--) {
		byte = message[i];
		crc = (crc >> 8) ^ table[(crc ^ byte) & 0xFF];
		i = i + 1;
	}
	return ~crc;
}


} // namespace Network
} // namespace Comnet 