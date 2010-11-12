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

#pragma once

#include "libraw1394/raw1394.h"

#include "DeviceIterator.h"

namespace LibPhantom
{
  class DeviceIteratorLibraw1394 : public DeviceIterator
  {
  public:
    //TODO: friend??
    DeviceIteratorLibraw1394();
    ~DeviceIteratorLibraw1394();
  public:
    FirewireDevice* next();
  protected:
    /**
     * Current port of this iterator
     */
    int port;

    /**
     * Current node of this iterator
     */
    int node;

    /**
     * Amount of available ports
     */
    static int ports;

    /**
     * Number of nodes available on current port
     */
    int nodes;

    /**
     * Libraw1394 handle which is connected to the current port
     */
    raw1394_handle *handle;

    /**
     * @return the number of available ports (cached)
     */
    int getPorts();
  };
}

