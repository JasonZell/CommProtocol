#include <CommProto/network/seriallink.h>
#include <CommProto/network/serial.h>

#include <CommProto/architecture/macros.h>
#include <CommProto/tools/data_structures/auto_vector.h>

namespace comnet {
namespace network {


using namespace comnet::tools::datastructures;


class SerialConn {
public:
  Serial serial;
  uint8_t id;
};


SerialLink::SerialLink()
: local(NULL)
, connections(allocate_pointer(AutoVector<SerialConn*>))
{
}


SerialLink::~SerialLink()
{
  cleanupListPointerAttributes(connections);
  
  free_pointer(connections);
}


bool SerialLink::InitConnection(const char* port, const char* address, uint32_t baudrate) {
  bool success = false;

  if (!local) {
    local = allocate_pointer(Serial);
    success = local->OpenConnection(port, address, baudrate);
  }

  return success;
}


bool SerialLink::AddAddress(uint8_t destId, const char* address, uint16_t port) {
  bool success = false;
  //Serial* conn = allocate_pointer(Serial);
  /*
    TODO(Garcia) : We will need to figure this out later.
  */
  //conn->openConnection(port, address, local->getSerialPort().baudrate);
  return success;
}


bool SerialLink::RemoveAddress(uint8_t destId) {
  bool success = false;

  return success;
}


bool SerialLink::Send(uint8_t destId, uint8_t* txData, uint32_t txLength) {
  bool success = false;
#if 0
  for (int32_t i = 0; i < connections->getSize(); ++i) {
    SerialConn* s = connections->at(i);
    if (s->id == destId) {
      success = s->serial.send(destId, txData, txLength);
      break;  
    }
  }
#endif 
  // For now.
  success = local->Send(destId, txData, txLength);
  return success;
}


bool SerialLink::Recv(uint8_t* rxData, uint32_t* rxLength) {
  bool success = false;
#if 0
  for (int32_t i = 0; i < connections->getSize(); ++i) {
    SerialConn* s = connections->at(i);
    if (s->serial.recv(rxData, rxLength)) {
      success = true;
      break;
    }
  }
#endif
  // for now.
  success = local->Recv(rxData, rxLength);
  return success;
}
} // namespace Network
} // namespace Comnet