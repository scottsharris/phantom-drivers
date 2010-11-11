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
 * Phantom Library: implementation of Phantom functionality
 */

#pragma once

#include <stdint.h>
#include "BaseDevice.h"

namespace LibPhantom
{
  class PhantomIsoChannel;

  class Phantom : public BaseDevice
  {
  public:
    virtual ~Phantom();

    /**
     * @return an unused phantom device, or 0 if there are no unused phantom devices found.
     * @todo Return an unused Phantom devices with the lowest serial (to be able to easily select the same one over and over)
     */
    static Phantom* findPhantom();

    /**
     * @return the device with the given serial, or 0 if that device already is in use
     *         If the device with serial already got claimed, the pointer to the device is returned again (not a copy!!)
     */
    static Phantom* findPhantom(unsigned int serial);

    /**
     * @return serial id (read directly from device memory) or 0 when something went wrong
     */
    uint32_t readDeviceSerial();

    /**
     * Starts the communication with the phantom
     */
    void startPhantom();

    /**
     * Stops the communication with the phantom
     */
    void stopPhantom();

  protected:
    /**
     * When true, isochronous communication is enabled (ie the device is started)
     */
    bool started;

    /**
     * Transmit isochronous channel
     */
    PhantomIsoChannel* xmit_channel;

    /**
     * Receive isochronous channel
     */
    PhantomIsoChannel* recv_channel;

    /**
     * Do no use constructor directly, but use findPhantom() functionalities.
     */
    Phantom(FirewireDevice *fw);

    /**
     * @return serial id (read directly from device memory) or 0 when something went wrong
     */
    static uint32_t readDeviceSerial(FirewireDevice *firewireDevice);

  };
}
