#include <CommProto/comms.h>
#include <CommProto/architecture/macros.h>

#include <CommProto/network/udplink.h>
#include <CommProto/network/seriallink.h>
#include <CommProto/network/xbeelink.h>

#include <CommProto/debug/comms_debug.h>

#include <CommProto/callback.h>

using namespace comnet;

/***********************************************/
/******************* Private *******************/
/***********************************************/


/**
Helper function to convert between C++ and C function signatures
due to casting as a class member being incompatible with C style
thread creation APIs. Static linkage helps with that.
*/
void* Comms::commuincationHelperSend(void* context)
{
	return ((Comms*)context)->commuincationHandlerSend();
}

void* Comms::commuincationHelperRecv(void* context)
{
	return ((Comms*)context)->commuincationHandlerRecv();
}


/** function for communication thread */
void* Comms::commuincationHandlerSend()
{
	while (this->IsRunning())
	{
		if (!send_queue->IsEmpty())
		{
			//Send data here
			ObjectStream *temp = send_queue->Front();
			send_queue->Dequeue();
			connectionLayer->Send(temp->header_packet.dest_id, temp->GetBuffer(), temp->GetSize());
			free_pointer(temp);
		}
	}
	return 0;
}

/** function for communication thread */
void* Comms::commuincationHandlerRecv() {
  while (this->IsRunning()) {
    AbstractPacket* packet = NULL;
    //Send data here
	  uint8_t streamBuffer[MAX_BUFFER_SIZE];
    uint32_t recvLen = 0;
    connectionLayer->Recv(streamBuffer, &recvLen);
    ObjectStream *temp = new ObjectStream();
    temp->SetBuffer((char*)streamBuffer, recvLen);

    /*
      Algorithm should get the header, get the message id from header, then
      produce the packet from the header, finally get the callback.
     */
    if (temp->GetSize() > 0) {
      Header header = temp->DeserializeHeader();
      // Create the packet.
      packet = this->packet_manager.ProduceFromId(header.msg_id);
    
      if (packet) {
        // Unpack the object stream.
        packet->Unpack(*temp);
        Callback* callback = NULL;
        callback = this->packet_manager.Get(*packet);

        if (callback) {
          error_t error;
          /*
            TODO(Wallace): This might need to Run on a separate thread, or 
            on a new thread, to prevent it from stopping the Receive handler.
            User figures out what to do with the packet.
          */
          error = callback->CallFunction(header, *packet);
          // Handle error.
          switch (error) {
            case CALLBACK_SUCCESS:
              cout << "Successful callback" << endl; break;
            default:  
              cout << "Nothing" << endl;
          };
        } else {
          // store the packet into the Receive queue.
          recv_queue->Enqueue(packet);
        }
      } else {
        COMMS_DEBUG("Unknown packet recieved.\n");
      }	
    }

    free_pointer(temp);				
  }

  return 0;
}

/***********************************************/
/******************* Public  *******************/
/***********************************************/
Comms::Comms(uint8_t platformID)
: CommNode(platformID)
{
	this->recv_queue = new AutoQueue <AbstractPacket*>;
	this->send_queue = new AutoQueue <ObjectStream*>;
	mutex_init(&sendMutex);
	mutex_init(&recvMutex);
	connectionLayer = NULL;
}

Comms::~Comms()
{
	free_pointer(connectionLayer);
	mutex_destroy(&sendMutex);
	mutex_destroy(&recvMutex);
}

bool Comms::InitConnection(transport_protocol_t connectionType, const char* port, const char* address, uint32_t baudrate)
{
	uint16_t length = 0;
	switch (connectionType)
	{
		case UDP_LINK:
		{			
			
			str_length(address, length);
			if (length < ADDRESS_LENGTH)
			{	
			  COMMS_DEBUG("UDP connection.\n");
				connectionLayer = new UDPLink();
				return connectionLayer->InitConnection(port, address);
			}
			break;
		}
		case SERIAL_LINK:
		{
			
			str_length(address, length);
			if (length < ADDRESS_LENGTH)
			{
				connectionLayer = new SerialLink();
				return connectionLayer->InitConnection(port, NULL, baudrate);
			}
			break;
		
		}
		case ZIGBEE_LINK:
		{
      str_length(address, length);
      if (length < ADDRESS_LENGTH) {
        connectionLayer = new experimental::XBeeLink();
        return connectionLayer->InitConnection(port, NULL, baudrate);
      }
      break;
    }
		default:
		  COMMS_DEBUG("NO CONNECTION\n");
		{return false;}
	}
	return true;
}


bool Comms::AddAddress(uint8_t dest_id, const char* address , uint16_t port)
{
	if (connectionLayer == NULL)return false;
	return connectionLayer->AddAddress(dest_id, address, port);
}


bool Comms::RemoveAddress(uint8_t dest_id)
{
	if (connectionLayer == NULL)return false;
	return connectionLayer->RemoveAddress(dest_id);
}


bool Comms::Send(AbstractPacket* packet, uint8_t dest_id) {
  if (connectionLayer == NULL) { 
    return false;
  }
  
  ObjectStream *stream = new ObjectStream();
  // Pack the stream with the packet.		
  packet->Pack(*stream);		
  Header header;

  header.dest_id = dest_id;
  header.source_id = this->GetNodeId();
  header.msg_id = packet->GetId();
  header.msg_len = stream->GetSize();
  //
  //call encryption here
  //
  stream->SerializeHeader(header);
  send_queue->Enqueue(stream);

  return true;
}


AbstractPacket* Comms::Receive(uint8_t&  source_id) {
  if (connectionLayer == NULL) return NULL;
  if (!recv_queue->IsEmpty()) {
    cout << "Message recv in Comms" << endl;
    // This is a manual Receive function. The user does not need to call this function,
    // however it SHOULD be used to manually grab a packet from the "orphanage" queue.
    recv_queue->Dequeue();  
  }
	
  return NULL;
}


int32_t Comms::Run()
{
	thread_create(&this->communicationThreadSend, commuincationHelperSend, this);
	thread_create(&this->communicationThreadRecv, commuincationHelperRecv, this);
	return CommNode::Run();
}


int32_t Comms::Stop()
{
	return CommNode::Stop();
}


int32_t Comms::Pause()
{
  return CommNode::Pause();
}