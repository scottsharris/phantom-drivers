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
 * Phantom Library: Generic implementation of communication functionalities
 */

#include <stdlib.h>
#include "Communication.h"
#include "PhantomIsoChannel.h"

// Depending on which FW_METHOD is selected, add header file for static implementations
/*
#ifdef USE_libraw1394
#include "CommunicationLibraw1394.h"
#include "FirewireDeviceLibraw1394.h"
#endif
#ifdef USE_macosx
#include "CommunicationMacOSX.h"
#include "FirewireDeviceMacOSX.h"
#endif
*/

using namespace LibPhantom;

Communication::Communication()
{
}

Communication::~Communication()
{
}

/*
 * Code moved to platform specific constructors
 *

Communication* Communication::createInstance(FirewireDevice *firewireDevice)
{
#ifdef USE_libraw1394
  FirewireDeviceLibraw1394* device = (FirewireDeviceLibraw1394*) firewireDevice;
  return new CommunicationLibraw1394(device->getPort(), device->getNode());
#endif
#ifdef USE_macosx
  FirewireDeviceMacOSX* device = (FirewireDeviceMacOSX*) firewireDevice;
  return new CommunicationMacOSX(device->getInterface());
#endif
  throw "Unknown FW_METHOD used";
}
*/

void Communication::startRecvIsoTransfer(unsigned int channel, PhantomIsoChannel *iso_channel)
{
  this->iso_channel = iso_channel;
}

void Communication::startXmitIsoTransfer(unsigned int channel, PhantomIsoChannel *iso_channel)
{
  this->iso_channel = iso_channel;
}

void Communication::callbackRecvHandler(unsigned char *data, unsigned int len)
{
  iso_channel->receivedData(data, len);
}

void Communication::callbackXmitHandler(unsigned char *data, unsigned int *len)
{
  iso_channel->transmitData(data, len);
}
