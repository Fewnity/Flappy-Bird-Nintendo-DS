# Flappy Bird DS
I recreated the original Flappy Bird game for the Nintendo DS with libnds and [NightFox's Lib](https://nightfoxandco.com/?page_id=52).
You can download and modify the project to learn how to create a 2D Nintendo DS game.
Feel free to create a pull requests to improve the game :)

## Screenshots

**On emulator**

<img src="https://github.com/Fewnity/Flappy-Bird-Nintendo-DS/blob/main/GitImages/Flap2.PNG" width="350"><img src="https://github.com/Fewnity/Flappy-Bird-Nintendo-DS/blob/main/GitImages/FlapGif.gif" width="470">

**On a real Nintendo DS**

<img src="https://github.com/Fewnity/Flappy-Bird-Nintendo-DS/blob/main/GitImages/FlapReal.jpg" width="500">

**How to run the game**

Download Flappy_Bird_DS.nds [here](https://github.com/Fewnity/Flappy-Bird-Nintendo-DS/raw/main/Flappy_Bird_DS.nds).

Run it on emulator like [melonDS](http://melonds.kuribo64.net/downloads.php).

**For a real Nintendo DS**, you will need a R4 linker, may no work on other linker like M3DS REAL. (need nitrofs support)

Put the game on a SD card and run the game.

# Setup for programming
Download an IDE software like [Visual Studio Code](https://code.visualstudio.com/).

Download [devkitPro](https://github.com/devkitPro/installer/releases).

Put all projects files in a folder and put this folder in "examples" devkitPro folder like: C:\devkitPro\examples\Flappy_Bird_DS.

### Compile the game
Open the IDE console and write make or run compile.bat

The Nintendo DS game file will be here : C:\devkitPro\examples\nds\Flappy_Bird_DS\Flappy_Bird_DS.nds

Have a good game ^^
