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
 * Phantom Library: list of device addresses
 */

#pragma once

// List of addresses which can be used to configure the PHANTOM device
#define ADDR_XMIT_CHANNEL          0x1000  /* sets the xmit isochronous channel */
#define ADDR_RECV_CHANNEL          0x1001  /* sets the recv isochronous channel */
#define ADDR_CONTROL               0x1087  /* control register? */
#define ADDR_CONTROL_enable_iso    (1<<3)  /*   bit 3 enables the isochronous data transfer of the device */
                                           /*   other bits are unknown and always seems to be zero? */
