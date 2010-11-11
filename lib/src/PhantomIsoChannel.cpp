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
 * Phantom Library: isochronous channel to communicate with a Phantom device
 */

#include <stdio.h>

#include "PhantomIsoChannel.h"

#include "Communication.h"
#include "FirewireDevice.h"
#include "PhantomAddr.h"

using namespace LibPhantom;

PhantomIsoChannel::PhantomIsoChannel(FirewireDevice *firewireDevice, bool receiving) :
  firewireDevice(firewireDevice), receiving(receiving)
{
  com = Communication::createInstance(firewireDevice);
  com_config = Communication::createInstance(firewireDevice);

  channel = firewireDevice->getFreeChannel();
  firewireDevice->claimChannel(channel);
}

PhantomIsoChannel::~PhantomIsoChannel()
{
  firewireDevice->releaseChannel(channel);
  delete com;
  delete com_config;
}

void PhantomIsoChannel::start()
{
  unsigned char c;

  // Tell Phantom which isochronous channel is used for receiving/transmitting
  c = channel;
  com_config->write(receiving ? ADDR_RECV_CHANNEL : ADDR_XMIT_CHANNEL, (char *) &c, 1);

  if (receiving)
  {
    // Read back channel number (to see whether the device understood)
    com_config->read(ADDR_RECV_CHANNEL, (char *) &c, 1);
    if (c != channel)
    {
      // TODO Create some library exception and throw that one
      throw "Receiving unexpected value instead of receiving channel";
    }

    // Enable bit 6 (0x40) and see whether we can read it -> test to check whether device is working?
    c = 0x40;
    com_config->write(ADDR_RECV_CHANNEL, (char *) &c, 1);
    com_config->read(ADDR_RECV_CHANNEL, (char *) &c, 1);
    if (c != 0x40)
    {
      // TODO Create some library exception and throw that one
      throw "Receiving unexpected value instead of 0x40";
    }

    c = channel;
    com_config->write(ADDR_RECV_CHANNEL, (char *) &c, 1);
  }

  // TODO What is this doing? (copied from rev-eng/omni.c)
  com_config->read(0x1083, (char *) &c, 1);
  if (c != 0xc0) // -> c: 0xc0 = bit 6 & 7
  {
    // TODO Create some library exception and throw that one
    char *buf = new char[256];
    sprintf(buf, "line %d: Expected 0xc0 but got 0x%2.2x instead!\n", __LINE__, c);
    throw buf;
  }

  com_config->read(0x1082, (char *) &c, 1);
  if (c != 0)
  {
    // TODO Create some library exception and throw that one
    char *buf = new char[256];
    sprintf(buf, "line %d: Expected 0x00 but got 0x%2.2x instead!\n", __LINE__, c);
    throw buf;
  }

  // Tell Phanton to actual start the isochronous transfer
  com_config->read(ADDR_CONTROL, (char *) &c, 1);
  if (!(c & ADDR_CONTROL_enable_iso))
  {
    // Not started yet, so start
    c |= ADDR_CONTROL_enable_iso;
    com_config->write(ADDR_CONTROL, (char *) &c, 1);
  }

  if (receiving)
  {
    com->startRecvIsoTransfer(channel);
  }
  else
  {
    com->startXmitIsoTransfer(channel);
  }
}

void PhantomIsoChannel::stop()
{
  unsigned char c;

  // TODO What is this doing? (copied from rev-eng/omni.c)
  com_config->read(0x1083, (char *) &c, 1);
  if (c != 0xc0) // -> c: 0xc0 = bit 6 & 7
  {
    // TODO Create some library exception and throw that one
    char *buf = new char[256];
    sprintf(buf, "line %d: Expected 0xc0 but got 0x%2.2x instead!\n", __LINE__, c);
    throw buf;
  }

  com_config->read(0x1082, (char *) &c, 1);
  if (c != 0)
  {
    // TODO Create some library exception and throw that one
    char *buf = new char[256];
    sprintf(buf, "line %d: Expected 0x00 but got 0x%2.2x instead!\n", __LINE__, c);
    throw buf;
  }

  //TODO This stops both receiving and transmitting... might need some code to only stop if last transfer got stopped (instead of first)
  // Tell Phantom to stop the isochronous transfer
  com_config->read(ADDR_CONTROL, (char *) &c, 1);
  if (c & ADDR_CONTROL_enable_iso)
  {
    // Transfer is enabled, so stop it
    c &= ADDR_CONTROL_enable_iso;
    com_config->write(ADDR_CONTROL, (char *) &c, 1);
  }

  com->stopIsoTransfer();
}
