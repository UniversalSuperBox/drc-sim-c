//
// Created by rolando on 5/14/17.
//

#include "PacketHandler.h"
#include "../util/logging/Logger.h"

bool PacketHandler::update_seq_id(unsigned int seq_id) {
    bool matched = true;
    if (seq_id_expected == UINT_MAX) {
        Logger::verbose(Logger::PACKET, "Initializing packet sequence to %i", seq_id);
        seq_id_expected = seq_id;
    }
    else if (seq_id_expected != seq_id) {
        Logger::info(Logger::PACKET, "Out of order packet; expected %i got %i", seq_id_expected, seq_id);
        matched = false;
    }
    seq_id_expected = (seq_id + 1) & 0x3ff;  // 10 bit number
    return matched;
}

void PacketHandler::update(unsigned char *packet, size_t size) {
    update(packet, size, NULL, NULL);
}
