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

#include "libraw1394/raw1394.h"

#include "Communication.h"

namespace LibPhantom
{
  /**
   * Defines the communication methods to the firewire device (libraw1394 implementation)
   *
   * Do not create an instance of this class directly, instead use Communication::createInstance() to create a new instance of this class,
   * this function will return the correct underlying instance.
   */
  class CommunicationLibraw1394 : public Communication
  {
  public:
    CommunicationLibraw1394(unsigned int port, nodeid_t node);
    ~CommunicationLibraw1394();

    virtual void read(u_int64_t address, char *buffer, unsigned int length);
    void read(nodeid_t node, u_int64_t address, char *buffer, unsigned int length);

    virtual void write(u_int64_t address, char *buffer, unsigned int length);
    void write(nodeid_t node, u_int64_t address, char *buffer, unsigned int length);

    virtual void startRecvIsoTransfer(unsigned int channel, PhantomIsoChannel *iso_channel);
    virtual void startXmitIsoTransfer(unsigned int channel, PhantomIsoChannel *iso_channel);
    virtual void stopIsoTransfer();
    virtual void doIterate();
  protected:

   /**
    * Node to which this Communication object is communication with
    */
    nodeid_t node;

    /**
     * Handle used for Communication with the node
     */
    raw1394_handle *handle;

  private:
    static enum raw1394_iso_disposition xmit_handler(raw1394handle_t handle, unsigned char *data, unsigned int *len,
        unsigned char *tag, unsigned char *sy, int cycle, unsigned int dropped);
    static enum raw1394_iso_disposition recv_handler(raw1394handle_t handle, unsigned char *data, unsigned int len,
        unsigned char channel, unsigned char tag, unsigned char sy, unsigned int cycle, unsigned int dropped);
  };
}

