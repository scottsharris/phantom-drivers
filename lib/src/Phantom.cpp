/*
 * This file is part of phantom-drivers.
 *
 * phantom-drivers is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * phantom-drivers is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with phantom-drivers.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Phantom Library: implementation of Phantom functionality
 */

#include "DeviceIterator.h"
#include "Phantom.h"
#include "PhantomIsoChannel.h"

using namespace LibPhantom;

Phantom::Phantom(FirewireDevice *fw) :
  BaseDevice(fw), started(false)
{

}

Phantom::~Phantom()
{
  // TODO Remove callback handler(s)
  stopPhantom();
}

Phantom* Phantom::findPhantom()
{
  return findPhantom(0);
}

// If serial is 0, any Phantom device will suffice
Phantom* Phantom::findPhantom(unsigned int serial)
{
  // TODO Cache the Phantom devices and recreate list upon bus resets (much more efficient, assuming that it does not take too much resources to create the list...)

  FirewireDevice *dev;
  DeviceIterator *i = DeviceIterator::createInstance();

  for (dev = i->next(); dev; dev = i->next())
  {
    if (dev->isSensableDevice())
    {
      uint32_t device_serial = readDeviceSerial(dev);
      if (serial != 0 && serial != device_serial)
      {
        delete dev;
        continue;
      }

      //We have found a Phantom device
      delete i;
      return new Phantom(dev);
    }

    delete dev;
  }
  delete i;

  // We failed to find a(n unused) Phantom device...
  return 0;
}

uint32_t Phantom::readDeviceSerial()
{
  return readDeviceSerial(firewireDevice);
}

uint32_t Phantom::readDeviceSerial(FirewireDevice *firewireDevice)
{
  uint32_t serial;
  // For a PHANTOM Omni this address can be read to obtain the serial/unique number
  firewireDevice->read(0x10060010, (char *) &serial, 4);
  return serial;
}

void Phantom::startPhantom()
{
  if (started)
  {
    // TODO Create some library exception and throw that one
    throw "This phantom device is already started";
  }
  started = true;

  try
  {
    recv_channel = new PhantomIsoChannel(firewireDevice, true);
    try
    {
//      xmit_channel = new PhantomIsoChannel(firewireDevice, false);
    }
    catch (...)
    {
      // Failed to get both channels, free recv_channel
      delete recv_channel;
      throw;
    }
  }
  catch (...)
  {
    // Failed to start isochronous communication with phantom
    started = false;
    throw;
  }

  recv_channel->start();
//  xmit_channel->start();
}

void Phantom::stopPhantom()
{
  if (!started)
  {
    return;
  }
  started = false;
  recv_channel->stop();
//  xmit_channel->stop();

  delete recv_channel;
//  delete xmit_channel;
}

void Phantom::isoIterate()
{
  recv_channel->iterate();
//  xmit_channel->iterate();
}
