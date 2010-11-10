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
 * Phantom Library: FireWire communication driver for Mac OS X
 */

#include <string.h>

#include "FirewireDeviceMacOSX.h"

using namespace LibPhantom;

FirewireDeviceMacOSX::FirewireDeviceMacOSX(IOFireWireLibDeviceRef interface) :
  interface(interface)
{

}

FirewireDeviceMacOSX::~FirewireDeviceMacOSX()
{
  (*interface)->Close(interface);
}

void FirewireDeviceMacOSX::read(unsigned long address, char *buffer, unsigned int length)
{
  FWAddress full_addr;

  full_addr.addressHi = address >> 32;
  full_addr.addressLo = address & 0xffffffff;

  (*interface)->Read(interface, (*interface)->GetDevice(interface), &full_addr, buffer, &length, false, 0);

}

void FirewireDeviceMacOSX::write(unsigned long address, char *buffer, unsigned int length)
{

}

