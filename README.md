# Arduino-CoDeSys-NetVars
Arduino implementation to send and receive CoDeSys Network Variables (NetVars)

## Example

```
#include <ESP8266WiFi.h>

#define CODESYS_IP      "225.10.10.11"
#define CODESYS_PORT    1202
#define CODESYS_COBID   4

WiFiUDP udp;
CoDeSysNetvars netvars(&udp, CODESYS_IP, CODESYS_PORT, CODESYS_COBID, CoDeSysNetvars::TT_ONCHANGE);

setup:
netvars.clear();
netvars.add( CoDeSysNetvars::VT_REAL, "Temp1" );
netvars.add( CoDeSysNetvars::VT_REAL, "Temp2" );
netvars.add( CoDeSysNetvars::VT_REAL, "Temp3" );
netvars.add( CoDeSysNetvars::VT_REAL, "Temp4" );
netvars.begin();

loop:
netvars[0] = 10.0f;
netvars["Temp2"] = 20.0f;
netvars.handle();
```
