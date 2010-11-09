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

#pragma once

#include <stdint.h>
#include "lp-com.h"

namespace LibPhantom
{
  class BaseDevice
  {
  public:
    /**
     * Reference to the FirewireDevice is consumed (i.e. the object is deleted
     * upon destruction of the BaseDevice
     */
    BaseDevice(FirewireDevice *fw);
    virtual ~BaseDevice();

    /**
     * @return the device serial/unique number directly from device
     */
    virtual uint32_t readDeviceSerial() = 0;

  protected:
    /**
     * Communication handle for device
     */
    FirewireDevice *firewireDevice;

  };
}
