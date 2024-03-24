//
// Created by rolando on 5/14/17.
//

#ifndef DRC_SIM_C_PACKETHANDLER_H
#define DRC_SIM_C_PACKETHANDLER_H


#include <cstddef>
#include <netinet/in.h>
#include <limits.h>

class PacketHandler {

public:
    virtual void update(unsigned char *packet, size_t size);

    virtual void update(unsigned char *packet, size_t packet_size, sockaddr_in *from_address,
                        unsigned int *address_size)= 0;

protected:
    bool update_seq_id(unsigned int seq_id);

    unsigned int seq_id_expected = UINT_MAX;
};


#endif //DRC_SIM_C_PACKETHANDLER_H
