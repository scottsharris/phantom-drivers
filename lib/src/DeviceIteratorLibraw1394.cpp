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
 * Phantom Library: iterator to iterate over FirewireDevices, libraw1394 implementation
 */

#include <stdlib.h>     // NULL
#include "DeviceIteratorLibraw1394.h"
#include "FirewireDeviceLibraw1394.h"

using namespace LibPhantom;

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
      // Firewire nodes start at 0xffc0 and counts upwards (see specs... something about local bus address)
      FirewireDevice *device = new FirewireDeviceLibraw1394(port, node | 0xffc0);
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
