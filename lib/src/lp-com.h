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

namespace LibPhantom
{
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

    /**
     * @returns the number of available ports on the system, or -1 if an error occurred
     */
    static int getPorts();

    /**
     * Set the port for the object
     */
    virtual int setPort(int port);

    /**
     * @return the number of nodes connected to the selected port
     */
    virtual unsigned int getNumberOfNodes();

    /**
     * Read data from given node and address on the selected port (see setPort())
     *
     * @return 0 when there where no errors
     */
    virtual int read(unsigned int node, unsigned long address, char *buffer, unsigned int length) = 0;

    /**
     * Write data to given node and address on the selected port (see setPort())
     *
     * @return 0 when there where no errors
     */
    virtual int write(unsigned int node, unsigned long address, char *buffer, unsigned int length) = 0;

    /**
     * @returns the vendor id of the given node, or 0 when an error occurred (assuming vendor id 0 is not used...)
     */
    unsigned int getVendorId(unsigned node);

    /**
     * @returns the name of the vendor if it is supplied in the ROM of the device, or 0 when an error occurred (ie the name is not available)
     */
    char *getVendorName(unsigned node);

    /**
     * @return true if the node is a SensAble device
     */
    bool isSensAbleDevice(unsigned int node);
  protected:
    /**
     * Cached value of the number of ports available on the current system.
     * (Assumes that this number will not change at runtime)
     * Read value with getPorts()
     */
    static int ports;

    /**
     * Port to which this object is connected to (or -1 if not connected yet)
     */
    int port;

    /**
     * Number of nodes connected to the port (cached value)
     */
    int nodes;

    /**
     * @returns the number of nodes without using the cached value (required to fill the cache)
     */
    virtual unsigned int getRealNumberOfNodes() = 0;

    /**
     * @param node is the node for which the config rom struct needs to be returned (it is not checked naymore since this is an internal function, so provide a valid node number!)
     *
     * @returns the config rom struct. Do not use directly, but use getters (eg getVendorId())
     */
    struct config_rom* getConfigRom(unsigned int node);

  private:
    /**
     * Number of config_rom structs that are cached in config_roms
     */
    static unsigned int number_of_config_roms;

    /**
     * Cached config roms, prevents to recreate them all the time
     */
    static struct config_rom** config_roms;
  };
}

