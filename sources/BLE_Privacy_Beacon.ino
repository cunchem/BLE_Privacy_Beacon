/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE_Carousel.h>
#include <iostream>
#include <algorithm>
#include <ArduinoJson.h>
using namespace std;

#define PACKET_SIZE 24
#define HEADER_SIZE 3 // Object ID 1 byte, Object size 1 byte  Packet Number 1 byte

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b" // UUID dedicated to consent transmission
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8" // Characterisitic that will receive the consent
int idx = 0;
BLEAdvertising *pAdvertising;
BLE_Carousel carousel = BLE_Carousel(PACKET_SIZE,HEADER_SIZE);

// Define a callback to handle consent reception
// (ie when the corresponding characteristic is modify by a write)
//  found here : http://bbs.esp32.com/viewtopic.php?f=19&t=4181
class MyConsentCallBack: public BLECharacteristicCallbacks {
  
struct PilotRule
{
  std::string datatype;
  std::string entity;
  std::string purpose;
  int retention_time;
};


struct PilotPolicy 
{
  std::vector<PilotRule> rules;
};

PilotPolicy stringToPolicy(string policy) //policy should be formatted according to pilot_policy.proto but Jsonified see https://arduinojson.org/v5/assistant/
{
  char *cstr = new char[policy.length() + 1];
  strcpy(cstr, policy.c_str());

const size_t capacity = 6*JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(1) + 9*JSON_OBJECT_SIZE(2) + 300;
DynamicJsonBuffer jsonBuffer(capacity);


JsonObject& root = jsonBuffer.parseObject(cstr);

JsonArray& pilotRule = root["pilotRule"];

struct PilotPolicy pilotPolicy;
for (int i = 0; i< pilotRule.size(); i++)
{
  struct PilotRule rule;  
  rule.datatype=pilotRule[i]["datatype"].as<char*>();
  rule.entity=pilotRule[i]["dcr"][0]["entity"].as<char*>();
  rule.purpose=pilotRule[i]["dcr"][0]["dur"][0]["purpose"].as<char*>();
  rule.retention_time=pilotRule[i]["dcr"][0]["dur"][0]["retentionTime"].as<int>();
  pilotPolicy.rules.push_back(rule);
}

return pilotPolicy;
} 


void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();
     if (rxValue.length() > 0) {
      Serial.println("*********");
      Serial.println("Received a new consent: ");
      Serial.print("Length:");
      Serial.println(rxValue.length());
      Serial.print("Value: ");
      for (int i = 0; i < rxValue.length(); i++)
        Serial.print(rxValue[i]);
      Serial.println();
      Serial.println("*********");
      // Insert Consent Parser here
    }
    if (rxValue.find ('&') != std::string::npos) {
      Serial.println("Contains DS policy");
      rxValue.erase(std::remove(rxValue.begin(), rxValue.end(), '&'), rxValue.end());
      struct PilotPolicy DSP;
      DSP = stringToPolicy(rxValue);
      struct PilotPolicy inter = intersectionPolicies(dummyDCP(),DSP);
      std::cout << inter.rules[0].datatype << "\n";      
    }  
}
  



PilotPolicy dummyDCP()
{
  struct PilotPolicy DCP;
  struct PilotRule rule1;
  struct PilotRule rule2;
  
  rule1.datatype="Location";
  rule1.entity="Interparking";
  rule1.purpose="Marketing";
  rule1.retention_time=30;
  
  rule2.datatype="Location";
  rule2.entity="Interparking";
  rule2.purpose="Analytics";
  rule2.retention_time=30;  
  
  DCP.rules.push_back(rule1);
  DCP.rules.push_back(rule2);
  
  return DCP;   
}


PilotPolicy dummyDSP()
{
  struct PilotPolicy DSP;
  struct PilotRule rule1;

  rule1.datatype="Location";
  rule1.entity="Interparking";
  rule1.purpose="Analytics";
  rule1.retention_time=30;

  DSP.rules.push_back(rule1);
  
  return DSP;   
}


PilotPolicy intersectionPolicies(PilotPolicy DCP, PilotPolicy DSP)
{
  std::vector<PilotRule> rules;

  for(int i = 0; i < DCP.rules.size(); i++)
  {
    for(int j = 0; j < DSP.rules.size(); j++)
    {
      if((DCP.rules.at(i).datatype == DSP.rules.at(j).datatype)&&(DCP.rules.at(i).entity == DSP.rules.at(j).entity)&&(DCP.rules.at(i).purpose == DSP.rules.at(j).purpose)&&(DCP.rules.at(i).retention_time <= DSP.rules.at(j).retention_time)) //
      {
        rules.push_back(DCP.rules.at(i));
      }
    }
  }
  
  struct PilotPolicy intersection;
  for(int k = 0; k<rules.size(); k++)
  {
    intersection.rules.push_back(rules.at(k));
  }
  return intersection;
}
    
};




