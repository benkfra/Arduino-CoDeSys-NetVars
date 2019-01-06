
#ifndef CoDeSysNetvars_h
#define CoDeSysNetvars_h

#include <stdint.h>

#ifdef ESP8266
# include <ESP8266WiFi.h>
# include <WiFiUDP.h>
#else
class WiFiUDP;
#endif

// maximum number of items the network struct can contain
#define CSNV_MAX_ITEMS (16)
// maximum length of itemname
#define CSNV_MAX_ITEMNAME (16)
// maximum length of IP-Address
#define CSNV_MAX_IPADDRLEN (20)

/*
 * TODO:
 *
 * - add hostname capability e.g.
 *     IPAddress myip(0, 0, 0, 0);
 *     DNSClient dns;
 *     dns.begin(Ethernet.dnsServerIP());
 *     dns.getHostByName("pool.ntp.org",myip);
 */

class CoDeSysNetvars
{
  // typedefs
public:
  typedef enum
  {
    VT_NONE = 0,
    VT_BOOL,  // unsigned 8 bit
    VT_BYTE,  // unsigned 8 bit
    VT_WORD,  // unsigned 16 bit
    VT_DWORD, // unsigned 32 bit
    VT_SINT,  // signed 8 bit
    VT_USINT, // unsigned 8 bit
    VT_INT,   // signed 16 bit
    VT_UINT,  // unsigned 16 bit
    VT_DINT,  // signed 32 bit
    VT_UDINT, // unsigned 32 bit
    VT_DT,    // timestamp unsigned 32 bit
    VT_REAL,  // float 32 bit
    //VT_LREAL,  // double 64 bit
    //VT_STRING, // (scheinbar) immer 81 byte lang, jedoch nur bis zur ersten 0 gueltig
    VT_LAST
  } variable_t;

  typedef enum
  {
    TT_NONE = 0,
    TT_ONCHANGE,
    TT_LAST
  } trigger_t;

protected:
  typedef struct
  {
    char name[CSNV_MAX_ITEMNAME];
    CoDeSysNetvars::variable_t varType;

    union {
      int8_t i8;
      uint8_t ui8;
      int16_t i16;
      uint16_t ui16;
      int32_t i32;
      uint32_t ui32;
      float f;
      // double d;
      // char s[81];
    } data;
  } item_t;

  typedef struct
  {
    uint32_t identity;
    uint32_t id;
    uint16_t index; /* CobId */
    uint16_t subIndex;
    uint16_t numItems;
    uint16_t len;
    uint16_t nextCounter;
    uint8_t flags;
    uint8_t checksum;
  } networkHeader_t;

private:
  // con- and destructors
public:
  CoDeSysNetvars(void);
  CoDeSysNetvars(WiFiUDP *wifiUdp, const char *ipAddr, uint16_t port, uint16_t cobId, CoDeSysNetvars::trigger_t triggerType = TT_ONCHANGE);
  virtual ~CoDeSysNetvars(void);

protected:
private:
  // methods
public:
  bool clear(void);
  bool add(CoDeSysNetvars::variable_t varType, const char *varName);

  bool begin(void);
  bool handle(void);

  template <typename T>
  bool set(const int idx, const T value);

  template <typename T>
  bool set(const char *varName, const T value);

protected:
private:
  // attributes
public:
protected:
  char _ipAddr[CSNV_MAX_IPADDRLEN];
  unsigned short _port;
  uint16_t _cobId;
  item_t _items[CSNV_MAX_ITEMS];
  bool _dataChanged;
  WiFiUDP *_wifiUdp;

private:
};

#endif /* CoDeSysNetvars_h */
