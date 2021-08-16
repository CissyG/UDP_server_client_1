/*************************************************************************
 * COEN 233 Assignment 1 - client.c
 *
 * Name: Xin Guan
 * Student ID: 1610150
 * Date: 05/28/2021
 *
 * This file defines a client that sends multiple UDP packets to a
 * server and receives responses from the server with acknowledgement
 * or a reject code. It will resend the packet multiple times if it does
 * not receive the response in ack_timer.
 ************************************************************************/
#include "myPacket.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// function prototype
void sendAndWaitForResponse(int socketFd, uint8_t *packet, int packetLen,
                            struct sockaddr_in *servAddr);

int main() {
  int socketFd;
  struct sockaddr_in servAddr;

  // Create a UDP socket
  socketFd = socket(AF_INET, SOCK_DGRAM, 0);
  if (socketFd < 0) {
    perror("Socket creation failed.");
    exit(EXIT_FAILURE);
  }

  // set server address and port
  memset(&servAddr, 0, sizeof(servAddr));
  int port = 2021;
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = INADDR_ANY;
  servAddr.sin_port = htons(port);

  uint8_t data[MAXLEN] = {0}; // dummy data to be sent
  uint8_t packet[MAXLEN];     // buffer to store the outgoing packet
  // build and send 5 correct packets
  int l;
  for (int segNo = 0; segNo < 5; segNo++) {
    printf("Sending correct packet (segment %d)\n", segNo);
    l = buildDataPacket(packet, 0, segNo, 240, data);
    sendAndWaitForResponse(socketFd, packet, l, &servAddr);
  }

  // send another one correct packet
  printf("Sending correct packet (segment %d)\n", 5);
  l = buildDataPacket(packet, 0, 5, 240, data);
  sendAndWaitForResponse(socketFd, packet, l, &servAddr);
  // out of sequence case
  printf("Sending out of sequence packet\n");
  l = buildDataPacket(packet, 0, 7, 240, data);
  sendAndWaitForResponse(socketFd, packet, l, &servAddr);
  // length mismatch case
  printf("Sending length mismatch packet\n");
  l = buildDataPacket(packet, 0, 6, 240, data);
  packet[6] = 239;
  sendAndWaitForResponse(socketFd, packet, l, &servAddr);
  // incorrect end packet id case
  printf("Sending incorrect end packet ID packet\n");
  l = buildDataPacket(packet, 0, 6, 240, data);
  packet[l - 1] = 241;
  sendAndWaitForResponse(socketFd, packet, l, &servAddr);
  // duplicated segment number case
  printf("Sending duplicated packet\n");
  l = buildDataPacket(packet, 0, 5, 240, data);
  sendAndWaitForResponse(socketFd, packet, l, &servAddr);

  close(socketFd);
  return 0;
}

/* This function sends a UDP packet to the server and wait for
 * response from the server. It will resend 3 times if it does not
 * receive a response within 3 seconds.
 * Parameters: socketFd - the socket identifier
 *             packet - buffer with the outgoing packet
 *             packetLen - the length of the packet
 *             servAddr = the address of server
 */
void sendAndWaitForResponse(int socketFd, uint8_t *packet, int packetLen,
                            struct sockaddr_in *servAddr) {
  uint8_t buffer[MAXLEN]; // buffer to store the received packet
  uint8_t data[MAXLEN];   // dummy data buffer
  int clientId, dataLen, segNo;
  enum REJECT_CODE rejCode;
  socklen_t len;
  int count = 0;
  // resend loop if we do not receive the response
  while (count < 3) {
    // send the packet to server
    sendto(socketFd, packet, packetLen, 0, (const struct sockaddr *)servAddr,
           sizeof(*servAddr));

    // set timer for 3 seconds
    fd_set sock_set;
    FD_ZERO(&sock_set);
    FD_SET(socketFd, &sock_set);
    struct timeval ack_timer = {0, 0};
    ack_timer.tv_sec = 3;
    if (select(socketFd + 1, &sock_set, NULL, NULL, &ack_timer) > 0) {
      // received the packet within 3 seconds
      int n = recvfrom(socketFd, buffer, MAXLEN, 0, (struct sockaddr *)servAddr,
                       &len);
      enum PACKET_ERROR ret =
          readPacket(buffer, n, 0, &clientId, data, &dataLen, &segNo, &rejCode);
      if (ret == SUCCESS_ACK) {
        // received acknowledgement
        printf("Received server ACK for segment %d\n", segNo);
      } else if (ret == SUCCESS_REJ) {
        // rejected code from server
        if (rejCode == OUT_OF_SEQ_CODE) {
          printf("Received server reject of segment %d. Reject reason: out of "
                 "sequence.\n",
                 segNo);
        } else if (rejCode == NO_END_ID_CODE) {
          printf("Received server reject of segment %d. Reject reason: wrong "
                 "end of packet.\n",
                 segNo);
        } else if (rejCode == LEN_MISMATCH_CODE) {
          printf("Received server reject of segment %d. Reject reason: data "
                 "length mismatch.\n",
                 segNo);
        } else if (rejCode == DUPLICATE_CODE) {
          printf("Received server reject of segment %d. Reject reason: "
                 "duplicated packets.\n",
                 segNo);
        }
      }
      break;
    } else {
      // time out for the packet
      printf("Time out. ");
      count++;
      if (count < 3) {
        // resend the packet
        printf("Re-sending packet. \n");
      } else {
        // we have sent the packet three times, so do not resend anymore
        printf("Server does not respond\n");
      }
    }
  }
}