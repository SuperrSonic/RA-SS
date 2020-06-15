# A simplified version of RetroArch Wii:
- Automatic resolution switching and saving.
- Better documented settings.
- Wii Message Board playlog feature.
- Full screen overlays are now supported.
- Support up to 8 players.
- Reworked input to allow multiple controllers on the same player.
- The interface shifts to a simpler look when single game loading.
- Per-game settings using arguments or the native interface.
- You can change text and highlighted text color by editing text_color and hover_color. 
- Wii Remote tilting for OutRun engine. (Cannonball)
- More customization, fade effects, ux improvements, etc.
- En Español.


![Alt text](https://user-images.githubusercontent.com/6880539/55585031-b548ce80-56f3-11e9-89f2-236e88c60ae3.png?raw=true "TRON") ![Alt text](https://user-images.githubusercontent.com/6880539/55585036-b974ec00-56f3-11e9-964b-9b75ffa4d176.png?raw=true "El Dorado")
![Alt text](https://user-images.githubusercontent.com/6880539/55585018-b0841a80-56f3-11e9-856c-6d150ad10297.png?raw=true "Default colors") ![Alt text](https://user-images.githubusercontent.com/6880539/55585607-12914f80-56f5-11e9-83e0-0e309a51561a.png?raw=true "Original")


**Editing color values:**

Default values are gray(54965) and white(32767) respectively, you may write them in hex, they will be converted to decimal on exit.
To convert RGB8 to the required ARGB5 you must first convert to 15-bit values and then follow this formula:

R * 1024 + G * 32 + B + 32768

Example converting red; in 24-bit is 255,0,0(0xFF0000), converted into 15-bit values it's 31,0,0

31 * 1024 + 0 * 32 + 0 + 32768 = 64512(0xFC00)

You can also use an image editor to select your color, save your image and convert it to RGB5A3 format using a tool like brawlbox
and then in a hex editor you can find the value already converted in hex, this is faster and less error-prone as well as convenient
for calculating alpha values if you plan on using a transparent color for the interface's background; enabled with the setting
*Solid Color BG*.

[Video preview and link](https://www.youtube.com/watch?v=az5fjJHof68)
