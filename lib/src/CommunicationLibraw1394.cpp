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
}

CommunicationLibraw1394::~CommunicationLibraw1394()
{
  raw1394_destroy_handle(handle);
}

void CommunicationLibraw1394::read(u_int64_t address, char *buffer, unsigned int length)
{
  read(node, address, buffer, length);
}

void CommunicationLibraw1394::read(nodeid_t node, u_int64_t address, char *buffer, unsigned int length)
{
  if (raw1394_read(handle, node | 0xffc0, address, length, (quadlet_t *) buffer))
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
