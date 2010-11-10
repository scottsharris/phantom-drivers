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
 * Phantom Library: generic header file for communication over firewire
 */

#pragma once

#include <stdint.h>
#include "DeviceIterator.h"

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
    virtual ~Communication();
    virtual DeviceIterator* getDevices()=0;

  protected:
    Communication();
  };
}

