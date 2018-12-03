/*
  BLE_Carousel.cpp - Library for broadcasting  data in a carousel fashion in BLE advertisement packets.
  Created by Mathieu Cunche (mathieu.cunche@insa-lyon.fr)
  17/10/18
*/

#include "Arduino.h"
#include "BLE_Carousel.h"


BLE_Carousel::BLE_Carousel(int p_Packet_Size,int p_Header_Size ){
  packet_Size = p_Packet_Size;
  header_Size = p_Header_Size;
  payload_Size = packet_Size - header_Size;
  nb_Packets = 0;
  
  
}

void BLE_Carousel::set_Data(void * src, int length){
  // Update the number of packets to be generated
  nb_Packets = length / (payload_Size);
  if( (length % payload_Size) > 0){
	nb_Packets++;
  }
 
  // Allocate memory space and copy the data and the padding
  data = (byte* )malloc(nb_Packets* payload_Size * sizeof(byte)  );
  memset(data, 0, nb_Packets* payload_Size   * sizeof(byte)  );
  memcpy(data, src, length *  sizeof(byte));

}
int BLE_Carousel::get_Nb_Packets(){

  return nb_Packets;
}

int BLE_Carousel::get_Packet_Size(){

  return packet_Size;
}

int BLE_Carousel::get_Payload_Size(){

  return payload_Size;
}

int BLE_Carousel::get_Header_Size(){

  return header_Size;
}

void* BLE_Carousel::get_Packet(int i){
  //Serial.println("get_Packet::malloc");

  // create a buffer to return
  void * packet = malloc(packet_Size * sizeof(byte));
  //Serial.println("get_Packet::memset");
  //memset(packet, 0, sizeof(packet) );
  
  memset(packet, 0,packet_Size * sizeof(byte) );
  //Serial.println("get_Packet::header");

  // fill the header
  ((byte*)packet) [0] = 0;
  ((byte*)packet) [1] = nb_Packets;
  ((byte*)packet) [2] = i;
  //Serial.println("get_Packet::memcpy");

  // fill the payload
  memcpy(packet + header_Size * sizeof(byte) , data +  i * payload_Size * sizeof(byte) , payload_Size * sizeof(byte));

  
  return packet;
  
}

