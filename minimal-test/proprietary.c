/**
 * Minimal application which uses the proprietary driver/library
 * Acts as an example to trace the firewire communications
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <HD/hd.h>
#include <HDU/hduError.h>
#include <HDU/hduVector.h>

// When enabled the proprietary.log does not match the opensource.log anymore...
//#define ADD_FORCEFEEDBACK
//#define ADD_CALIBRATION
//#define ADD_MOTORTEMP

void PrintDevicePosition();
#ifdef ADD_FORCEFEEDBACK
HDCallbackCode ForceFeedbackCallback(void *pUserData);
#endif
HDSchedulerHandle gCallbackHandle = HD_INVALID_HANDLE;
#ifdef ADD_CALIBRATION
HDenum GetCalibrationStatus();
HDboolean CheckCalibration(HDenum calibrationStyle);
#endif

static int count = 4;
static int enableForceFeedback = 0;

int main(int argc, char* argv[])
{
    HHD hHD;
    HDErrorInfo error;
#ifdef ADD_CALIBRATION
    int supportedCalibrationStyles;
    int calibrationStyle;
#endif

    hHD = hdInitDevice(HD_DEFAULT_DEVICE);
    if (HD_DEVICE_ERROR(error = hdGetError()))
    {
        hduPrintError(stderr, &error, "Failed to initialize haptic device");
        fprintf(stderr, "\nPress any key to quit.\n");
        return -1;
    }

    printf("Found %s.\n\n", hdGetString(HD_DEVICE_MODEL_TYPE));

#ifdef ADD_CALIBRATION
    hdGetIntegerv(HD_CALIBRATION_STYLE, &supportedCalibrationStyles);
    if (supportedCalibrationStyles & HD_CALIBRATION_ENCODER_RESET)
    {
        calibrationStyle = HD_CALIBRATION_ENCODER_RESET;
        printf("Calibration style: HD_CALIBRATION_ENCODER_RESET\n");
    }
    if (supportedCalibrationStyles & HD_CALIBRATION_INKWELL)
    {
        calibrationStyle = HD_CALIBRATION_INKWELL;
        printf("Calibration style: HD_CALIBRATION_INKWELL\n");
    }
    if (supportedCalibrationStyles & HD_CALIBRATION_AUTO)
    {
        calibrationStyle = HD_CALIBRATION_AUTO;
        printf("Calibration style: HD_CALIBRATION_AUTO\n");
    }

    /* Some haptic devices only support manual encoder calibration via a
       hardware reset. This requires that the endpoint be placed at a known
       physical location when the reset is commanded. For the PHANTOM haptic
       devices, this means positioning the device so that all links are
       orthogonal. Also, this reset is typically performed before the servoloop
       is running, and only technically needs to be performed once after each
       time the device is plugged in. */
    if (calibrationStyle == HD_CALIBRATION_ENCODER_RESET)
    {
        printf("Please prepare for manual calibration by\n");
        printf("placing the device at its reset position.\n\n");
        printf("Press any key to continue...\n");

        getch();

        hdUpdateCalibration(calibrationStyle);
        if (hdCheckCalibration() == HD_CALIBRATION_OK)
        {
            printf("Calibration complete.\n\n");
        }
        if (HD_DEVICE_ERROR(error = hdGetError()))
        {
            hduPrintError(stderr, &error, "Reset encoders reset failed.");
            return -1;
        }
    }
#endif

#ifdef ADD_FORCEFEEDBACK
    gCallbackHandle = hdScheduleAsynchronous(ForceFeedbackCallback, 0, HD_MAX_SCHEDULER_PRIORITY);
#endif
    hdStartScheduler();
    if (HD_DEVICE_ERROR(error = hdGetError()))
    {
        hduPrintError(stderr, &error, "Failed to start the scheduler");
        return -1;
    }

#ifdef ADD_CALIBRATION
    /* Some haptic devices are calibrated when the gimbal is placed into
       the device inkwell and updateCalibration is called.  This form of
       calibration is always performed after the servoloop has started
       running. */
    if (calibrationStyle  == HD_CALIBRATION_INKWELL)
    {
        if (GetCalibrationStatus() == HD_CALIBRATION_NEEDS_MANUAL_INPUT)
        {
            printf("Please place the device into the inkwell ");
            printf("for calibration.\n\n");
        }
    }
