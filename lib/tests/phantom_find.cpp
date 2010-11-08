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
  * Test application to test the Phantom::findPhantom() functionality
  */

#include <stdio.h>
#include <sys/types.h>

#include "phantom.h"

#define MAX_PHANTOMS 16

int main()
{
  using namespace LibPhantom;
  Phantom *list[MAX_PHANTOMS];
  unsigned int devices_found = 0;
  uint32_t serial = 0;

  // Claim all PHANTOMS
  Phantom *p;
  while((p = Phantom::findPhantom(0)) != 0)
  {
    printf("Found a PHANTOM: %x\n", p->readDeviceSerial());
    list[devices_found] = p;
    serial = p->readDeviceSerial();
    devices_found++;
  }
  if(devices_found == 0)
    return 0; // Nothing fun to do...

  // and release them again
  while(devices_found > 0)
  {
    devices_found--;
    delete list[devices_found];
  }

  // Find last PHANTOM again
  p = Phantom::findPhantom(serial);
  if(p == 0)
  {
    printf("Error: could not find last device again...\n");
    return 1;
  }

  // Find last PHANTOM a second time
  p = Phantom::findPhantom(serial);
  if(p != 0)
  {
    printf("Error: could open a device twice...\n");
    return 1;
  }

  printf("Tests succeeded!\n");
  return 0;
}
