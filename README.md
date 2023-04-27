# NVRAM-BOOTINFO-DUMP
Dumps boot entries from NVRAM using `GetFirmwareEnvironmentVariable`

In need of evaluating boot options, I was looking for a code online that I could copypaste and use, but I couldn't find anything.  
So if someone needs to copypasta this, I got you mate.  

### Sample output
```
[+] SeDebugPrivilege obtained

#################################
Boot Option Name:     Boot0004
EFI_LOAD_OPTION Size: 300
FilePath List Length: 116
Description Length:   20
Description:          Windows Boot Manager
Device Path Type:     04
Device Path SubType:  01
Partition Number:     1
Partition Start:      2048
Partition Size:       204800
Partition GUID:       {552B7E47-7A07-433B-B5EB-4982F1775851}
Optional Data Size:   136
Optional Data Dump:
00000000  57 49 4e 44 4f 57 53 00 01 00 00 00 88 00 00 00  | WINDOWS......... |
00000010  78 00 00 00 42 00 43 00 44 00 4f 00 42 00 4a 00  | x...B.C.D.O.B.J. |
00000020  45 00 43 00 54 00 3d 00 7b 00 39 00 64 00 65 00  | E.C.T.=.{.9.d.e. |
00000030  61 00 38 00 36 00 32 00 63 00 2d 00 35 00 63 00  | a.8.6.2.c.-.5.c. |
00000040  64 00 64 00 2d 00 34 00 65 00 37 00 30 00 2d 00  | d.d.-.4.e.7.0.-. |
00000050  61 00 63 00 63 00 31 00 2d 00 66 00 33 00 32 00  | a.c.c.1.-.f.3.2. |
00000060  62 00 33 00 34 00 34 00 64 00 34 00 37 00 39 00  | b.3.4.4.d.4.7.9. |
00000070  35 00 7d 00 00 00 43 00 01 00 00 00 10 00 00 00  | 5.}...C......... |
00000080  04 00 00 00 7f ff 04 00                          | ........         |


#################################
Boot Option Name:     Boot0000
EFI_LOAD_OPTION Size: 136
FilePath List Length: 44
Description Length:   42
Description:          EFI VMware Virtual NVME Namespace (NSID 1)
Device Path Type:     02
Device Path SubType:  01
Optional Data Size:   0

#################################
Boot Option Name:     Boot0001
EFI_LOAD_OPTION Size: 128
FilePath List Length: 38
Description Length:   41
Description:          EFI VMware Virtual SATA CDROM Drive (1.0)
Device Path Type:     02
Device Path SubType:  01
Optional Data Size:   0

#################################
Boot Option Name:     Boot0002
EFI_LOAD_OPTION Size: 95
FilePath List Length: 65
Description Length:   11
Description:          EFI Network
Device Path Type:     02
Device Path SubType:  01
Optional Data Size:   0

#################################
Boot Option Name:     Boot0003
EFI_LOAD_OPTION Size: 134
FilePath List Length: 48
Description Length:   39
Description:          EFI Internal Shell (Unsupported option)
Device Path Type:     01
Device Path SubType:  03
Optional Data Size:   0
```
