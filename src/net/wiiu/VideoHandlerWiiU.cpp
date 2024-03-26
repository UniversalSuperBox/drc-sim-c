//
// Created by rolando on 5/8/17.
//

#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <thread>
#include <unistd.h>
#include "VideoHandlerWiiU.h"
#include "../../util/logging/Logger.h"
#include "../../Server.h"
#include "../../util/ImageUtil.h"
#include "../../Gamepad.h"
#include "../../data/Constants.h"

using namespace std;

VideoHandlerWiiU::VideoHandlerWiiU():
    frame_decode_num{0}, frame{}
{
    frame.reserve(FRAME_SIZE);
}

void VideoHandlerWiiU::update(unsigned char *packet, size_t packet_size, sockaddr_in *from_address,
                          unsigned int *address_size) {
    (void) from_address;
    (void) address_size;
    const VideoPacketWiiU &video_packet = VideoPacketWiiU(packet, packet_size);
    bool is_idr = is_idr_packet(video_packet.header);

    bool seq_ok = update_seq_id(video_packet.header->seq_id);

    if (is_streaming && !seq_ok) {
        Logger::debug(Logger::VIDEO, "Packets received out of order, cancelling frame.");
        is_streaming = false;
    }

    if (video_packet.header->frame_begin) {
        frame.clear();
        if (!is_streaming) {
            if (is_idr)
                is_streaming = true;
            else {
                unsigned char idr_request[] = {1, 0, 0, 0}; // Undocumented
                Gamepad::sendwiiu(Gamepad::socket_msg, idr_request, sizeof(idr_request), PORT_WII_MSG);
                return;
            }
        }
    }

    frame.insert(frame.end(), &video_packet.header->payload[0], &video_packet.header->payload[video_packet.header->payload_size]);

    if (is_streaming and video_packet.header->frame_end) {
        size_t frame_size = frame.size();
        if (frame_size > FRAME_SIZE) {
            Logger::info(Logger::VIDEO, "Video frame grew more than default vector size! %i", frame.size());
        }

        uint8_t *nals = new uint8_t[frame_size];
        int nals_size = h264_nal_encapsulate(is_idr, frame.data(), frame_size, nals);

        uint8_t *image_rgb = new uint8_t[2000000]; // ~2 megabytes
        decoder.image(nals, nals_size, image_rgb);

        uint8_t *image_jpeg = new uint8_t[2000000];
        int image_size_jpeg = ImageUtil::rgb_to_jpeg(image_rgb, image_jpeg);

        Server::broadcast_video(image_jpeg, (size_t) image_size_jpeg);

        delete [] nals;
        delete [] image_rgb;
        delete [] image_jpeg;
    }
    else if (video_packet.header->frame_end and !is_streaming) {
        Logger::debug(Logger::VIDEO, "Skipping video frame");
    }
}

bool VideoHandlerWiiU::is_idr_packet(VideoPacketHeaderWiiU *header) {
    for (size_t byte = 0; byte < sizeof(header->extended_header); ++byte) {
        if (header->extended_header[byte] == 0x80) {
            return true;
        }
    }
    return false;
}

int VideoHandlerWiiU::h264_nal_encapsulate(bool is_idr, uint8_t *frame, size_t frame_size, uint8_t *nals) {
    int slice_header = is_idr ? 0x25b804ff : (0x21e003ff | ((frame_decode_num & 0xff) << 13));
    frame_decode_num++;

    uint8_t params[] = {
            // sps
            0x00, 0x00, 0x00, 0x01, 0x67, 0x64, 0x00, 0x20, 0xac, 0x2b, 0x40, 0x6c, 0x1e, 0xf3, 0x68,
            // pps
            0x00, 0x00, 0x00, 0x01, 0x68, 0xee, 0x06, 0x0c, 0xe8
    };

    if (is_idr)
        memcpy(nals, params, sizeof(params));

    int params_offset = is_idr ? sizeof(params) : 0;

    // begin slice nalu
    uint8_t slice[] = {0x00, 0x00, 0x00, 0x01,
                       (uint8_t) ((slice_header >> 24) & 0xff),
                       (uint8_t) ((slice_header >> 16) & 0xff),
                       (uint8_t) ((slice_header >> 8) & 0xff),
                       (uint8_t) (slice_header & 0xff)
    };
    memcpy(nals + params_offset, slice, sizeof(slice));

    // Frame
    memcpy(nals + params_offset + sizeof(slice), frame, 2);

    // Escape codes
    int size = params_offset + sizeof(slice) + 2;
    for (size_t byte = 2; byte < frame_size; ++byte) {
        if (frame[byte] <= 3 and nals[size - 2] == 0 and nals[size - 1] == 0)
            nals[size++] = 3;
        nals[size++] = frame[byte];
    }
    return size;
}
