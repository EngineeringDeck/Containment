# Containment

Containment is a tool for gamers with games behaving badly.

Some games, especially older games and ports from other platforms to PC, like to do bad things like changing the desktop resolution or refuse to run in a window. Containment attempts to contain these games and revert the, albeit temporary, damage they do.

The tool doesn't resort to any hacks like debug hooks like dxwnd. It simply attempts to take ownership of the game window and keep track of any global settings the game changes and undo them using Windows API calls.
