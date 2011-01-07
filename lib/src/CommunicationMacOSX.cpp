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
 * Phantom Library: FireWire communication driver for Mac OS X
 */

#include "CommunicationMacOSX.h"
#include <iostream>

#import <mach/mach.h>
using namespace std;
using namespace LibPhantom;

CommunicationMacOSX::CommunicationMacOSX(IOFireWireLibDeviceRef interface) :
  interface(interface)
{
	//Allocate buffer
	::vm_allocate( mach_task_self(), & buffer, 64*2000, true );


	dclPool=(*interface)->CreateDCLCommandPool(interface,0x2000, CFUUIDGetUUIDBytes( kIOFireWireDCLCommandPoolInterfaceID ));

	//assert(!CreateDCLCommandPool( interface, & dclPool ));


}

CommunicationMacOSX::~CommunicationMacOSX()
{

}

void CommunicationMacOSX::read(u_int64_t address, char *buffer, unsigned int length)
{
  FWAddress full_addr;

  full_addr.addressHi = address >> 32;
  full_addr.addressLo = address & 0xffffffff;

  (*interface)->Read(interface, (*interface)->GetDevice(interface), &full_addr, buffer, &length, false, 0);

}

void CommunicationMacOSX::write(u_int64_t address, char *buffer, unsigned int length)
{
  FWAddress full_addr;

  full_addr.addressHi = address >> 32;
  full_addr.addressLo = address & 0xffffffff;

  (*interface)->Write(interface, (*interface)->GetDevice(interface), &full_addr, buffer, &length, false, 0);

}

//
// callback handler
//

void
DCLCallbackHandler( DCLCommand* dcl )
{
	static int count = 0 ;

	count++;
	printf("isoch callback %u\n", count) ;

	DCLCallProcStruct *pCallProc =  (DCLCallProcStruct*) dcl;
	CommunicationMacOSX *com = (CommunicationMacOSX*) pCallProc->procData;
	com->callbackRecvHandler(&((UInt8*)com->buffer)[0], 64); //TODO: length

}
int i=0;


//
// Some handlers taken from the IOFireWireLibIsochTest example
//

IOReturn
RemotePort_GetSupported( IOFireWireLibIsochPortRef interface, IOFWSpeed* outMaxSpeed, UInt64* outChanSupported)
{
	// In this routine we return the capabilities of our remote
	// device. We don't have an actual listener out on the bus
	// so we just say we run at all speeds and on all isochronous
	// channel numbers.

	cout << i << "+RemotePort_GetSupported\n" ;

	// we support all speeds...
	// if we didn't, we could set this to kFWSpeed200MBit or another
	// appropriate value
	*outMaxSpeed		= kFWSpeedMaximum ;

	// we support all channels
	// if you had a specific channel number in mind, you could set
	// 'outChanSupported' to allow only that channel you are interested in
	// to be used
	*outChanSupported	= (UInt64)0xFFFFFFFF << 32 | (UInt64)0xFFFFFFFF ;

	// ok!
	return kIOReturnSuccess ;
}

IOReturn
RemotePort_AllocatePort( IOFireWireLibIsochPortRef interface, IOFWSpeed maxSpeed, UInt32 channel )
{
	//Todo: store/set the port number??
	printf("We've got channel %d\n",channel);
	return kIOReturnSuccess ;
}

IOReturn
RemotePort_ReleasePort( IOFireWireLibIsochPortRef interface )
{
	return kIOReturnSuccess ;
}

IOReturn
RemotePort_Start( IOFireWireLibIsochPortRef interface )
{
	cout << i << "+RemotePort_Start\n" ;
	// Talk to remote device and tell it to start listening.

	return kIOReturnSuccess ;
}

IOReturn
RemotePort_Stop( IOFireWireLibIsochPortRef interface )
{
	cout << i << "+RemotePort_Stop\n" ;
	// Talk to remote device and tell it to stop listening.

	return kIOReturnSuccess ;
}


