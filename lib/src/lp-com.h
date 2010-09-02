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
  class Communication
  {
  public:
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
  };
}

