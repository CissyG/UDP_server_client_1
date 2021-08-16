/*************************************************************************
 * COEN 233 Assignment 1 - myPacket.h
 *
 * Name: Xin Guan
 * Student ID: 1610150
 * Date: 05/28/2021
 *
 * This file is a header file. It contains a few enum definitions related
 * to the customized UDP packet. It also contains prototypes of functions
 * to read and build such packets.
 ************************************************************************/
#ifndef COEN233_PROJECT1_MYPACKET_H
#define COEN233_PROJECT1_MYPACKET_H

#include <stdlib.h>
#include <string.h>

#define MAXLEN 264 // maximum length of packet

// possible returns when reading a packet
enum PACKET_ERROR {
  OUT_OF_SEQ = -5,   // out of sequence error
  LEN_MISMATCH = -4, // length mismatch error
  NO_END_ID = -3,    // no end identifier error
  DUPLICATE = -2,    // duplicated packets error
  OTHER_ERROR = -1,  // other errors
  SUCCESS_DATA = 0,  // successfully received data packet
  SUCCESS_ACK = 1,   // successfully received ack packet
  SUCCESS_REJ = 2    // successfully received reject packet
};

// codes for reject reasons
enum REJECT_CODE {
  OUT_OF_SEQ_CODE = 0,   // out of sequence
  LEN_MISMATCH_CODE = 1, // length mismatch
  NO_END_ID_CODE = 2,    // no end identifier
  DUPLICATE_CODE = 3     // duplicated packet
};

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
                             int *segNo, enum REJECT_CODE *rejCode);

/* This function builds a data packet.
 * Parameters: packet - buffer to store the packet
 *             clientId - client Id
 *             segNo - segment number
 *             length - the length of the data
 *             data - pointer to the data
 * Return: length of the packet
 */
int buildDataPacket(uint8_t *packet, int clientId, int segmentNo, int length,
                    uint8_t *data);

/* This function builds a ack packet.
 * Parameters: packet - buffer to store the packet
 *             clientId - client Id
 *             segmentNo - segment number
 * Return: length of the packet
 */
int buildAckPacket(uint8_t *packet, int clientId, int segmentNo);

/* This function builds a reject packet.
 * Parameters: packet - buffer to store the packet
 *             clientId - client Id
 *             segmentNo - segment number
 *             rejCode - the reason for rejection
 * Return: length of the packet
 */
int buildRejectPacket(uint8_t *packet, int clientId, int segmentNo,
                      enum REJECT_CODE rejCode);

#endif // COEN233_PROJECT1_MYPACKET_H
