/*
 * CoDeSysNetvars.cpp
 * 
 * Module to send so called Network Variables to CoDeSys powered devices.
 * Currently the only transport layer is UDP Multicast.
 * And the only supported Trigger to send the data is "on change".
 * 
 */

#include <string.h>

#include "CoDeSysNetvars.h"

#define TELEGRAM_IDENT_3S 0x33532D00

CoDeSysNetvars::CoDeSysNetvars(void)
    : _port(0), _wifiUdp(NULL), _cobId(0)
{
  strncpy(_ipAddr, "", sizeof(_ipAddr));
}

CoDeSysNetvars::CoDeSysNetvars(WiFiUDP *wifiUdp, const char *ipAddr, uint16_t port, uint16_t cobId, CoDeSysNetvars::trigger_t triggerType /* = TT_ONCHANGE */)
    : _port(port), _wifiUdp(wifiUdp), _cobId(cobId)
{
  strncpy(_ipAddr, ipAddr, sizeof(_ipAddr));
}

CoDeSysNetvars::~CoDeSysNetvars(void)
{
}

bool CoDeSysNetvars::clear(void)
{
  return false;
}

bool CoDeSysNetvars::add(CoDeSysNetvars::variable_t varType, const char *varName)
{
  if ((varType <= VT_NONE) || (varType >= VT_LAST))
  {
    return false;
  }
  if (varName[0] == 0x00)
  {
    return false;
  }

  for (int ia = 0; ia < CSNV_MAX_ITEMS; ia++)
  {
    if (_items[ia].varType == VT_NONE)
    {
      _items[ia].varType = varType;
      strncpy(_items[ia].name, varName, sizeof(_items[ia].name));
      _items[ia].data.ui32 = 0;
      return true;
    }
  }
  return false;
}

bool CoDeSysNetvars::begin(void)
{
#ifdef ESP8266
  return true;
#endif
  return false;
}

bool CoDeSysNetvars::handle(void)
{
  if (_dataChanged)
  {
    char buffer[256];
    int bufferLen = 0;
    memset(buffer, 0, sizeof(buffer));

    networkHeader_t header;
    memset(&header, 0, sizeof(networkHeader_t));

    header.identity = TELEGRAM_IDENT_3S;
    header.id = 0;         /* ODSTATEFREE */
    header.index = _cobId; /* CobId */
    header.nextCounter = 1;
    header.numItems = 0;

    // send data
    for (int ia = 0; ia < CSNV_MAX_ITEMS; ia++)
    {
      switch (_items[ia].varType)
      {
      case VT_SINT:  // signed 8 bit
      case VT_BOOL:  // unsigned 8 bit
      case VT_BYTE:  // unsigned 8 bit
      case VT_USINT: // unsigned 8 bit
        memcpy(&buffer[bufferLen], &_items[ia].data.ui8, 1);
        bufferLen += 1;
        header.numItems++;
        break;

      case VT_INT:  // signed 16 bit
      case VT_WORD: // unsigned 16 bit
      case VT_UINT: // unsigned 16 bit
        memcpy(&buffer[bufferLen], &_items[ia].data.ui16, 2);
        bufferLen += 2;
        header.numItems++;
        break;

      case VT_DINT:  // signed 32 bit
      case VT_DWORD: // unsigned 32 bit
      case VT_UDINT: // unsigned 32 bit
      case VT_DT:    // timestamp unsigned 32 bit
      case VT_REAL:  // float 32 bit
        memcpy(&buffer[bufferLen], &_items[ia].data.ui16, 4);
        bufferLen += 4;
        header.numItems++;
        break;

        //case VT_LREAL:  // double 64 bit
        //case VT_STRING: // (scheinbar) immer 81 byte lang, jedoch nur bis zur ersten 0 gueltig

      case VT_NONE: // erlaubt, aber ohne Funktion
        break;
      default:
        Serial.println("FATAL: falscher Datentyp in Konfiguration");
        return false;
        break;
      }
    }
    header.len = sizeof(networkHeader_t) + bufferLen;

#ifdef ESP8266
    IPAddress ip;
    ip.fromString(_ipAddr);

    if (_wifiUdp->beginPacketMulticast(ip, _port, WiFi.localIP(), 5) == false)
    {
      Serial.println("ERROR: beginPacket returned ERROR");
      return false;
    }
    if (_wifiUdp->write((const uint8_t *)&header, sizeof(networkHeader_t)) != sizeof(networkHeader_t))
    {
      Serial.println("ERROR: beginPacket returned ERROR");
      return false;
    }
    if (_wifiUdp->write(buffer, bufferLen) != bufferLen)
    {
      Serial.println("ERROR: beginPacket returned ERROR");
      return false;
    }
    if (_wifiUdp->endPacket() == false)
    {
      Serial.println("ERROR: beginPacket returned ERROR");
      return false;
    }
#endif

    _dataChanged = false;
  }

  return false;
}

template <typename T>
bool CoDeSysNetvars::set(int idx, const T value)
{
  if ((idx < 0) || (idx >= CSNV_MAX_ITEMS))
  {
    return false;
  }
  if ((_items[idx].name[0] == 0) || (_items[idx].varType == VT_NONE))
  {
    return false;
  }

  switch (_items[idx].varType)
  {
  case VT_SINT: // signed 8 bit
    _items[idx].data.i8 = (int8_t)value;
    break;
  case VT_BOOL:  // unsigned 8 bit
  case VT_BYTE:  // unsigned 8 bit
  case VT_USINT: // unsigned 8 bit
    _items[idx].data.ui8 = (uint8_t)value;
    break;

  case VT_INT: // signed 16 bit
    _items[idx].data.i16 = (int16_t)value;
    break;
  case VT_WORD: // unsigned 16 bit
  case VT_UINT: // unsigned 16 bit
    _items[idx].data.ui16 = (uint16_t)value;
    break;

  case VT_DINT: // signed 32 bit
    _items[idx].data.i32 = (int32_t)value;
    break;
  case VT_DWORD: // unsigned 32 bit
  case VT_UDINT: // unsigned 32 bit
  case VT_DT:    // timestamp unsigned 32 bit
    _items[idx].data.ui32 = (uint32_t)value;
    break;

  case VT_REAL: // float 32 bit
    _items[idx].data.f = (float)value;
    break;

    //case VT_LREAL:  // double 64 bit
    //case VT_STRING: // (scheinbar) immer 81 byte lang, jedoch nur bis zur ersten 0 gueltig

  default:
    Serial.println("FATAL: falscher Datentyp bei <set> angegeben");
    return false;
    break;
  }

  _dataChanged = true;
  return true;
}

template <typename T>
bool CoDeSysNetvars::set(const char *varName, const T value)
{
  for (int ia = 0; ia < CSNV_MAX_ITEMS; ia++)
  {
    if (0 == strcmp(varName, _items[ia].name))
    {
      return set(ia, value);
    }
  }
  return false;
}

// explicite instantiation
template bool CoDeSysNetvars::set(const int, const float);
template bool CoDeSysNetvars::set(const char *, const float);
template bool CoDeSysNetvars::set(const int, const int);
template bool CoDeSysNetvars::set(const char *, const int);
template bool CoDeSysNetvars::set(const int, const unsigned long);
template bool CoDeSysNetvars::set(const char *, const unsigned long);
