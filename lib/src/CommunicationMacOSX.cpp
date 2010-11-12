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

#include "CommunicationMacOSX.h"

using namespace LibPhantom;

CommunicationMacOSX::CommunicationMacOSX(IOFireWireLibDeviceRef interface) :
  interface(interface)
{

}

CommunicationMacOSX::~CommunicationMacOSX()
{

}

void CommunicationMacOSX::read(u_int64_t address, char *buffer, unsigned int length)
{
  FWAddress full_addr;

  full_addr.addressHi = address >> 32;
  full_addr.addressLo = address & 0xffffffff;

  (*interface)->Read(interface, (*interface)->GetDevice(interface), &full_addr, buffer, &length, false, 0);

}

void CommunicationMacOSX::write(u_int64_t address, char *buffer, unsigned int length)
{
	//TODO: implement
	throw "Not implemented";
}

void CommunicationMacOSX::stopIsoTransfer() {
	throw "Not implemented";
}
void CommunicationMacOSX::doIterate(){
	throw "Not implemented";
}
