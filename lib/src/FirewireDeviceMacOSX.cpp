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
#include "CommunicationMacOSX.h"

using namespace LibPhantom;

FirewireDeviceMacOSX::FirewireDeviceMacOSX(IOFireWireLibDeviceRef interface) :
  interface(interface)
{
	com=createCommunication();

	//TODO: should this be in the Communication class?
	(*interface)->AddIsochCallbackDispatcherToRunLoop(interface,CFRunLoopGetCurrent());
}


FirewireDeviceMacOSX::~FirewireDeviceMacOSX()
{
	delete com;
	(*interface)->Close(interface);

}


Communication * FirewireDeviceMacOSX::createCommunication() {
	return new CommunicationMacOSX(interface);
}

IOFireWireLibDeviceRef FirewireDeviceMacOSX::getInterface()
{
  return interface;
}

//Hack
long claimed=0;

unsigned int FirewireDeviceMacOSX::getFreeChannel() {
	int ch;
	for(ch=0;ch<32;ch++) {
		if (claimed & (1<<ch)) continue;
		return ch;

	}
	throw "No free isochronous channels available";
	throw "Not implemented getfreechannel";
}

void FirewireDeviceMacOSX::claimChannel(unsigned int channel) {
	claimed|=(1<<channel);

	//	throw "Not implemented claimchannel";
}
void FirewireDeviceMacOSX::releaseChannel(unsigned int channel) {
	claimed&=~(1<<channel);
	//	throw "Not implemented releasechannel";
}

