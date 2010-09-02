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

#pragma once

#include "base-device.h"

namespace LibPhantom
{
  class Phantom : public BaseDevice
  {
    virtual ~Phantom();

    /**
     * @return an unused phantom device, or 0 if there are no unused phantom devices found.
     * @todo Return an unused Phantom devices with the lowest serial (to be able to easily select the same one over and over)
     */
    static Phantom* findPhantom();

    /**
     * @return the device with the given serial, or 0 if that device already is in use
     */
    static Phantom* findPhantom(unsigned int serial);

  protected:
    /**
     * Do no use constructor directly, but use findPhantom() functionalities.
     */
    Phantom();

    Communication *com;

   private:
    /**
     * This list contains all active Phantom instances
     */
    static Phantom** phantoms;

    /**
     * Nubmer of active phantoms
     */
    static unsigned int number_of_phantoms;

    /**
     * Size of phantoms list
     */
    static unsigned int phantoms_size;
  };
}
