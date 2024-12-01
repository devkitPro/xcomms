#ifndef __WIN32__
#include "xboo.h"
#include "parport.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/io.h>

#include <linux/ppdev.h>
#include <sys/ioctl.h>

#include <FL/fl_ask.H>

#include <linux/gpio.h>

#include <linux/parport.h>

#include <memory>
#include <string>

// GND: Pin 6

// In bit order:
// STROBE: SC: Pin 5
// AUTOLF: SI: Pin 3
// SELECT: SD: Pin 4
// ACK: SO: Pin 2

// Bits:
// 0: SC
// 1: SI
// 2: SD
// 6: SO

class Parport
{
public:
  // PARPORT_CONTROL_INIT
  unsigned char parport_byte = 4;

  Parport(unsigned int port) : port(port)
  {
    std::string portname = "/dev/parport" + std::to_string(port);
    fd = open(portname.c_str(), O_RDWR);
    if (fd == -1)
    {
      fl_alert("Failed to open %s\nPlease select another port", portname.c_str());
      return;
    }

    if (ioctl(fd, PPCLAIM) == -1)
    {
      fl_alert("Failed to claim %s\nMake sure no other program currently uses this parallel port", portname.c_str());
      close(fd);
      return;
    }
  }

  ~Parport()
  {
    ioctl(fd, PPRELEASE);
    close(fd);
  }

  void InitPort()
  {
    ioctl(fd, PPWCONTROL, &parport_byte);
    ioctl(fd, PPWDATA, 0xff);
  }

  void PortDelay(int delay)
  {
    unsigned char al = 0;
    for (int i = 0; i < delay * 6; i++)
    {
      ioctl(fd, PPRDATA, &al);
    }
  }

  void XbooSend32(unsigned int Data)
  {
    unsigned int data = Data;
    for (int i = 0; i < 32; i++)
    {
      bool bLeadingBit = data & 0x80000000;
      data = data << 1;

      unsigned char byte_to_send = bLeadingBit ? PARPORT_CONTROL_STROBE | PARPORT_CONTROL_INIT : PARPORT_CONTROL_STROBE | PARPORT_CONTROL_AUTOFD | PARPORT_CONTROL_INIT;
      ioctl(fd, PPWCONTROL, &byte_to_send);
      // Unset strobe
      byte_to_send &= ~PARPORT_CONTROL_STROBE;
      ioctl(fd, PPWCONTROL, &byte_to_send);
      // Unset AUTO_LF
      byte_to_send &= ~PARPORT_CONTROL_AUTOFD;
      ioctl(fd, PPWCONTROL, &byte_to_send);
    }
  }

  uint32_t XbooExchange32(uint32_t Data)
  {
    // in/out buffer
    unsigned int data = Data;

    unsigned char al = 0;
    unsigned char ah = parport_byte;

    for (int i = 0; i < 32; i++)
    {
      bool bLeadingBit = data & 0x80000000;
      data = data << 1;

      al = bLeadingBit ? 0x02 : 0x0;
      ah &= bLeadingBit ? 0x0c : 0x0;

      // invert bit & SC low
      al ^= 0x3;
      // saved SD state & Reset
      al ^= ah;

      // write to control port
      ioctl(fd, PPWCONTROL, &al);

      // preserve data written
      ah = al;

      // read from status port
      ioctl(fd, PPRSTATUS, &al);

      // bit 6 (ACK) to carry (GBA SO)
      bool inbit = al & 0xc0;

      // ebx+=inbit

      // append read bit to data
      data += inbit;

      // restore data
      al = ah;

      // SC high
      al--;

      // write to control port
      ioctl(fd, PPWCONTROL, &al);

      // clear bit 1 (GBA SI high)
      // al &= 0xfd;
      al &= ~PARPORT_CONTROL_AUTOFD;

      // write to control port
      ioctl(fd, PPWCONTROL, &al);
    }

    parport_byte = al;
    return data;
  }

  void ResetGBA()
  {
    unsigned char ah = parport_byte;
    ah &= 0xfb;
    unsigned char al = ah;
    ioctl(fd, PPWDATA, &ah);

    for (int i = 0; i < 12 * 4; i++)
    {
      ioctl(fd, PPRDATA, &al);
    }

    // I suppose, this was used for debugging (?)
    // I see no reason to set this bit otherwise
    ah |= 4;
    al = ah;
    ioctl(fd, PPWDATA, &al);
  }

  unsigned short CalcCRC(unsigned int data)
  {
    unsigned int eax = (data & 0xffff0000) | ((data & 0x0000ffff) ^ CRC1);

    // For every bit, apply the CRC2 XOR on the lower 16 bits
    for (int i = 0; i < 32; i++)
    {
      bool bHadBit = eax & 1;
      eax >>= 1;
      if (bHadBit)
      {
        eax = (eax & 0xffff0000) | ((eax & 0x0000ffff) ^ CRC2);
      }
    }

    CRC1 = eax;
    return CRC1;
  }

  void InitCRC()
  {
    CRC1 = 0xc387;
    CRC2 = 0xc37b;
  }

  int ReadSOState()
  {
    unsigned char al;
    for (int i = 0; i < 8; i++)
    {
      ioctl(fd, PPRSTATUS, &al);
    }
    return al & PARPORT_STATUS_ACK;
  }

  short GetPortAddress()
  {
    return port;
  }

private:
  int fd;
  short port;

  unsigned short CRC1;
  unsigned short CRC2;
};

static std::unique_ptr<Parport> parport;

void ResetGBA()
{
  parport->ResetGBA();
}
void InitPort()
{
  parport->InitPort();
}

void PortDelay(int delay)
{
  parport->PortDelay(delay);
}
void InitCRC()
{
  parport->InitCRC();
}
int ReadSOState()
{
  return parport->ReadSOState();
}

void XbooSend32(unsigned int Data)
{
  parport->XbooSend32(Data);
}

unsigned int XbooExchange32(unsigned int Data)
{
  return parport->XbooExchange32(Data);
}
unsigned short CalcCRC(unsigned int data)
{
  return parport->CalcCRC(data);
}

short GetPortAddress()
{
  return parport->GetPortAddress();
}
void SetPortAddress(short Address)
{
  // Reopen the port for a new address
  if (!parport)
  {
    parport = std::make_unique<Parport>(Address);
    return;
  }

  if (parport->GetPortAddress() != Address)
  {
    parport.reset();
    parport = std::make_unique<Parport>(Address);
  }
}
#endif
