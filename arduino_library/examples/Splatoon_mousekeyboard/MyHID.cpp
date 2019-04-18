
#include "MyHID.hpp"

static int vid_pid_cmp(vid_pid_t a, vid_pid_t b) {
  return ((a.vid==b.vid) && (a.pid==b.pid));
}

uint8_t MyHID::OnInitSuccessful() {
  
  vid_pid_t new_device = { HIDUniversal::VID, HIDUniversal::PID };
  int i=0;
  char buf[50] = {0};;
  int found = 0;
  
  if(device_list == NULL) {
    PRINT("No devices in list");
    Release();
    return 1;
  }

  for(int i=0; device_list[i].last==0; i++) {
    sprintf(buf, "Comparing %04X:%04X %04X:%04X", new_device.vid, new_device.pid, device_list[i].vid_pid.vid, device_list[i].vid_pid.pid);
    PRINT(buf);
    hid_device_n = i;
    if(vid_pid_cmp(new_device, device_list[i].vid_pid)) {
      found = 1;
      break;
    }
  }

  if(found==1) {
    PRINT("driver found");
    delay(10);
    if(device_list[hid_device_n].OnInitSuccessful)
      device_list[hid_device_n].OnInitSuccessful();
    return 0;
  }
  PRINT("Unknown device");
  delay(10);
  //Release();
  return 1;
}
void MyHID::ParseHIDData(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
  if(device_list[hid_device_n].ParseHID)
    device_list[hid_device_n].ParseHID(buf, len);
}
