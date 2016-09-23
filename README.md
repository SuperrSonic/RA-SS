# RA-SS; A simplified version of RetroArch Wii for use with forwarder channels, features:
- Screen Resolution saving.
- Proper 240p control for handheld games.
- Screen width changing for accurate PARs.
- In Interlaced mode you can enable/disable Deflicker.
- Removed options that never worked with RA Wii.
- Renamed settings for convenience.
- Single game loading via arguments only, no game browsing screen, no core selection screen and no game history screen.
- Added Normal2x as CPU filter.
- You can change text and highlighted text color by editing text_color and hover_color. 



*Default values are gray(54965) and white(32767) respectively, you may write them in hex, they will be converted to decimal on exit.
To convert RGB8 to the required ARGB5 you must first convert to 15-bit values and then follow this formula:

R * 1024 + G * 32 + B + 32768

Example converting red; in 24-bit is 255,0,0(0xFF0000), converted into 15-bit values it's 31,0,0

31 * 1024 + 0 * 32 + 0 + 32768 = 64512(0xFC00)

https://cloud.githubusercontent.com/assets/6880539/15104810/2dba7020-158a-11e6-883c-92b0f3119156.jpg