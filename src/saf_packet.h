// Copyright (C) 2015 sails Authors.
// All rights reserved.
//
// Official git repository and contact information can be found at
// https://github.com/sails/sails and http://www.sailsxu.com/.
//
// Filename: saf_packet.h
// Description: saf消息包
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2015-05-21 15:56:17


#ifndef SRC_SAF_PACKET_H_
#define SRC_SAF_PACKET_H_

#include "sails/net/packets.h"

namespace sails {

#pragma pack(push, 1)

struct PacketRPCRequest : net::PacketCommon {
  uint16_t version;  // 客户端版本号，服务器可以针对老版本做兼容处理
  char service_name[20];
  int method_index;
  char data[1];
  explicit PacketRPCRequest(uint16_t len, uint32_t sn) {
    type.opcode = net::PacketDefine::PACKET_PROTOBUF_CALL;
    this->len = len;
    this->sn = sn;
    version = 1;
    service_name[0] = '\0';
    method_index = 0;
    data[0] = '\0';
  }
};



struct PacketRPCResponse : net::PacketCommon {
  char data[1];
  explicit PacketRPCResponse(uint16_t len, uint32_t sn) {
    type.opcode = net::PacketDefine::PACKET_PROTOBUF_CALL;
    this->len = len;
    this->sn = sn;
    data[0] = '\0';
  }
};

#pragma pack(pop)

}  // namespace sails

#endif  // SRC_SAF_PACKET_H_
