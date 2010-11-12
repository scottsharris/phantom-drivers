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
 * Phantom Library: iterator to iterate over FirewireDevices, Mac OS X implementation
 */

#include "DeviceIteratorMacOSX.h"
#include "FirewireDeviceMacOSX.h"
#include <string.h>

using namespace LibPhantom;

DeviceIteratorMacOSX::DeviceIteratorMacOSX()
{
  kern_return_t result;

  //Create a matching directory that finds Firewire devices and create an
  //iterator
  CFMutableDictionaryRef matchingDictionary = IOServiceMatching("IOFireWireDevice");

  result = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDictionary, &deviceIterator);

  unitIterator = 0;
}

FirewireDevice* DeviceIteratorMacOSX::next()
{
  io_object_t device;
  kern_return_t result;

  /* Use the deviceIterator to iterate over all Firewire devices (independent
   * of which bus, this is taken care of by the OS)
   *
   * Then, for each device, iterate over all Units
   */

  for (;;)
  {

    if (!unitIterator)
    { //If there are no more units in this device
      //Get the next device
      device = IOIteratorNext(deviceIterator);
      if (!device)
        return NULL; //There are no more devices, return NULL

      //Get the iterator to iterate over this device
      result = IORegistryEntryGetChildIterator(device, kIOServicePlane, &unitIterator);

      if (result != KERN_SUCCESS)
      {
        unitIterator = 0;
        continue;
      }
    }

    //Find the next unit using the unitIterator
    io_object_t unit;
    unit = IOIteratorNext(unitIterator);
    if (!unit)
    {
      //Continue on the next device
      unitIterator = 0;
      continue;
    }

    //See if this is an IOFireWireUnit
    io_name_t name;
    IORegistryEntryGetName(unit, name);

    if (strcmp(name, "IOFireWireUnit"))
    { //If it is *not* an IOFireWireUnit
      IOObjectRelease(unit);
      continue;
    }

    //Create the plugin_interface
    IOCFPlugInInterface ** plugin_interface = NULL;
    SInt32 score;
    result = IOCreatePlugInInterfaceForService(unit, kIOFireWireLibTypeID, kIOCFPlugInInterfaceID, &plugin_interface,
        &score);

    IOObjectRelease(unit);
    if (result != KERN_SUCCESS)
      continue;

    //Get a reference to the interface
    IOFireWireLibDeviceRef iface = NULL;
    (*plugin_interface)->QueryInterface(plugin_interface, CFUUIDGetUUIDBytes(kIOFireWireDeviceInterfaceID),
        (void**) &iface);
    IODestroyPlugInInterface(plugin_interface);
    if (!iface)
      continue;

    result = (*iface)->Open(iface);
    if (result != KERN_SUCCESS)
    {
      continue;
    }
    return new FirewireDeviceMacOSX(iface);
  }
}
