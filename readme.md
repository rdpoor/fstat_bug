# Demonstration of `SYS_FS_FileStat()` / `FAFTS_stat()` bug

## Issue

```

// Bug in sys_fs_fat_interface.c:FATFS_stat():290 if stat_buf has a stray
// value in stat_buf->lfname, `fileStat->lfname[0] = `\0` hard faults.
```

## Summary

Passing an uninitialized `SYS_FS_FSTAT` structure to `SYS_FS_FileStat()` will
lead to a hard fault.

## Details

The following code will reliably generate a hard fault on the SAME54:

```
    SYS_FS_FSTAT fileStatus;
    // If there are stray bytes in fileStatus, as would be the case if it is
    // allocated on the stack, then SYS_FS_FileStat() will get a hard fault.
    // In this example, we assign the value `1234` to `fileStatus.lfname` to
    // simulate its uninitialized state.
    fileStatus.lfname = (char *)1234;
    SYS_FS_FileStat(APP_MOUNT_NAME "/" APP_FILE_NAME, &fileStatus);
```

## Analysis

In the file `sys_fs_fat_interface.c`, the function `FATFS_stat()` has the
following code:

```
if ((res == FR_OK) && (fileStat->lfname != NULL))
{
    /* Use fileStat->fname instead */
    fileStat->lfname[0] = '\0';
}
```

The problem is that if fileStat->lfname contains a non-zero stray values, as
would be the case if fileStat is allocated on the stack, then the code will
attempt to de-reference the pointer and write to it.  

In the best case, the system will get a hard fault.  But a much worse problem
ensues if it does NOT get a hard fault, since an arbitrary byte of memory will
be set to '\0'.

## Demonstrating the bug

### Preprequisites:

* SAME54 Xplained PRO development board
* MPLAB.X IDE
* (Optional) Serial terminal (TeraTerm, puTTY, etc)

### Running

* Connect the SAME54 XPRO EDBG port to your computer
* Launch MPLAB.X IDE
* Open the project in this directory (sam_e54_xpro.X)
* (Optional) Open the serial terminal on the EDBG port (115200 baud, 8n1)
* Launch the program in the debugger

### Observe

The program will stope in the debugger with a hard fault before completing.

### Workaround

Prior to calling `SYS_FS_FileStat()`, make sure the `fileStatus` structure is
cleared, for example, by calling memset():

```
    SYS_FS_FSTAT fileStatus;
    memset(fileStatus, 0, sizeof(fileStatus));
    SYS_FS_FileStat(APP_MOUNT_NAME "/" APP_FILE_NAME, &fileStatus);
```

## Environment

This example was built using the following environment:

```

C:/Users/r/Projects/temp/nvm_fat/firmware/sam_e54_xpro.X
nvm_fat_sam_e54_xpro
Project Type: Application - Configuration: sam_e54_xpro
Device
ATSAME54P20A
 Checksum: Debug Image
 CRC32: 0x2BAB962B
Packs
CMSIS (5.8.0)
SAME54_DFP (3.5.87)
Compiler Toolchain
XC32 (v4.00) [C:\Program Files\Microchip\xc32\v4.00\bin]
Debug Image: ELF: Optimization: gcc O1 g++ O1
 Device support information: SAME54_DFP (3.5.87)
Linker Reserved Heap:  512 (0x200) bytes
Memory
Data 270,336 (0x42000) bytes
Data Used: 5.6%
Data Used: 15,134 (0x3B1E) Free: 255,202 (0x3E4E2)
Program 1,048,576 (0x100000) bytes
Program Used: 9.7%
Program Used: 102,227 (0x18F53) Free: 946,349 (0xE70AD)
Debug Tool
 EDBG: ATML2748071800008034
SWD (2.000 MHz)
Debug Resources
Program BP Used: 0  Free: 6
Data BP Used: 0  Free: 2
Data Capture BP: No Support
Unlimited BP (S/W): Disabled

Other Project Info
Data Reserved Memory

Program Reserved Memory


Control Points.

Java NetBeans Properties
MPLAB X IDE v6.00 - nvm_fat_sam_e54_xpro : sam_e54_xpro
                   os.name : Windows 10
                   os.arch : amd64
                os.version : 10.0
       sun.arch.data.model : 64
          sun.java.command : null
              java.version : 1.8.0_292
           java.vm.version : 25.292-b10
      java.runtime.version : 1.8.0_292-b10
java.specification.version : 1.8
             netbeans.user : C:\Users\r\AppData\Roaming\mplab_ide\dev\v6.00
             netbeans.home : C:\Program Files\Microchip\MPLABX\v6.00\mplab_platform\platform
     netbeans.projects.dir : C:\Users\r\MPLABXProjects
            java.io.tmpdir : C:\Users\r\AppData\Local\Temp\mplab_ide
                 user.home : C:\Users\r
              user.country : US
             user.language : en
             user.timezone : America/Los_Angeles
          sun.jnu.encoding : Cp1252
             file.encoding : Cp1252
            file.separator : \

Java & System Memory
         Java Total Memory =   3123 Mb
         Java Used Memory  =   1988 Mb
         Java Free Memory  =   1135 Mb
CommittedVirtualMemorySize =   4547 Mb
    FreePhysicalMemorySize =  18734 Mb
             SystemCpuLoad =      0 Mb
            ProcessCpuLoad =      0 Mb
   TotalPhysicalMemorySize =  32636 Mb

System Path
   Path = C:\Program Files\Microchip\MPLABX\v6.00\gnuBins\GnuWin32\bin;C:\Program Files\Microchip\MPLABX\v6.00\mplab_platform\bin;C:\Program Files (x86)\Common Files\Oracle\Java\javapath;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;C:\WINDOWS\System32\WindowsPowerShell\v1.0\;C:\WINDOWS\System32\OpenSSH\;C:\Program Files\Git\cmd;C:\Program Files (x86)\WinMerge;C:\Program Files (x86)\IVI Foundation\IVI\bin;C:\Program Files\IVI Foundation\IVI\bin;C:\Program Files (x86)\IVI Foundation\VISA\WinNT\Bin;C:\Program Files\IVI Foundation\VISA\Win64\Bin\;C:\Program Files (x86)\IVI Foundation\VISA\WinNT\Bin\;C:\Program Files\doxygen\bin;C:\Program Files\WinMerge;C:\Program Files\Microchip\xc32\v2.40\bin;C:\Program Files\PuTTY\;C:\Program Files\dotnet\;C:\Program Files\Graphviz\bin;C:\Program Files\Microchip\xc32\v3.01\bin;C:\Program Files\Microchip\xc8\v2.36\bin;C:\Program Files\Microchip\xc32\v4.00\bin;C:\Users\r\.cargo\bin;C:\Users\r\AppData\Local\atom\bin;C:\Program Files (x86)\mitmproxy\bin;C:\Users\r\AppData\Local\Programs\Python\Python32-32;C:\Users\r\AppData\Local\Programs\Python\Python32-32\Scripts;C:\Users\r\AppData\Local\Microsoft\WindowsApps;
ComSpec = C:\WINDOWS\system32\cmd.exe
```
