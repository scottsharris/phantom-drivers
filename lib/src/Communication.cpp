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

// Depending on which FW_METHOD is selected, add header file for static implementations
#ifdef USE_libraw1394
#include "CommunicationLibraw1394.h"
#endif
#ifdef USE_macosx
#include "CommunicationMacOSX.h"
#endif

using namespace LibPhantom;

Communication::Communication()
{
}

Communication::~Communication()
{
}

Communication* Communication::createInstance()
{
#ifdef USE_libraw1394
  return new CommunicationLibraw1394;
#endif
#ifdef USE_macosx
  return new CommunicationMacOSX;
#endif
  return NULL;
}
