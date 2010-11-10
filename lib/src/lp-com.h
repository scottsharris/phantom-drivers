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
 * Phantom Library Communications: generic header file for communication over firewire
 */

#pragma once

#include <sys/types.h>

namespace LibPhantom
{
  //TODO Take a look at the fields and remove the ones which do not influence finding PHANTOM devices (ie vendor name)

  struct config_rom
  {
    unsigned char motu_fw_audio :1;
    unsigned char irm_cap :1;
    unsigned char cycle_master_cap :1;
    unsigned char iso_cap :1;
    unsigned char bus_manager_cap :1;
    unsigned char cycle_clk_accuracy;
    unsigned int max_async_bwrite_payload;
    unsigned int link_speed;
    union
    {
      struct
      {
        u_int32_t guid_lo;
        u_int32_t guid_hi;
      };
      u_int64_t guid;
    };
    u_int32_t node_capabilities;
    u_int32_t vendor_id;
    u_int32_t unit_spec_id;
    u_int32_t unit_sw_version;
    u_int32_t model_id;

    char *vendor;

  };

  class DeviceIterator;
  class FirewireDevice;

  /**
   * Defines the communication methods to the firewire device (independent of the underlying library/driver)
   *
   * Do not create an instance of this class directly, instead use createInstance() to create a new instance of this class, 
   * this function will return the correct underlying instance.
   */
  class Communication
  {
  public:
    /**
     * @return a new instance of the underlying communication method, which can be used to communicate with teh firewire devices
     */
    static Communication* createInstance();

    Communication();
    virtual ~Communication();
    virtual DeviceIterator* getDevices()=0;
  };

  class DeviceIterator
  {
  public:
    virtual FirewireDevice* next()=0;
  };

  class FirewireDevice
  {
  public:
    FirewireDevice();
    virtual ~FirewireDevice();

    /**
     * @return the first free isochronous channel available
     * @throws some exception if not free channels are available
     */
    virtual unsigned int getFreeChannel() = 0;

    /**
     * Claims the given isochronous channel, which can be found using getFreeChannel().
     */
    virtual void claimChannel(unsigned int channel) = 0;

    /**
     * Releases the given isochronous channel.
     */
    virtual void releaseChannel(unsigned int channel) = 0;

    /**
     * Read data from given address
     */
    virtual void read(u_int64_t address, char *buffer, unsigned int length) = 0;

    /**
     * Write data to given address
     */
    virtual void write(u_int64_t address, char *buffer, unsigned int length) = 0;

    /**
     * @return the vendor id of the device
     */
    unsigned int getVendorId();

    /**
     * @return the name of the vendor if it is supplied in the ROM of the device, or 0 when an error occurred (ie the name is not available)
     */
    char *getVendorName();

    /**
     * @return true if the node is a SensAble device
     */
    bool isSensableDevice();

    /**
     * @return the config rom struct. Do not use directly, but use getters (eg getVendorId())
     */
    struct config_rom* getConfigRom();
  private:
    void readConfigRom();
    struct config_rom configRom;
    bool configRomRead; //Did we try to read the config ROM?
    bool configRomValid; //Did we successfully read the config ROM?


  };

}

