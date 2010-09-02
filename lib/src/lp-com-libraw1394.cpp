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
  handles = new raw1394handle_t[ports];
  for(int i = 0; i < ports; i++)
    handles[i] = raw1394_new_handle_on_port(i);
  raw1394_destroy_handle(h);

  return ports;
}
