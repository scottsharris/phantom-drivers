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
 * Test application to test the Communication::getRonfigRom() functionality
 */

#include <stdio.h>
#include <sys/types.h>
#include <assert.h>

#include "Communication.h"
#include "DeviceIterator.h"

#define CSR_REGISTER_BASE  0xfffff0000000ULL
#define CONFIG_ROM_ADDR    CSR_REGISTER_BASE + 0x400

int main()
{
  using namespace LibPhantom;
  try
  {
    DeviceIterator *i = DeviceIterator::createInstance();
    FirewireDevice *d;

    int test1NumDevices = 0;
    int test2NumDevices = 0;

    printf("Test 1: finding devices, closing after use\n");
    for (d = i->next(); d; d = i->next())
    {
      printf("Device -> vendor: 0x%6.6x %s\n", d->getVendorId(), d->getVendorName());
      delete d;
      test1NumDevices++;
    }
    if (test1NumDevices == 0)
    {
      printf("No device was found\n");
      return 1;
    }

    printf("Test 2: finding devices, not closing after use\n");
    i = DeviceIterator::createInstance();
    for (d = i->next(); d; d = i->next())
    {
      printf("Device -> vendor: 0x%6.6x %s\n", d->getVendorId(), d->getVendorName());
      test2NumDevices++;
    }
    if (test1NumDevices != test2NumDevices)
    {
      printf("Number of devices changed\n");
      return 1;
    }

    //Test: do deliberately *not* delete the devices, they should not be found again!

    printf("Test 3: finding devices, even though all should be open\n");
    i = DeviceIterator::createInstance();
    for (d = i->next(); d; d = i->next())
    {
      printf("A device could be opened twice\n");
      return 1;
    }
    delete i;
  }
  catch (char const* str)
  {
    printf("Exception raised: %s\n", str);
    return 1;
  }
  printf("Tests succeeded!\n");
  return 0;
}
