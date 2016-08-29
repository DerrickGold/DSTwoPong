#DSTwoPong

A historical archive for a project I created prior to any formal programming education.


![Alt text](/screenshots/10-9-27-23-15-31.png?raw=true "Title Menu")
![Alt text](/screenshots/10-9-27-21-18-13.png?raw=true "In Game")


##Original Readmen:

DS2 Pong
By BassAceGold
http://bag.nfnet.org/

###Installation
- If you use Imenu, just copy the Imenu folder to the root of your card.
- If you use the default supercard menu, copy the _dstwoplug folder to the root of your card



DS2 Pong is a native pong plugin for the Supercard DSTwo! 
It is fully skinable(the game itself, not the menus). All skins (in their respective titled folders) should
be placed in the 


Supercard default install:
/_dstwoplug/dstwopong/ 

Imenu install:
/_Imenu/_plg/dstwopong/

folder and must have all of the following files
to properly load:

(names are fairly self explanitory)

ball.bmp
paddle1.bmp and paddle2.bmp 
numbers.bmp - each frame must be 24 x 32
background.bmp - the playing background

all graphics must be bitmap, 24 or 16 bit

ai.wav - sound effect for p2 paddle hit
player.wav -sound effect for p1 paddle hit
wall.wav - sound effect for wall hits

###Optional
bgm.wav - background music for skinning (repeats during game play)

all sound effects must be in wave format - either 11025hz, 22050hz, or 44100hz 
and 16 bits or 8 bits (stereo and mono supported)




###Change log:

v1.01
- updated plugin icon
- added Imenu file system support
- fixed screenshots not capturing the win or lose graphics at end of the game
- added Irock23 's Halo skin. Release thread here ( http://forum.supercard.sc/thread-7217-1-1.html)
- added transparency support for paddles and ball - same color for number transparency (255,0,255)


v 1.0
- Initial Release


