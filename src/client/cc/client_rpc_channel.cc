#include "client_rpc_channel.h"
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <sails/net/packets.h>

using namespace std;
using namespace google::protobuf;


namespace sails {


RpcChannelImp::RpcChannelImp(string ip, int port):ip(ip),port(port) {
    assert(connector.connect(ip.c_str(), 8000, true));
}

void RpcChannelImp::CallMethod(const MethodDescriptor* method, 
			       RpcController *controller, 
			       const Message *request, 
			       Message *response, 
			       Closure *done) 
{
    int ret = this->sync_call(method, controller, request, response);
    if(ret == 0 && done != NULL) {
	done->Run();		
    }
}

net::PacketCommon* RpcChannelImp::parser(
    net::Connector *connector) {

    if (connector->readable() < sizeof(net::PacketCommon)) {
	return NULL;
    }
    net::PacketCommon *packet = (net::PacketCommon*)connector->peek();
    if (packet->type.opcode >= net::PACKET_MAX) { // error, and empty all data
	connector->retrieve(connector->readable());
	return NULL;
    }
    if (packet != NULL) {
	int packetlen = packet->len;
	if(connector->readable() >= packetlen) {
	    net::PacketCommon *item = (net::PacketCommon*)malloc(packetlen);
	    memset(item, 0, packetlen);
	    memcpy(item, packet, packetlen);
	    connector->retrieve(packetlen);
	    return item;
	}
    }
    return NULL;
}
int RpcChannelImp::sync_call(const google::protobuf::MethodDescriptor *method, 
			     google::protobuf::RpcController *controller, 
			     const google::protobuf::Message *request, 
			     google::protobuf::Message *response)
{

    const string service_name = method->service()->name();
    string content = request->SerializeAsString();

    int len = sizeof(net::PacketRPC)+content.length()-1;
    net::PacketRPC *packet = (net::PacketRPC*)malloc(len);
    memset(packet, 0, len);
    packet->common.type.opcode = net::PACKET_PROTOBUF_CALL;
    packet->common.len = len;
    memcpy(packet->service_name, service_name.c_str(), service_name.length());
    memcpy(packet->method_name, method->name().c_str(), method->name().length());
    packet->method_index = method->index();
    memcpy(packet->data, content.c_str(), content.length());

    connector.write((char *)packet, len);
    if (len <= 1000) {
      connector.send();
    }else {
      printf("error len:%d\n", len);
    }

    int n = connector.read();
    if(n > 0) {

	bool continueParse = false;
	do {
	    continueParse = false;
	    net::PacketCommon *resp = RpcChannelImp::parser(&connector);
	    
	    if (resp != NULL) {
		continueParse = true;
		char *body = ((net::PacketRPC*)resp)->data;
		string str_body(body, resp->len-sizeof(net::PacketRPC)+1);
		if(strlen(body) > 0) {
		    // protobuf message
		    response->ParseFromString(str_body);
		}
		delete(resp);
	    }


	}while(continueParse);
//	
    }

    return 0;
}


} // namespace sails