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
 * Phantom Library: iterator to iterate over FirewireDevices
 */

#include "DeviceIterator.h"

// Depending on which FW_METHOD is selected, add header file for static implementations
#ifdef USE_libraw1394
#include "DeviceIteratorLibraw1394.h"
#endif
#ifdef USE_macosx
#include "DeviceIteratorMacOSX.h"
#endif

using namespace LibPhantom;

DeviceIterator* DeviceIterator::createInstance()
{
#ifdef USE_libraw1394
  return new DeviceIteratorLibraw1394;
#endif
#ifdef USE_macosx
  return new DeviceIteratorMacOSX;
#endif
  throw "Unknown FW_METHOD used";
}
