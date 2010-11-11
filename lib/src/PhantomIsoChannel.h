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
 * Phantom Library: isochronous channel to communicate with a Phantom device
 */

#pragma once

namespace LibPhantom
{
  class Communication;
  class FirewireDevice;

  class PhantomIsoChannel
  {
  public:
    PhantomIsoChannel(FirewireDevice *firewireDevice, bool receiving);
    ~PhantomIsoChannel();

    /**
     * (Re)starts the isochrnous communication
     */
    void start();

    /**
     * Stops the isochronous communication
     */
    void stop();
  protected:
    /**
     * Phantom device to which the isochronous channels belongs to
     */
    FirewireDevice* firewireDevice;

    /**
     * When true this is a receiving channel
     */
    bool receiving;

    /**
     * Communication object for isochronous communication
     */
    Communication *com;

    /**
     * Configuration Communication object used to instruct Phantom about our isochronous communcation.
     */
    Communication *com_config;

    /**
     * Claimed isochronous channel
     */
    unsigned int channel;
  };
}