#endif

    while(count > 0)
    {
      PrintDevicePosition();
#ifdef ADD_CALIBRATION
      printf("\n\nstart hgUpdateCalibration()\n");
      CheckCalibration(calibrationStyle);
      printf("done hgUpdateCalibration()\n\n\n");
      count--;
#endif
#ifndef ADD_FORCEFEEDBACK
#ifndef ADD_CALIBRATION
      count--;
#endif
#endif
    }
    PrintDevicePosition();

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

    printf("Device position: %.3f %.3f %.3f\n\n\n",
        position[0], position[1], position[2]);

#ifdef ADD_MOTORTEMP
    HDint numMotors;
    HDdouble *temps;
    int i;
    // TODO Nothing seems to happen when the temperatures are read... (missing TRACEs in libraw1394??)
    printf("\n\nstart reading motor temp\n");
    hdGetIntegerv(HD_OUTPUT_DOF, &numMotors);
    temps = (HDdouble *) malloc(sizeof(HDdouble) * numMotors);
    hdGetDoublev(HD_MOTOR_TEMPERATURE, temps);
    printf("\n\nfinished reading motor temp\n");

    for(i = 0; i < numMotors; i++)
      printf("Motor %d temp: %f\n", i, temps[i]);
    free(temps);
#endif
}

#ifdef ADD_CALIBRATION
HDCallbackCode HDCALLBACK UpdateCalibrationCallback(void *pUserData)
{
    HDenum *calibrationStyle = (int *) pUserData;

    if (hdCheckCalibration() == HD_CALIBRATION_NEEDS_UPDATE)
    {
        printf("hdUpdateCalibration called...\n");
        hdUpdateCalibration(*calibrationStyle);
    }

    return HD_CALLBACK_DONE;
}

HDCallbackCode HDCALLBACK CalibrationStatusCallback(void *pUserData)
{
    HDenum *pStatus = (HDenum *) pUserData;
    printf("CalibrationStatusCallback()\n");

    hdBeginFrame(hdGetCurrentDevice());
    *pStatus = hdCheckCalibration();
    hdEndFrame(hdGetCurrentDevice());

    printf("CalibrationStatusCallback() ended\n");
    return HD_CALLBACK_DONE;
}

HDenum GetCalibrationStatus()
{
    HDenum status;
    hdScheduleSynchronous(CalibrationStatusCallback, &status,
                          HD_DEFAULT_SCHEDULER_PRIORITY);
    return status;
}

HDboolean CheckCalibration(HDenum calibrationStyle)
{
    HDenum status = GetCalibrationStatus();

    printf("CheckCalibration() start...\n");

    if (status == HD_CALIBRATION_OK)
    {
        return HD_TRUE;
    }
    else if (status == HD_CALIBRATION_NEEDS_MANUAL_INPUT)
    {
        printf("Calibration requires manual input...\n");
        return HD_FALSE;
    }
    else if (status == HD_CALIBRATION_NEEDS_UPDATE)
    {
        hdScheduleSynchronous(UpdateCalibrationCallback, &calibrationStyle,
            HD_DEFAULT_SCHEDULER_PRIORITY);

        if (HD_DEVICE_ERROR(hdGetError()))
        {
            printf("\nFailed to update calibration.\n\n");
            return HD_FALSE;
        }
        else
        {
            printf("\nCalibration updated successfully.\n\n");
            count = 0;
            return HD_TRUE;
        }
    }
    else
    {
//        assert(!"Unknown calibration status");
        return HD_FALSE;
    }
}
#endif

#ifdef ADD_FORCEFEEDBACK
HDCallbackCode ForceFeedbackCallback(void *pUserData)
{
    long int force[3];
    int i;

    hdBeginFrame(hdGetCurrentDevice());
    printf("Motors enabled: %s\n", enableForceFeedback ? "yes" : "no");
    for(i = 0; i < 3; i++)
      force[i] = (enableForceFeedback?0x1234:0);
    if(enableForceFeedback)
      hdEnable(HD_FORCE_OUTPUT);
    else
      hdDisable(HD_FORCE_OUTPUT);
    enableForceFeedback = !enableForceFeedback;
    count--;

    hdSetLongv(HD_CURRENT_MOTOR_DAC_VALUES, force);
    hdEndFrame(hdGetCurrentDevice());

    return HD_CALLBACK_CONTINUE;
}
#endif