void CommunicationMacOSX::startRecvIsoTransfer(unsigned int channel, PhantomIsoChannel *iso_channel)
{
	IOReturn	error;

//	IOFireWireLibDCLCommandPoolRef		commandPool 	= 0 ;
	IOFireWireLibRemoteIsochPortRef		remoteIsochPort	= 0 ;
	IOFireWireLibLocalIsochPortRef		localIsochPort	= 0 ;
	//IOFireWireLibIsochChannelRef		isochChannel	= 0 ;

	Communication::startRecvIsoTransfer(channel, iso_channel);


	//mChannel=(*interface)->CreateIsochChannel(interface,true,64,kFWSpeed400MBit,CFUUIDGetUUIDBytes( kIOFireWireIsochChannelInterfaceID ));
	//printf("Channel is %p\n",mChannel);

	//Write DCL program

	DCLCommand *dcl,*firstDcl;
	dcl=(*dclPool)->AllocateLabelDCL(dclPool,nil);
	firstDcl=dcl;

	int i;
	for(i=0;i<100;i++) {
	dcl=(*dclPool)->AllocateReceivePacketStartDCL(dclPool,dcl,&((UInt8*)buffer)[64*i],64);
	dcl=(*dclPool)->AllocateCallProcDCL( dclPool, dcl, & DCLCallbackHandler, (DCLCallProcDataType) this ) ;
	}
	dcl=(*dclPool)->AllocateJumpDCL(dclPool,dcl,(DCLLabel*)firstDcl);

	remoteIsochPort=(*interface)->CreateRemoteIsochPort(interface,true,CFUUIDGetUUIDBytes( kIOFireWireRemoteIsochPortInterfaceID ));
	assert(remoteIsochPort);

	IOFireWireLibRemoteIsochPortRef port=remoteIsochPort;

	(**port).SetGetSupportedHandler( port, & RemotePort_GetSupported ) ;
	(**port).SetAllocatePortHandler( port, & RemotePort_AllocatePort ) ;
	(**port).SetReleasePortHandler( port, & RemotePort_ReleasePort ) ;
	(**port).SetStartHandler( port, & RemotePort_Start ) ;
	(**port).SetStopHandler( port, & RemotePort_Stop ) ;


	localIsochPort=(*interface)->CreateLocalIsochPort(interface,false,firstDcl,0,0,0,nil,0,nil,0,CFUUIDGetUUIDBytes( kIOFireWireLocalIsochPortInterfaceID ) );
	assert(localIsochPort);

	mChannel=(*interface)->CreateIsochChannel(interface,true,64,kFWSpeed400MBit,CFUUIDGetUUIDBytes( kIOFireWireIsochChannelInterfaceID ));
	printf("Channel is %p\n",mChannel);
	assert(mChannel);

	error=(*mChannel)->AddListener(mChannel,(IOFireWireLibIsochPortRef) localIsochPort);
	assert(!error);
	error=(*mChannel)->SetTalker(mChannel,(IOFireWireLibIsochPortRef) remoteIsochPort);
	assert(!error);
	error=(*mChannel)->AllocateChannel(mChannel);
	assert(!error);
	error=(*mChannel)->Start(mChannel);




	//dclProgram = WriteTalkingDCLProgram()

}

void CommunicationMacOSX::startXmitIsoTransfer(unsigned int channel, PhantomIsoChannel *iso_channel)
{
	throw "Not implemented startXmitIsoTransfer";
}

void CommunicationMacOSX::stopIsoTransfer() {
//	throw "Not implemented stopiso";
}
void CommunicationMacOSX::doIterate(){
	SInt16	runLoopResult ;

	printf("doIterate called\n");
	while (  kCFRunLoopRunHandledSource == ( runLoopResult = CFRunLoopRunInMode( kCFRunLoopDefaultMode, 1, true ) ) )
	{
		printf("x\n");
	}
	printf("end of iterate\n");

//	throw "Not implemented doiterate";
}
