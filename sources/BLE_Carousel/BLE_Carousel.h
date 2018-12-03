/*
  BLE_Carousel.h - Library for broadcasting  data in a carousel fashion in BLE advertisement packets.
  Created by Mathieu Cunche (mathieu.cunche@insa-lyon.fr)
  17/10/18
*/
#ifndef BLE_Carousel_h
#define BLE_Carousel_h

#include "Arduino.h"

class BLE_Carousel
{
  public:
  BLE_Carousel(int p_Packet_Size,int p_Header_Size);
  void set_Data(void * src, int length);
  int get_Nb_Packets();
  int get_Packet_Size();
  int get_Header_Size();
  int get_Payload_Size();

  void* get_Packet(int i);
  
  private:
  byte* data;
  int nb_Packets;
  int packet_Size;
  int header_Size;
  int payload_Size;
};

#endif
