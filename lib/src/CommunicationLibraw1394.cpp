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
 * Phantom Library: FireWire communication driver for libraw1394
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "CommunicationLibraw1394.h"
#include "DeviceIteratorLibraw1394.h"

using namespace LibPhantom;

CommunicationLibraw1394::CommunicationLibraw1394(unsigned int port, nodeid_t node) :
  node(node)
{
  handle = raw1394_new_handle_on_port(port);
  // Add pointer to ourself to the handle, so the callback functions can be used more easily
  raw1394_set_userdata(handle, this);
}

CommunicationLibraw1394::~CommunicationLibraw1394()
{
  raw1394_destroy_handle( handle);
}

void CommunicationLibraw1394::read(u_int64_t address, char *buffer, unsigned int length)
{
  read(node, address, buffer, length);
}

void CommunicationLibraw1394::read(nodeid_t node, u_int64_t address, char *buffer, unsigned int length)
{
  if (raw1394_read(handle, node, address, length, (quadlet_t *) buffer))
  {
    if (errno != EAGAIN)
    {
      // TODO Create some library exception and throw that one
      char *buffer = new char[256];
      sprintf(buffer, "Failed to read data at address 0x%lx from device %d: (%d) %s\n", address, node, errno, strerror(
          errno));
      throw buffer;
    }
  }
}

void CommunicationLibraw1394::write(u_int64_t address, char *buffer, unsigned int length)
{
  write(node, address, buffer, length);
}

void CommunicationLibraw1394::write(nodeid_t node, u_int64_t address, char *buffer, unsigned int length)
{
  if (raw1394_write(handle, node, address, length, (quadlet_t *) buffer))
  {
    if (errno != EAGAIN)
    {
      // TODO Create some library exception and throw that one
      char *buffer = new char[256];
      sprintf(buffer, "Failed to read data at address 0x%lx from device %d: (%d) %s\n", address, node, errno, strerror(
          errno));
      throw buffer;
    }
  }
}

void CommunicationLibraw1394::startRecvIsoTransfer(unsigned int channel, PhantomIsoChannel *iso_channel)
{
  Communication::startRecvIsoTransfer(channel, iso_channel);

  raw1394_iso_recv_init(handle, &recv_handler, 1000, 64, channel, RAW1394_DMA_DEFAULT, 1);
  raw1394_iso_recv_start(handle, -1, -1, 0);
}

void CommunicationLibraw1394::startXmitIsoTransfer(unsigned int channel, PhantomIsoChannel *iso_channel)
{
  Communication::startXmitIsoTransfer(channel, iso_channel);

  raw1394_iso_xmit_init(handle, &xmit_handler, 1000, 64, channel, RAW1394_ISO_SPEED_100, 1);
  raw1394_iso_xmit_start(handle, -1, -1);
}

void CommunicationLibraw1394::stopIsoTransfer()
{
  raw1394_iso_shutdown( handle);
}

void CommunicationLibraw1394::doIterate()
{
  if (raw1394_loop_iterate(handle))
  {
    if (errno != 0 && errno != EAGAIN)
    {
      // TODO Create some library exception and throw that one
      throw "Something went wrong in an iterate loop...";
    }
  }
}

enum raw1394_iso_disposition CommunicationLibraw1394::recv_handler(raw1394handle_t handle, unsigned char *data,
    unsigned int len, unsigned char channel, unsigned char tag, unsigned char sy, unsigned int cycle,
    unsigned int dropped)
{
  CommunicationLibraw1394 *com = (CommunicationLibraw1394 *) raw1394_get_userdata(handle);
  com->callbackRecvHandler(data, len);
  return RAW1394_ISO_OK;
}

enum raw1394_iso_disposition CommunicationLibraw1394::xmit_handler(raw1394handle_t handle, unsigned char *data,
    unsigned int *len, unsigned char *tag, unsigned char *sy, int cycle, unsigned int dropped)
{
  CommunicationLibraw1394 *com = (CommunicationLibraw1394 *) raw1394_get_userdata(handle);
  com->callbackXmitHandler(data, len);
  *tag = 0;
  *sy = 0;

  return RAW1394_ISO_OK;
}
