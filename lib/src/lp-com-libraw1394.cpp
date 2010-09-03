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
  * Phantom Library Communications: Implement communications using libraw1394
  */

#include "lp-com-libraw1394.h"

using namespace LibPhantom;

raw1394handle_t* CommunicationLibraw1394::handles = 0;

CommunicationLibraw1394::CommunicationLibraw1394() : Communication()
{
}

CommunicationLibraw1394::~CommunicationLibraw1394()
{
}

int CommunicationLibraw1394::getPorts()
{
  if(ports != -1)
    return ports;

  raw1394handle_t h = raw1394_new_handle();
  if(h == 0)
    return -1;
  // Cache value, since we assume it will not change
  ports = raw1394_get_port_info(h, 0, 0);

  // Create handles bound to each available port
  handles = new raw1394handle_t[ports];
  for(int i = 0; i < ports; i++)
    handles[i] = raw1394_new_handle_on_port(i);
  raw1394_destroy_handle(h);

  return ports;
}

unsigned int CommunicationLibraw1394::getRealNumberOfNodes()
{
  // Checking wether the port is connected is done in getNumberOfNodes() already
  return raw1394_get_nodecount(handles[port]);
}

int CommunicationLibraw1394::read(unsigned int node, unsigned long address, char *buffer, unsigned int length)
{
  if(port == -1)
  {
    // Port is not set for this instance, so it is not possible to communicate
    // TODO Add some error indication (use errno?)
    return 1;
  }
  return raw1394_read(handles[port], node, address, length, (quadlet_t *) buffer);
}

int CommunicationLibraw1394::write(unsigned int node, unsigned long address, char *buffer, unsigned int length)
{
  if(port == -1)
  {
    // Port is not set for this instance, so it is not possible to communicate
    // TODO Add some error indication (use errno?)
    return 1;
  }
  return raw1394_write(handles[port], node, address, length, (quadlet_t *) buffer);
}
