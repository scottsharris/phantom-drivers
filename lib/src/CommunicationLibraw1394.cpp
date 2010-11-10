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
 * Phantom Library: FireWire communication driver for libraw1394
 */

#include "CommunicationLibraw1394.h"
#include "DeviceIteratorLibraw1394.h"

using namespace LibPhantom;

CommunicationLibraw1394::CommunicationLibraw1394()
{
}

CommunicationLibraw1394::~CommunicationLibraw1394()
{
}

DeviceIterator *CommunicationLibraw1394::getDevices()
{
  return new DeviceIteratorLibraw1394();
}
