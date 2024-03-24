//
// Created by rolando on 5/8/17.
//

#ifndef DRC_SIM_C_VIDEOHANDLER_WII_U_H
#define DRC_SIM_C_VIDEOHANDLER_WII_U_H


#include "../PacketHandler.h"
#include "packet/VideoPacketWiiU.h"
#include "../../data/H264Decoder.h"

class VideoHandlerWiiU : PacketHandler {

public:
    VideoHandlerWiiU();

    bool is_idr_packet(VideoPacketHeaderWiiU *header);

private:
    bool is_streaming = false;
    uint32_t frame_index = 0;

    int h264_nal_encapsulate(bool is_idr, uint8_t *frame, size_t frame_size, uint8_t *nals);

    void update(unsigned char *packet, size_t packet_size, sockaddr_in *from_address,
                unsigned int *address_size) override;

    uint8_t *frame{nullptr};
    int frame_decode_num{0};
    H264Decoder decoder;
};


#endif //DRC_SIM_C_VIDEOHANDLER_WII_U_H
