# RA-SS; A simplified version of RetroArch Wii for use mainly with channels:
- Screen resolution saving.
- Better documented settings.
- Removed options that don't apply to RA for Wii.
- Single game loading by using args, this makes the interface simpler.
- Per-game settings using arguments.
- Added Normal2x as CPU filter. // As of April 4 this has been replaced by the official one which is slightly faster.
- You can change text and highlighted text color by editing text_color and hover_color. 
- Wii Remote tilting for OutRun engine. (Cannonball)
- More customizing, fade effects, ux improvements.
- En español.


![Alt text](https://user-images.githubusercontent.com/6880539/55585031-b548ce80-56f3-11e9-89f2-236e88c60ae3.png?raw=true "TRON") ![Alt text](https://user-images.githubusercontent.com/6880539/55585036-b974ec00-56f3-11e9-964b-9b75ffa4d176.png?raw=true "El Dorado")
![Alt text](https://user-images.githubusercontent.com/6880539/55585018-b0841a80-56f3-11e9-856c-6d150ad10297.png?raw=true "Default colors") ![Alt text](https://user-images.githubusercontent.com/6880539/55585607-12914f80-56f5-11e9-83e0-0e309a51561a.png?raw=true "Original")


For editing color values:
*Default values are gray(54965) and white(32767) respectively, you may write them in hex, they will be converted to decimal on exit.
To convert RGB8 to the required ARGB5 you must first convert to 15-bit values and then follow this formula:

R * 1024 + G * 32 + B + 32768

Example converting red; in 24-bit is 255,0,0(0xFF0000), converted into 15-bit values it's 31,0,0

31 * 1024 + 0 * 32 + 0 + 32768 = 64512(0xFC00)

You can also use an image editor to select your color and save your image and convert it to RGBA3 format using a tool like brawlbox
and then in a hex editor you can find the value already converted in hex, this is faster and less error-prone as well as convenient
for calculating alpha values if you plan on using a transparent color for the interface's background; enabled with "BG Solid Color".

[**Donate**](https://www.paypal.me/die5a)