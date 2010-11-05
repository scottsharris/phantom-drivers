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
  * Phantom Library Communications: FireWire communication driver for Mac OS X
  */

#pragma once

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/firewire/IOFireWireLib.h>
#include <IOKit/firewire/IOFireWireLibIsoch.h>

namespace LibPhantom
{
  class DeviceIteratorMacOSX;
  class FirewireDeviceMacOSX;


  /**
   * Defines the communication methods to the firewire device (independent of the underlying library/driver)
   *
   * Do not create an instance of this class directly, instead use createInstance() to create a new instance of this class,
   * this function will return the correct underlying instance.
   */
  class CommunicationMacOSX:public Communication
  {
  public:
     CommunicationMacOSX();
    ~CommunicationMacOSX();
    DeviceIterator *getDevices();

  };


  class DeviceIteratorMacOSX:public DeviceIterator {
  public: //TODO: friend??
	  DeviceIteratorMacOSX();
  public:
	  FirewireDevice* next();
  private:
	  io_iterator_t deviceIterator;
	  io_iterator_t unitIterator;

  };


  class FirewireDeviceMacOSX: public FirewireDevice {
public: //TODO: friend??
	  FirewireDeviceMacOSX(IOFireWireLibDeviceRef interface);
	  ~FirewireDeviceMacOSX();
	  void read(unsigned long address, char *buffer, unsigned int length);
	  void write(unsigned long address, char *buffer, unsigned int length);



private:
	  IOFireWireLibDeviceRef interface;

  };
}

