/*************************************************************************
 * COEN 233 Assignment 1 - server.c
 *
 * Name: Xin Guan
 * Student ID: 1610150
 * Date: 05/28/2021
 *
 * This file defines a server that can receive customized UDP packets and
 * send responses to the client with acknowledgement or a reject code.
 ************************************************************************/
#include "myPacket.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

// function prototype
int receiveAndResponse(int socketFd, uint8_t *buffer, int *lastSegNo);

int main() {
  int socketFd;
  uint8_t buffer[MAXLEN];
  struct sockaddr_in servAddr;

  // Create a UDP socket
  socketFd = socket(AF_INET, SOCK_DGRAM, 0);
  if (socketFd < 0) {
    perror("Socket creation failed.\n");
    exit(EXIT_FAILURE);
  }

  // set server address and port
  memset(&servAddr, 0, sizeof(servAddr));
  const uint16_t port = 2021;
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = INADDR_ANY;
  servAddr.sin_port = htons(port);

  // Bind server to the socket
  if (bind(socketFd, (const struct sockaddr *)&servAddr, sizeof(servAddr)) <
      0) {
    perror("Bind failed.\n");
    exit(EXIT_FAILURE);
  }

  // main loop to receive packets and respond
  int lastSegNo = -1;
  while (1) {
    int ret = receiveAndResponse(socketFd, buffer, &lastSegNo);
  }
}

/* This function receives a UDP packet and responds to the client
 * Parameters: socketFd - the socket identifier
 *             buffer - buffer to store the received packet
 *             lastSegNo - pointer to the last received segment number
 * Return: 0 - successfully received
 *         1 - packet was rejected
 */
int receiveAndResponse(int socketFd, uint8_t *buffer, int *lastSegNo) {
  struct sockaddr_in clientAddr;
  socklen_t len = sizeof(clientAddr);

  // receive packet
  int n = recvfrom(socketFd, buffer, MAXLEN, 0, (struct sockaddr *)&clientAddr,
                   &len);
  uint8_t data[MAXLEN];   // buffer to store the data
  uint8_t packet[MAXLEN]; // buffer to store the outgoing packet
  int clientId, dataLen, segNo;
  enum REJECT_CODE rejCode; // dummy variable for readPacket
  // parse the packet and detect potential errors
  enum PACKET_ERROR ret = readPacket(buffer, n, *lastSegNo + 1, &clientId, data,
                                     &dataLen, &segNo, &rejCode);

  if (ret == SUCCESS_DATA) {
    // successfully received a data packet
    printf("Successfully received packet segment %d from client %d\n", segNo,
           clientId);
    // build and send acknowledge packet to the client
    int l = buildAckPacket(packet, clientId, segNo);
    sendto(socketFd, packet, l, 0, (const struct sockaddr *)&clientAddr, len);
    *lastSegNo = segNo;
    return 0;
  } else if (ret < 0) {
    // error occurred, reject
    if (ret == OUT_OF_SEQ) {
      // out of sequence error detected
      printf("Reject packet segment %d from client %d. Reason: Out of sequence "
             "packet.\n",
             segNo, clientId);
      // build and send reject packet to the client
      int l = buildRejectPacket(packet, clientId, segNo, OUT_OF_SEQ_CODE);
      sendto(socketFd, packet, l, 0, (const struct sockaddr *)&clientAddr, len);
      return 1;
    } else if (ret == LEN_MISMATCH) {
      // length mismatch error detected
      printf("Reject packet segment %d from client %d. Reason: Data length "
             "mismatch\n",
             segNo, clientId);
      // build and send reject packet to the client
      int l = buildRejectPacket(packet, clientId, segNo, LEN_MISMATCH_CODE);
      sendto(socketFd, packet, l, 0, (const struct sockaddr *)&clientAddr, len);
      return 1;
    } else if (ret == DUPLICATE) {
      // duplicated segment number error detected
      printf("Reject packet segment %d from client %d. Reason: duplicated "
             "packets\n",
             segNo, clientId);
      // build and send reject packet to the client
      int l = buildRejectPacket(packet, clientId, segNo, DUPLICATE_CODE);
      sendto(socketFd, packet, l, 0, (const struct sockaddr *)&clientAddr, len);
      return 1;
    } else if (ret == NO_END_ID) {
      // No end of packet identifier error detected
      printf("Reject packet segment %d from client %d. Reason: incorrect end "
             "of packet ID\n",
             segNo, clientId);
      // build and send reject packet to the client
      int l = buildRejectPacket(packet, clientId, segNo, NO_END_ID_CODE);
      sendto(socketFd, packet, l, 0, (const struct sockaddr *)&clientAddr, len);
      return 1;
    }
  }
  return 1;
}