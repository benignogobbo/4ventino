// +---------------------------------------------------+
// | The 4ventino firmware. Changelog                  |
// | Benigno Gobbo                                     |
// | © 2022 Istituto Nazionale di Fisica Nucleare      |
// +---------------------------------------------------+

// 2019-05-14. Version 0.1, first complete version.
// 2019-05-17. Version 0.1.1 bug fix
//             - fixed problem in eprom set: zero-end strings got from display
// 2019-05-24. Version 0.1.2 bug fix
//             - Tried to make NTP time setup more robust
//             - IP setup attempt again in case of failure (up to 5 attempts)
//             - Displays sleep 2 -> 3 minutes
//             - Few aestetic fixes
// 2019-05-28. Version 0.1.3 bug fix
//             - Still attempted to fix NTP
// 2019-06-03. Version 0.1.4
//             - Added "reset" command to reset the AVR from remote
// 2019-06-27. Version 0.1.5
//             - Removed #include <SPI.h> from main            
// 2019-07-25. Version 0.2
//             - Set the alarm function
//             - Few other monor changes
// 2019-07-30. Version 0.2.1
//             - Fix
// 2019-09-13. Version 0.2.2
//             - checkTp now uses TP1 only
// 2019-12-16. Version 0.2.3
//             - checkTP uses both TP1 and TP2
// 2019-12-17. Version 0.2.4           
//             - TP2 in dark colors on display
//             - Added underflow check on TP1 and TP2
//             - Added version retrieval via ethernet or USB
// 2020-02-12. Version 0.3
//             - Added remote EEPROM setup via Ethernet
// 2020-02-14. Version 0.4
//             - Serial to String instead of array
// 2020-02-18. Version 0.4.1
//             - Added remote EEPROM setup via Serial
// 2022-10-17. Version 0.4.2
//             - checkTP uses just TP1: alarm if TP1 > -30 °C
// 2022-11-30. Version 0.4.3
//             - "line" command asks for line id number
// 2023-02-14. version 0.4.4
//             - bug fix. (.ino L179 st[16] -> sf[16]) Thanks to H.Yin
