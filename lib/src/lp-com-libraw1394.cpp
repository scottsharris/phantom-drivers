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
 * Phantom Library Communications: FireWire communication driver for libraw1394
 */

#include <stdlib.h> // NULL
#include "lp-com-libraw1394.h"

using namespace LibPhantom;

CommunicationLibraw1394::CommunicationLibraw1394()
{
}

CommunicationLibraw1394::~CommunicationLibraw1394()
{
}

DeviceIterator *CommunicationLibraw1394::getDevices()
{
  return new DeviceIteratorLibraw1394();
}

// Number of ports is not available yet
// TODO On bus-reset set ports back to -1
int DeviceIteratorLibraw1394::ports = -1;

DeviceIteratorLibraw1394::DeviceIteratorLibraw1394() :
  port(0), node(0)
{
  if (getPorts() == 0)
  {
    nodes = 0;
    handle = 0;
  }
  else
  {
    handle = raw1394_new_handle_on_port(0);
    nodes = raw1394_get_nodecount(handle);
  }
}

DeviceIteratorLibraw1394::~DeviceIteratorLibraw1394()
{
  if (handle)
  {
    raw1394_destroy_handle( handle);
  }
}

FirewireDevice* DeviceIteratorLibraw1394::next()
{
  for (;;)
  {
    if (node >= nodes)
    {
      port++;
      if (port >= getPorts())
      {
        return NULL;
      }
      raw1394_destroy_handle( handle);
      handle = raw1394_new_handle_on_port(port);
      nodes = raw1394_get_nodecount(handle);
      node = 0;
    }

    if (!FirewireDeviceLibraw1394::deviceIsOpen(port, node))
    {
      FirewireDevice *device = new FirewireDeviceLibraw1394(port, node);
      node++;
      return device;
    }
    node++;
  }
}

int DeviceIteratorLibraw1394::getPorts()
{
  if (ports == -1)
  {
    raw1394handle_t h = raw1394_new_handle();
    if (h == 0)
    {
      // TODO Throw error
    }
    // Cache value, since we assume it will not change
    ports = raw1394_get_port_info(h, 0, 0);
    raw1394_destroy_handle(h);
  }
  return ports;
}

unsigned int FirewireDeviceLibraw1394::number_of_open_devices = 0;
unsigned int FirewireDeviceLibraw1394::max_open_devices = 2;
FirewireDeviceLibraw1394** FirewireDeviceLibraw1394::open_devices = (FirewireDeviceLibraw1394**) malloc(
    sizeof(FirewireDeviceLibraw1394**) * FirewireDeviceLibraw1394::max_open_devices);

FirewireDeviceLibraw1394::FirewireDeviceLibraw1394(u_int32_t port, u_int32_t node) :
  port(port), node(node)
{
  if (number_of_open_devices == max_open_devices)
  {
    max_open_devices += 2;
    open_devices = (FirewireDeviceLibraw1394**) realloc(open_devices, sizeof(FirewireDeviceLibraw1394**)
        * FirewireDeviceLibraw1394::max_open_devices);
    // TODO Recover & throw error if failed
  }
  open_devices[number_of_open_devices] = this;
  number_of_open_devices++;
  handle = raw1394_new_handle_on_port(port);
  // TODO throw error if failed
}

FirewireDeviceLibraw1394::~FirewireDeviceLibraw1394()
{
  // 'Close' the current Firewire Device
  number_of_open_devices--;
  unsigned int i;
  for (i = 0; i < number_of_open_devices; i++)
    if (open_devices[i] == this)
      break;
  for (i++; i < number_of_open_devices; i++)
    open_devices[i - 1] = open_devices[i];

  raw1394_destroy_handle( handle);
}

bool FirewireDeviceLibraw1394::deviceIsOpen(u_int32_t port, u_int32_t node)
{
  for (unsigned int i = 0; i < number_of_open_devices; i++)
    if (open_devices[i]->port == port && open_devices[i]->node == node)
      return true;
  return false;
}

void FirewireDeviceLibraw1394::read(unsigned long address, char *buffer, unsigned int length)
{
  // TODO throw exception when an error occurred
  raw1394_read(handle, node, address, length, (quadlet_t *) buffer);
}

void FirewireDeviceLibraw1394::write(unsigned long address, char *buffer, unsigned int length)
{
  // TODO throw exception when an error occurred
  raw1394_write(handle, node, address, length, (quadlet_t *) buffer);
}
