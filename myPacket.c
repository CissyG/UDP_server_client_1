/*************************************************************************
 * COEN 233 Assignment 1 - myPacket.c
 *
 * Name: Xin Guan
 * Student ID: 1610150
 * Date: 05/28/2021
 *
 * This file defines functions to read and build customized UDP packets.
 ************************************************************************/
#include "myPacket.h"
#include <stdlib.h>
#include <string.h>

/* This function reads a packet and returns whether it is successful.
 * Parameters: packet - buffer to store the packet
 *             nByte - the length of the packet
 *             expectedSeq - the expected segment number for data packets
 *             clientId - pointer to the client Id
 *             data - pointer to data buffer for data packets
 *             dataLen - pointer to the length of data for data packets
 *             segNo - pointer to segment number for data packets
 *             rejCode - reject reason for reject packets
 * Return: see enum PACKET_ERROR for details
 */
enum PACKET_ERROR readPacket(uint8_t *packet, int nByte, int expectedSeq,
                             int *clientId, uint8_t *data, int *dataLen,
                             int *segNo, enum REJECT_CODE *rejCode) {
  // verify the start of the packet
  if (packet[0] != 255 || packet[1] != 255) {
    return OTHER_ERROR;
  }

  *clientId = packet[2];
  if (packet[3] == 255 && packet[4] == 241) {
    // data packet
    int segment = packet[5];
    *segNo = segment;
    // detect duplicated packets
    if (segment == expectedSeq - 1) {
      return DUPLICATE;
    }
    // detect out of sequence packets
    if (segment != expectedSeq && segment != 0) {
      return OUT_OF_SEQ;
    }

    int length = packet[6];
    // detect length mismatch packets
    if (length != (nByte - 9)) {
      return LEN_MISMATCH;
    }

    // copy data to the data buffer
    memcpy(data, packet + 7, length);
    *dataLen = length;

    // detect no end identifier packets
    if (packet[7 + length] != 255 || packet[8 + length] != 255) {
      return NO_END_ID;
    }

    return SUCCESS_DATA;
  } else if (packet[3] == 255 && packet[4] == 242) {
    // ACK packet
    *segNo = packet[5];
    // detect no end identifier packets
    if (packet[6] != 255 || packet[7] != 255) {
      return NO_END_ID;
    }

    return SUCCESS_ACK;
  } else if (packet[3] == 255 && packet[4] == 243) {
    // Reject packet
    int rejectCode;
    // get the reject code
    if (packet[5] == 255 && packet[6] == 244) {
      rejectCode = OUT_OF_SEQ_CODE;
    } else if (packet[5] == 255 && packet[6] == 245) {
      rejectCode = LEN_MISMATCH_CODE;
    } else if (packet[5] == 255 && packet[6] == 246) {
      rejectCode = NO_END_ID_CODE;
    } else if (packet[5] == 255 && packet[6] == 247) {
      rejectCode = DUPLICATE_CODE;
    } else {
      return OTHER_ERROR;
    }
    *rejCode = rejectCode;

    *segNo = packet[7];
    // detect no end identifier packets
    if (packet[8] != 255 || packet[9] != 255) {
      return NO_END_ID;
    }

    return SUCCESS_REJ;
  } else {
    return OTHER_ERROR;
  }
}

/* This function builds a data packet.
 * Parameters: packet - buffer to store the packet
 *             clientId - client Id
 *             segNo - segment number
 *             length - the length of the data
 *             data - pointer to the data
 * Return: length of the packet
 */
int buildDataPacket(uint8_t *packet, int clientId, int segmentNo, int length,
                    uint8_t *data) {
  // start identifier
  packet[0] = 255;
  packet[1] = 255;
  // client ID
  packet[2] = clientId;
  // DATA
  packet[3] = 255;
  packet[4] = 241;
  // segment number
  packet[5] = segmentNo;
  // length of data
  packet[6] = length;
  // copy data payload
  memcpy(packet + 7, data, length);
  // end of packet identifier
  packet[7 + length] = 255;
  packet[8 + length] = 255;
  return 9 + length;
}

/* This function builds a ack packet.
 * Parameters: packet - buffer to store the packet
 *             clientId - client Id
 *             segmentNo - segment number
 * Return: length of the packet
 */
int buildAckPacket(uint8_t *packet, int clientId, int segmentNo) {
  // start identifier
  packet[0] = 255;
  packet[1] = 255;
  // client id
  packet[2] = clientId;
  // ACK
  packet[3] = 255;
  packet[4] = 242;
  // segment number
  packet[5] = segmentNo;
  // end of packet identifier
  packet[6] = 255;
  packet[7] = 255;
  return 8;
}

/* This function builds a reject packet.
 * Parameters: packet - buffer to store the packet
 *             clientId - client Id
 *             segmentNo - segment number
 *             rejCode - the reason for rejection
 * Return: length of the packet
 */
int buildRejectPacket(uint8_t *packet, int clientId, int segmentNo,
                      enum REJECT_CODE rejCode) {
  // start identifier
  packet[0] = 255;
  packet[1] = 255;
  // client id
  packet[2] = clientId;
  // REJECT
  packet[3] = 255;
  packet[4] = 243;
  // reject code
  if (rejCode == OUT_OF_SEQ_CODE) {
    packet[5] = 255;
    packet[6] = 244;
  } else if (rejCode == LEN_MISMATCH_CODE) {
    packet[5] = 255;
    packet[6] = 245;
  } else if (rejCode == NO_END_ID_CODE) {
    packet[5] = 255;
    packet[6] = 246;
  } else if (rejCode == DUPLICATE_CODE) {
    packet[5] = 255;
    packet[6] = 247;
  }
  // segment number
  packet[7] = segmentNo;
  // end of packet identifier
  packet[8] = 255;
  packet[9] = 255;
  return 10;
}