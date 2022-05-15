/*******************************************************************************
  MPLAB Harmony Application Source File
  NVM FAT Single Disk Demo Application
  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.
    NVM FAT Single Disk Demo
  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
 * Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
 *
 * Subject to your compliance with these terms, you may use Microchip software
 * and any derivatives exclusively with Microchip products. It is your
 * responsibility to comply with third party license terms applicable to your
 * use of third party software (including open source software) that may
 * accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
 * ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app.h"

#include "configuration.h"
#include "nvm_disk_images.h"
#include "system/fs/sys_fs.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// *****************************************************************************
// private definitions and types

#define DEMONSTRATE_BUG

#define APP_MOUNT_NAME "/mnt/myDrive1"
#define APP_DEVICE_NAME "/dev/nvma1"
#define APP_FS_TYPE FAT

#define APP_FILE_NAME "FILE.TXT"

#define WRITE_DATA_SIZE 13
#define ORIG_DATA_SIZE 4

#define BUFFER_SIZE (64U)

typedef enum {
  APP_STATE_MOUNT_DISK,
  APP_STATE_OPEN_FILE_FOR_WRITE,
  APP_STATE_WRITE_FILE,
  APP_STATE_FILE_STAT,
  APP_STATE_SUCCESS,
  APP_STATE_ERROR,
} APP_STATES;

typedef struct {
  APP_STATES state;
  SYS_FS_HANDLE fileHandle;
  uint32_t mount_attempts;
} APP_DATA;

// *****************************************************************************
// private (static, local) data

const uint8_t writeData[WRITE_DATA_SIZE] = "Hello World";

static APP_DATA appData;

// *****************************************************************************
// public code

void APP_Initialize(void) {
  printf("\n# =========="
         "\n# nvm_fat for sam_e54_xpro"
         "\n# ==========");
  appData.mount_attempts = 0;
  appData.state = APP_STATE_MOUNT_DISK;
}

void APP_Tasks(void) {
  switch (appData.state) {
  case APP_STATE_MOUNT_DISK: {
    appData.mount_attempts += 1;
    if (SYS_FS_Mount(APP_DEVICE_NAME, APP_MOUNT_NAME, APP_FS_TYPE, 0, NULL) !=
        0) {
      // could not mount disk.  remain in this state to retry
      asm("nop");
    } else {
      printf("\nSYS_FS_Mount succeeded after %ld attempts -- opening file for "
             "write",
             appData.mount_attempts);
      appData.state = APP_STATE_OPEN_FILE_FOR_WRITE;
    }
    break;
  }

  case APP_STATE_OPEN_FILE_FOR_WRITE: {
    appData.fileHandle = SYS_FS_FileOpen(APP_MOUNT_NAME "/" APP_FILE_NAME,
                                         SYS_FS_FILE_OPEN_WRITE);
    if (appData.fileHandle == SYS_FS_HANDLE_INVALID) {
      printf("\nSYS_FS_FileOpen failed -- quitting");
      appData.state = APP_STATE_ERROR;
    } else {
      printf("\nSYS_FS_FileOpen succeeded -- writing to file");
      appData.state = APP_STATE_WRITE_FILE;
    }
    break;
  }

  case APP_STATE_WRITE_FILE: {
    if (SYS_FS_FileWrite(appData.fileHandle,
                         (const void *)writeData,
                         WRITE_DATA_SIZE) == -1) {
      printf("\nSYS_FS_FileWrite failed -- quitting");
      SYS_FS_FileClose(appData.fileHandle);
      appData.state = APP_STATE_ERROR;
    } else {
      printf("\nSYS_FS_FileWrite succeeded -- closing file and reading status");
      SYS_FS_FileClose(appData.fileHandle);
      appData.state = APP_STATE_FILE_STAT;
    }
    break;
  }

  case APP_STATE_FILE_STAT: {
    SYS_FS_FSTAT fileStatus;
#ifdef DEMONSTRATE_BUG
    // This line simulates the uninitialized state of fileStatus when it is
    // allocated on the stack.
    fileStatus.lfname = (char *)1234;
#else
    // This line clears fileStatus.
    memset(&fileStatus, 0, sizeof(fileStatus));
#endif
    if (SYS_FS_FileStat(APP_MOUNT_NAME "/" APP_FILE_NAME,
                        &fileStatus) == SYS_FS_RES_FAILURE) {
      printf("\nSYS_FS_FileStat failed -- quitting");
      appData.state = APP_STATE_ERROR;
    } else {
      printf("\nSYS_FS_FileStat succeeded -- file contains %ld bytes"
             "\nDone!\n",
             fileStatus.fsize);
      appData.state = APP_STATE_SUCCESS;
    }
    break;
  }

  case APP_STATE_SUCCESS: {
    asm("nop");
    break;
  }

  case APP_STATE_ERROR: {
    asm("nop");
    break;
  }

  } // switch
  }