void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");


  BLEDevice::init("InCon_Beacon");
  BLEServer *pServer = BLEDevice::createServer();
  
  
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  // Configure the characteristic        
  // The maximum length of an attribute value shall be 512 octets.
  pCharacteristic->setValue("Consent");
  pCharacteristic->setCallbacks(new MyConsentCallBack());
  Serial.println("Characteristic defined!");
  Serial.println("Ready to receive consents!");


  
  // Allocate the buffer that will contain the privacy policy
  int pp_size = 100;          
  //byte pp[pp_size] = { 10, 44, 10, 17, 87, 105, 45, 70, 105, 32, 77, 65, 67, 32, 65, 100, 100, 114, 101, 115, 115, 18, 23, 18, 6, 71, 111, 111, 103, 108, 101, 26, 13, 10, 9, 77, 97, 114, 107, 101, 116, 105, 110, 103, 16, 20 };
  //byte pp[pp_size] = { 10, 41, 10, 8, 76, 111, 99, 97, 116, 105, 111, 110, 18, 29, 18, 12, 73, 110, 116, 101, 114, 112, 97, 114, 107, 105, 110, 103, 26, 13, 10, 9, 77, 97, 114, 107, 101, 116, 105, 110, 103, 16, 20 };
   byte pp[pp_size] = { 10, 41, 10, 8, 76, 111, 99, 97, 116, 105, 111, 110, 18, 29, 18, 12, 73, 110, 116, 101, 114, 112, 97, 114, 107, 105, 110, 103, 26, 13, 10, 9, 77, 97, 114, 107, 101, 116, 105, 110, 103, 16, 30, 10, 41, 10, 8, 76, 111, 99, 97, 116, 105, 111, 110, 18, 29, 18, 12, 73, 110, 116, 101, 114, 112, 97, 114, 107, 105, 110, 103, 26, 13, 10, 9, 65, 110, 97, 108, 121, 116, 105, 99, 115, 16, 30 };

  // Initialize the Carousel
  carousel.set_Data(pp,pp_size);
  
  // Configure the payload of advertising packets
  byte*  packet = (byte*)carousel.get_Packet( 0);  
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
  oAdvertisementData.setFlags(0x04); // BR_EDR_NOT_SUPPORTED 0x04
  
  //std::string strServiceData = "";
  std::string strServiceData = "";
  
  strServiceData += (char)11;     // Len
  strServiceData += (char)0xFF;   // Type
  strServiceData += "0123456789";
  oAdvertisementData.addData(strServiceData);

  
   pAdvertising = pServer->getAdvertising();
  //Serial.println("Payload:");
  //Serial.println(oAdvertisementData.getPayload().c_str());
  //pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oScanResponseData);
  set_new_adv_payload(idx);

  // Start service and advertising
  pService->start();
  pAdvertising->start();


}

void set_new_adv_payload(int idx){

  //Serial.println(idx);
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  oAdvertisementData.setFlags(0x04); // BR_EDR_NOT_SUPPORTED 0x04
  std::string strServiceData = "";
  //Serial.println(carousel.get_Nb_Packets());
  strServiceData += (char)(PACKET_SIZE+1);     // Len
  strServiceData += (char)0xFF;   // Type

   byte*  packet = (byte*)carousel.get_Packet( idx);
  for(int i=0; i < PACKET_SIZE; i++){
    strServiceData += (char) packet[i];
  }
  free(packet);
  //Serial.println(idx);
  oAdvertisementData.addData(strServiceData); 
  pAdvertising->setAdvertisementData(oAdvertisementData);
}

void loop() {
  delay(200);
  //Serial.println(idx);

  set_new_adv_payload(idx);
  
  
  
  idx = (idx + 1) % carousel.get_Nb_Packets();
}


//void myDSP(){
//  PilotPolicy myDSP = PilotPolicy_init_zero;
//  PilotPolicy_PilotRule rule;
//  PilotPolicy_PilotRule_DCR dcr;
//  PilotPolicy_PilotRule_DCR_DUR dur;
//  dur.purpose = "purpose";
//  dur.retention_time = 30;
//  dcr.entity = "entity";
//  dcr.dur = dur;
//  rule.datatype = "datatype";
//  rule.dcr = dcr;
//  myDSP.rule = rule;
//  
//}
