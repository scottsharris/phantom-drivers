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

#pragma once

#include <stdint.h>
#include "libraw1394/raw1394.h"

#include "FirewireDevice.h"

namespace LibPhantom
{

  class FirewireDeviceLibraw1394 : public FirewireDevice
  {
  public:
    //TODO: friend??
    FirewireDeviceLibraw1394(u_int32_t port, nodeid_t node);
    ~FirewireDeviceLibraw1394();
    Communication * createCommunication();
    unsigned int getFreeChannel();
    void claimChannel(unsigned int channel);
    void releaseChannel(unsigned int channel);

    /**
     * Returns the port to which the device is connected to
     */
    u_int32_t getPort();

    /**
     * Returns the node to which the device is connected to
     */
    nodeid_t getNode();

    /**
     * @return true if the device on the given port and node is in use (open) already
     */
    static bool deviceIsOpen(u_int32_t port, nodeid_t node);
  protected:
    /**
     * Handle connected to the port given at the constructor
     */
    raw1394_handle *handle;

    u_int32_t port;
    nodeid_t node;

    /**
     * Isochronous Resource Manager, ie the node which manages the isochronous communication
     */
    nodeid_t irm_node;

    static unsigned int max_open_devices;
    static unsigned int number_of_open_devices;
    static FirewireDeviceLibraw1394** open_devices;
  };
}

