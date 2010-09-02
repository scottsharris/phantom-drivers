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

 #include <stdlib.h>

#include "phantom.h"

using namespace LibPhantom;

// Create phantoms list
unsigned int Phantom::phantoms_size = 2;
Phantom** Phantom::phantoms = (Phantom **) malloc(sizeof(Phantom *) * Phantom::phantoms_size);

// No Phantoms are in use yet
unsigned int Phantom::number_of_phantoms = 0;

Phantom::Phantom() : BaseDevice()
{
  // Add this instance to the list of phantoms
  if(number_of_phantoms == phantoms_size)
  {
    // Make list bigger (not by too much since it is unlikely that there are much more devices attached to the system)
    phantoms_size += 2;
    phantoms = (Phantom **) realloc((void *) phantoms, sizeof(Phantom *) * Phantom::phantoms_size);
    // TODO Error checking!
  }
  phantoms[number_of_phantoms] = this;

  number_of_phantoms++;

  // TODO Add callback handlers (at least for bus reset)
}

Phantom::~Phantom()
{
  // TODO Remove callback handler(s)
  unsigned int i;

  number_of_phantoms--;
  // Remove this instance from list of phantoms
  for(i = 0; i < number_of_phantoms; i++)
    if(phantoms[i] == this)
      break;
  for(;i < number_of_phantoms; i++)
    phantoms[i] = phantoms[i + 1];
}

Phantom* Phantom::findPhantom()
{
  return findPhantom(0);
}

// If serial is 0, any Phantom device will suffice
Phantom* Phantom::findPhantom(unsigned int serial)
{
  // TODO Cache the Phatom devices and recreate list upon bus resets (much more efficient, assuming that it does not take too much resources to create the list...)

  unsigned int port = 0;
  while(port < Communication::getPorts())
  {
    Communication *c = Communication::createInstance();
    c->setPort(port);
    unsigned int nodes =  c->getNumberOfNodes();
    for(unsigned int node = 0; node < nodes; node++)
    {
      // Check if node is a SensAble device
      if(c->getVendorId(node) == 0x00b99)
      {
        // TODO Check if it is a PHANTOM device
        // TODO Check what PHANTOM device type it is

	// TODO Check if the device is in use (ie it is in the phantoms list)

	// TODO Check if the serial matches

	// Found an unused Phantom with correct serial
	return new Phantom;
      }
    }

    // Try next port
    port++;
  }

  // We failed to find a(n unused) Phantom device...
  return 0;
}
