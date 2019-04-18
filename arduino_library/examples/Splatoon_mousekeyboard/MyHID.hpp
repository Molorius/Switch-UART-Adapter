
#ifndef MYHID_HPP_
#define MYHID_HPP_

#include <hiduniversal.h>

//#define DEBUG

#ifdef DEBUG
#define BEGIN Serial.begin(115200)
#define PRINT(x) Serial.println(x)
#else
#define BEGIN 
#define PRINT(x)
#endif

#define DEVICE_UNKNOWN 0
#define DEVICE_MOUSE   1
#define DEVICE_KBD     2

typedef struct {
  uint16_t vid;
  uint16_t pid;
} vid_pid_t;

typedef struct {
  vid_pid_t vid_pid;
  uint8_t (*OnInitSuccessful)(void);
  void (*ParseHID)(uint8_t *buf, uint8_t len);
  int last;
} hid_device_t;

class MyHID : public HIDUniversal {
  public:
    MyHID(USB *p, hid_device_t* d) : HIDUniversal(p) { device_list = d; };
    hid_device_t* device_list;
    int hid_device_n;
  private:
    void ParseHIDData(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf);
    uint8_t OnInitSuccessful();
    
};





#endif
