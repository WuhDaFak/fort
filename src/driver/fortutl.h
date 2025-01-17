#ifndef FORTUTL_H
#define FORTUTL_H

#include "fortdrv.h"

#if defined(__cplusplus)
extern "C" {
#endif

FORT_API NTSTATUS fort_driver_path(
        PDRIVER_OBJECT driver, PUNICODE_STRING regPath, PUNICODE_STRING outPath);

FORT_API void fort_path_prefix_adjust(PUNICODE_STRING path);

FORT_API NTSTATUS fort_system32_path_init(PDRIVER_OBJECT driver, PUNICODE_STRING regPath);
FORT_API PUNICODE_STRING fort_system32_path();
FORT_API PUNICODE_STRING fort_system_drive_path();

FORT_API NTSTATUS fort_resolve_link(PUNICODE_STRING linkPath, PUNICODE_STRING outPath);

FORT_API NTSTATUS fort_file_read(HANDLE fileHandle, ULONG poolTag, PUCHAR *outData, DWORD *outSize);
FORT_API NTSTATUS fort_file_open(PUNICODE_STRING filePath, HANDLE *outHandle);

FORT_API USHORT fort_le_u16_read(const char *cp, int offset);
FORT_API DWORD fort_le_u32_read(const char *cp, int offset);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // FORTUTL_H
