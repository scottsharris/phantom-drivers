/**
 * Minimal application which uses the proprietary driver/library
 * Acts as an example to trace the firewire communications
 */

#include <stdio.h>
#include <string.h>

#include <HD/hd.h>
#include <HDU/hduError.h>
#include <HDU/hduVector.h>

void PrintDevicePosition();
HDCallbackCode ForceFeedbackCallback(void *pUserData);
HDSchedulerHandle gCallbackHandle = HD_INVALID_HANDLE;

// When enabled the proprietary.log does not match the opensource.log anymore...
#define ADD_FORCEFEEDBACK

static int count = 4;
static int enableForceFeedback = 0;

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

#ifdef ADD_FORCEFEEDBACK
    gCallbackHandle = hdScheduleAsynchronous(ForceFeedbackCallback, 0, HD_MAX_SCHEDULER_PRIORITY);
#endif
    hdStartScheduler();
    if (HD_DEVICE_ERROR(error = hdGetError()))
    {
        hduPrintError(stderr, &error, "Failed to start the scheduler");
        return -1;           
    }

    while(count > 0)
    {
      PrintDevicePosition();
#ifndef ADD_FORCEFEEDBACK
      count = 0;
#endif
    }
    
    hdStopScheduler();
#ifdef ADD_FORCEFEEDBACK
    hdUnschedule(gCallbackHandle);
#endif
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

HDCallbackCode ForceFeedbackCallback(void *pUserData)
{
    long int force[3];
    int i;

    printf("Motors %d\n", enableForceFeedback);
    for(i = 0; i < 3; i++)
      force[i] = (enableForceFeedback?0x1234:0);
    if(enableForceFeedback)
      hdEnable(HD_FORCE_OUTPUT);
    else
      hdDisable(HD_FORCE_OUTPUT);
    enableForceFeedback = !enableForceFeedback;
    count--;

    hdBeginFrame(hdGetCurrentDevice());
    //hdSetLongv(HD_CURRENT_MOTOR_DAC_VALUES, force);
    hdEndFrame(hdGetCurrentDevice());

    return HD_CALLBACK_CONTINUE;
}

