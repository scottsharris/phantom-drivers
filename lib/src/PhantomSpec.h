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
#define ADDR_CONTROL_enable_iso    (1<<3)  /*   bit 3 enables the isochronous data transfer of the device
                                            *   other bits are unknown and always seems to be zero?
                                            */
namespace LibPhantom
{
  /**
   * Contains the values of the gimbal. For each axis the 5 most LSB are unused (too erratic)
   */
  union GimbalData
  {
    unsigned short raw[3];
    struct
    {
      unsigned short x_unused :5;
      unsigned short x :11;
      unsigned short y_unused :5;
      unsigned short y :11;
      unsigned short z_unused :5;
      unsigned short z :11;
    };
  };

  /**
   * Data structure of the data presented when the read callback handler is called
   */
  struct PhantomDataRead
  {
    short unknown0; // always 0x0000 ??
    short unknown1; // always 0x001e ??
    unsigned short encoder_x;
    unsigned short encoder_y;
    unsigned short encoder_z;
    union GimbalData gimbal;
    short unknown8;
    unsigned char unknown9a; // Also status bits?
    union
    {
      unsigned char bits;
      struct
      {
        unsigned char button1 :1; // When 0 the button is pressed
        unsigned char button2 :1; // When 0 the button is pressed
        unsigned char docked :1; // When 0 the gimbal is docked
        unsigned char bit3 :1;
        unsigned char bit4 :1;
        unsigned char bit5 :1;
        unsigned char bit6 :1;
        unsigned char bit7 :1;
      };
    } status;
    short unknown10; // always 0x1007 -> not always: also have seen 0x0007
    union GimbalData gimbal_inv; // seems to be the inverse of gimbal
    short unknown14; // always 0x5746
    short unknown15;
    u_int32_t count0; // msg nr?? since this value is increased by one every time a message is received (even when app is not running)
    short unknown18;
    short unknown19;
    short count1; // Seems to be counting, slower than count0 (half the speed?)
    short unknown21;
    // After this: all zeros (not part of data anymore?)
  };

  /**
   * Data structure of the data presented when writing to the PHANTOM device
   */
  struct PhantomDataWrite
  {
    short force_x;
    short force_y;
    short force_z;
    union
    {
      unsigned short bits;
      struct
      {
        unsigned short dl_flash :1; // Dock Light flash (when enabled with dl_fflash, the light is permanently on)
        unsigned short dl_fflash :1; // Dock Light Fast flash
        unsigned short bit2 :1;
        unsigned short motors_on :1; // When set the motors are on (and the given forces are applied)
        unsigned short bit4 :1;
        unsigned short bit5 :1;
        unsigned short bit6 :1;
        unsigned short bit7 :1;
        unsigned short bit8 :1;
        unsigned short bit9 :1;
        unsigned short bit10 :1;
        unsigned short bit11 :1;
        unsigned short bit12 :1;
        unsigned short bit13 :1;
        unsigned short bit14 :1;
        unsigned short bit15 :1;
      };
    } status;
    u_int32_t unused1; // Must be zero
    u_int32_t unused2; // Must be zero
  };
}
