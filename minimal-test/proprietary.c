/**
 * Minimal application which uses the proprietary driver/library
 * Acts as an example to trace the firewire communications
 */

#include <stdio.h>

#include <HD/hd.h>
#include <HDU/hduError.h>
#include <HDU/hduVector.h>

void PrintDevicePosition();

int main(int argc, char* argv[])
{
    HHD hHD;
    HDErrorInfo error;
    int supportedCalibrationStyles;
    int calibrationStyle;
    
    hHD = hdInitDevice(HD_DEFAULT_DEVICE);
    if (HD_DEVICE_ERROR(error = hdGetError())) 
    {
        hduPrintError(stderr, &error, "Failed to initialize haptic device");
        fprintf(stderr, "\nPress any key to quit.\n");
        return -1;
    }

    printf("Found %s.\n\n", hdGetString(HD_DEVICE_MODEL_TYPE));
    
    hdStartScheduler();
    if (HD_DEVICE_ERROR(error = hdGetError()))
    {
        hduPrintError(stderr, &error, "Failed to start the scheduler");
        return -1;           
    }

    PrintDevicePosition();
    
    hdStopScheduler();
    hdDisableDevice(hHD);

    return 0;
}

HDCallbackCode DevicePositionCallback(void *pUserData)
{
    HDdouble *pPosition = (HDdouble *) pUserData;

    hdBeginFrame(hdGetCurrentDevice());
    hdGetDoublev(HD_CURRENT_POSITION, pPosition);
    hdEndFrame(hdGetCurrentDevice());

    return HD_CALLBACK_DONE;
}

void PrintDevicePosition()
{
    hduVector3Dd position;

    hdScheduleSynchronous(DevicePositionCallback, position,
        HD_DEFAULT_SCHEDULER_PRIORITY);
        
    printf("Device position: %.3f %.3f %.3f\n", 
        position[0], position[1], position[2]);
}
