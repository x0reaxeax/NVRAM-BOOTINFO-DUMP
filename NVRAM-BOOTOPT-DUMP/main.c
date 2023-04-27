#include <Windows.h>
#include <ShlObj_core.h>
#include <winternl.h>

#pragma comment(lib, "ntdll.lib")
#pragma warning (disable : 4996)

#include <stdio.h>
#include <string.h>

#define EFI_LOAD_OPTION_BUFSIZ  512

#define EFIVAR_BOOTORDER_GUID       "{8BE4DF61-93CA-11D2-AA0D-00E098032B8C}"
#define EFIVAR_BOOTENTRY_GUID       EFIVAR_BOOTORDER_GUID

#define EFIVAR_BOOTORDER_NAME       "BootOrder"

#define SE_SYSTEM_ENVIRONMENT_PRIVILEGE (22L)

typedef WORD CHAR16;

#pragma pack(push, 1)
typedef struct {
    UINT8 Type;
    UINT8 SubType;
    UINT8 Length[2];
} EFI_DEVICE_PATH_PROTOCOL;

typedef struct {
    UINT32 Attributes;
    UINT16 FilePathListLength;
    CHAR16 Description[0];
    /// EFI_DEVICE_PATH_PROTOCOL FilePathList[];
    /// UINT8 OptionalData[];
} EFI_LOAD_OPTION;

typedef struct {
    EFI_DEVICE_PATH_PROTOCOL    Header;
    ///
    UINT32                      PartitionNumber;
    UINT64                      PartitionStart;
    UINT64                      PartitionSize;
    UINT8                       Signature[16];
    UINT8                       MBRType;
    UINT8                       SignatureType;
} HARDDRIVE_DEVICE_PATH;
#pragma pack(pop)

enum _DEVICE_PATH_PROTOCOL_TYPE {
    HardwareDevicePath = 0x01,
    ACPIDevicePath,
    MessagingDevicePath,
    MediaDevicePath,
    BBSDevicePath,
    EndOfHardwareDevicePath = 0x7F
};

enum _DEVICE_PATH_PROTOCOL_SUBTYPE {
    EndEntireDevicePath = 0xFF,
    EndInstanceDevicePath = 0x01
};

enum _DEVICE_SIGNATURE_TYPE {
    NoDiskSignature = 0x00,
    MBRDiskSignature,
    GUIDDiskSignature
};

NTSYSAPI
NTSTATUS
NTAPI
RtlAdjustPrivilege(
    ULONG    Privilege,
    BOOLEAN  Enable,
    BOOLEAN  CurrentThread,
    PBOOLEAN Enabled
);

// for hexdump
BOOL IsCanonical(CHAR c) {
    if (c >= 0x20 && c <= 0x7E) {
        return TRUE;
    }
    return FALSE;
}

