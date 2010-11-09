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

#pragma once

#include <sys/types.h>
#include "libraw1394/raw1394.h"

#include "lp-com.h"

namespace LibPhantom
{
  class DeviceIteratorLibraw1394;
  class FirewireDeviceLibraw1394;

  /**
   * Defines the communication methods to the firewire device (independent of the underlying library/driver)
   *
   * Do not create an instance of this class directly, instead use createInstance() to create a new instance of this class,
   * this function will return the correct underlying instance.
   */
  class CommunicationLibraw1394 : public Communication
  {
  public:
    CommunicationLibraw1394();
    ~CommunicationLibraw1394();
    DeviceIterator *getDevices();
  };

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

  class FirewireDeviceLibraw1394 : public FirewireDevice
  {
  public:
    //TODO: friend??
    FirewireDeviceLibraw1394(u_int32_t port, u_int32_t node);
    ~FirewireDeviceLibraw1394();
    void read(unsigned long address, char *buffer, unsigned int length);
    void write(unsigned long address, char *buffer, unsigned int length);

    static bool deviceIsOpen(u_int32_t port, u_int32_t node);
  protected:

    raw1394_handle *handle;

    u_int32_t port;
    u_int32_t node;

    static unsigned int max_open_devices;
    static unsigned int number_of_open_devices;
    static FirewireDeviceLibraw1394** open_devices;
  };
}

