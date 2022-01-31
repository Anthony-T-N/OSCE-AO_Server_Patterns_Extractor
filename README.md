![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/Anthony-T-N/OSCE-AO_Server_Patterns_Extractor)
![C++](https://img.shields.io/badge/Language-C%2B%2B-ff69b4)
![version](https://img.shields.io/badge/version-1.0.0-yellow.svg)
![support](https://img.shields.io/badge/OS-Windows-orange.svg)

# OSCE-AO_Server_Patterns_Extractor

Overview
-
OSCE-AO_Server_Patterns_Extractor is the evolution of the previously created console application [Smart_Scan_Pattern_Extractor-URL_Builder](https://github.com/Anthony-T-N/Smart_Scan_Pattern_Extractor-URL_Builder#readme).

How It Works
-
**Version 1**

Usage Overview
-
**Version 1**
-
After executing the application. Simply select an option:
```text
Select an option:
[0] Download ICRC (Smart Scan Pattern(s)) files
[1] Download VSAPI (Virus Pattern(s)) files
[2] Download TSCPTN (Unknown Pattern(s)) files
[3] Download TMWHITE (IntelliTrap Pattern(s)) files
[4] Download SSAPTN (Spyware Active - Monitoring Pattern(s)) files
[5] Download SSPDA6 (Unknown Pattern(s)) files
[6] Download TMFWPTN (Unknown Pattern(s)) files
[7] Download TRENDXLM (Unknown Pattern(s)) files
[8] Download ENGINE (Unknown Pattern(s)) files
[9] Download PLMComponentList (Unknown Pattern(s)) files
[10] Download ALL pattern files
[11] OSCE_AO_file_integrity_check
[exit] Exit
Selection ?:
>
```
After selecting an option, the console application will perform the necessary operations to create URLs and download the individual files with its associated signature files.
```text
[!] Opening server.ini for reading;

[!] Downloading the following:
http://osce14-p.activeupdate.trendmicro.com/activeupdate/pattern/vsapi349.zip
[!] Downloading to:
2022-01-31\pattern\vsapi349.zip

[!] Downloading the following:
http://osce14-p.activeupdate.trendmicro.com/activeupdate/pattern/vsapi349.sig
[!] Downloading to:
2022-01-31\pattern\vsapi349.sig

[>                                                                                                   ] 0 %
```