int main(int argc, const char *argv[]) {
    if (!IsUserAnAdmin()) {
        fprintf(stderr, "[-] This program must be run as administrator\n");
        return EXIT_FAILURE;
    }

    DWORD nBootOptions = 16;
    if (argc >= 2) {
        nBootOptions = strtoul(argv[1], NULL, 0);

        if (0 == nBootOptions) {
            fprintf(stderr, "[-] Invalid input '%s'\n", argv[1]);
            return EXIT_FAILURE;
        }
    }

    // Obtain System Environment Privilege
    BOOLEAN bEnabled = FALSE;
    NTSTATUS status = RtlAdjustPrivilege(
        SE_SYSTEM_ENVIRONMENT_PRIVILEGE,
        TRUE,
        FALSE,
        &bEnabled
    );
    if (!NT_SUCCESS(status)) {
        fprintf(
            stderr,
            "[-] Failed to obtain SeSystemEnvironmentPrivilege\n"
        );
        return EXIT_FAILURE;
    }

    printf("[+] SeDebugPrivilege obtained\n");

    // Get list of boot options
    CONST DWORD nBootOptionsSize = sizeof(UINT16) * nBootOptions;
    UINT16 *wBootOptions = malloc(nBootOptionsSize);
    if (NULL == wBootOptions) {
        fprintf(stderr, "[-] Unable to allocate memory\n");
        return EXIT_FAILURE;
    }
    memset(wBootOptions, 0, nBootOptionsSize);

    if (!GetFirmwareEnvironmentVariableA(
        EFIVAR_BOOTORDER_NAME,
        EFIVAR_BOOTORDER_GUID,
        wBootOptions,
        nBootOptionsSize
    )) {
        fprintf(
            stderr,
            "[-] Failed to query BootOrder: %lu\n",
            GetLastError()
        );
        free(wBootOptions);
        return EXIT_FAILURE;
    }

    for (UINT16 i = 0; i < nBootOptions; i++) {
        CHAR szBootOptionName[16] = { 0 };
        sprintf(szBootOptionName, "Boot%04x", wBootOptions[i]);

        EFI_LOAD_OPTION *pEfiLoadOption = malloc(EFI_LOAD_OPTION_BUFSIZ);
        if (NULL == pEfiLoadOption) {
            fprintf(
                stderr,
                "[-] Failed to allocate memory for %s\n",
                szBootOptionName
            );
            return EXIT_FAILURE;
        }
        memset(pEfiLoadOption, 0, EFI_LOAD_OPTION_BUFSIZ);
       
        DWORD dwEfiLoadOptionSize;
        if (!(dwEfiLoadOptionSize = GetFirmwareEnvironmentVariableA(
            szBootOptionName,
            EFIVAR_BOOTENTRY_GUID,
            pEfiLoadOption,
            EFI_LOAD_OPTION_BUFSIZ
        ))) {
            fprintf(
                stderr,
                "[-] Failed to query %s: %lu\n",
                szBootOptionName,
                GetLastError()
            );
            free(pEfiLoadOption);
            return EXIT_FAILURE;
        }

        CONST UINT uiDescriptionLength = lstrlenW(pEfiLoadOption->Description);
        CONST DWORD dwFilePathListOffset = \
            sizeof(UINT32) + sizeof(UINT16) + (uiDescriptionLength * sizeof(CHAR16) + sizeof(CHAR16));

        printf("\n#################################\n");
        printf("Boot Option Name:     %s\n", szBootOptionName);
        printf("EFI_LOAD_OPTION Size: %lu\n", dwEfiLoadOptionSize);
        printf("FilePath List Length: %u\n", pEfiLoadOption->FilePathListLength);
        printf("Description Length:   %u\n", uiDescriptionLength);

        if (0 != uiDescriptionLength) {
            wprintf(L"Description:          %s\n", pEfiLoadOption->Description);
        }

        EFI_DEVICE_PATH_PROTOCOL *pEfiDevicePath = (EFI_DEVICE_PATH_PROTOCOL *) (
            (UINT_PTR) pEfiLoadOption + dwFilePathListOffset
        );

        printf("Device Path Type:     %02x\n", pEfiDevicePath->Type);
        printf("Device Path SubType:  %02x\n", pEfiDevicePath->SubType);

        HARDDRIVE_DEVICE_PATH *pHardDriveDevicePath = NULL;
        if (MediaDevicePath == pEfiDevicePath->Type && EndInstanceDevicePath == pEfiDevicePath->SubType) {
            pHardDriveDevicePath = (HARDDRIVE_DEVICE_PATH *) pEfiDevicePath;

            printf("Partition Number:     %u\n", pHardDriveDevicePath->PartitionNumber);
            printf("Partition Start:      %llu\n", pHardDriveDevicePath->PartitionStart);
            printf("Partition Size:       %llu\n", pHardDriveDevicePath->PartitionSize);

            // If the SignatureType is GPT (0x02), print the GUID signature
            if (GUIDDiskSignature == pHardDriveDevicePath->SignatureType) {
                CONST GUID *guid = (GUID *) pHardDriveDevicePath->Signature;
                printf("Partition GUID:       {%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}\n",
                       guid->Data1, guid->Data2, guid->Data3,
                       guid->Data4[0], guid->Data4[1], guid->Data4[2], guid->Data4[3],
                       guid->Data4[4], guid->Data4[5], guid->Data4[6], guid->Data4[7]);
            }
        }

        CONST DWORD dwOptionalDataOffset =
            dwFilePathListOffset + pEfiLoadOption->FilePathListLength;
        CONST DWORD dwOptionalDataSize = dwEfiLoadOptionSize - dwOptionalDataOffset;

        printf("Optional Data Size:   %lu\n", dwOptionalDataSize);
        PBYTE pOptionalData = (PBYTE) pEfiLoadOption + dwOptionalDataOffset;

        if (0 != dwOptionalDataSize) {
            printf("Optional Data Dump:\n");

            CONST DWORD bytesPerLine = 16;

            for (DWORD j = 0; j < dwOptionalDataSize; ++j) {
                if (0 == j % bytesPerLine) {
                    printf("%08x  ", j);
                }

                printf("%02x ", pOptionalData[j]);

                if (j % bytesPerLine == (bytesPerLine - 1)) {
                    printf(" | ");
                    for (DWORD k = j - bytesPerLine + 1; k <= j; ++k) {
                        printf("%c", IsCanonical(pOptionalData[k]) ? pOptionalData[k] : '.');
                    }
                    printf(" | \n");
                }
            }

            // Handle the last line
            if (dwOptionalDataSize % bytesPerLine != 0) {
                for (DWORD i = dwOptionalDataSize % bytesPerLine; i < bytesPerLine; ++i) {
                    printf("   ");
                }

                printf(" | ");
                for (DWORD k = dwOptionalDataSize - (dwOptionalDataSize % bytesPerLine); k < dwOptionalDataSize; ++k) {
                    printf("%c", IsCanonical(pOptionalData[k]) ? pOptionalData[k] : '.');
                }

                for (DWORD i = dwOptionalDataSize % bytesPerLine; i < bytesPerLine; ++i) {
                    printf(" ");
                }
                printf(" |\n");
            }
            putchar('\n');
        }
        free(pEfiLoadOption);
    }

    return EXIT_SUCCESS;
}