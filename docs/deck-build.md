# Building on a Steam Deck
*It's just a PC, right. I use Arch BTW*

1. Roughly follow this guide: (https://medium.com/@JeanMichelParis/how-to-install-a-coding-environment-on-your-steam-deck-1b4720810be9)[https://medium.com/@JeanMichelParis/how-to-install-a-coding-environment-on-your-steam-deck-1b4720810be9]. I can't describe which steps to skip or adapt, so bring a strong coffee and just keep playing with it until
   it works
2. Copy headers around randomly, you should eventually prevent errors.
  - If you don't know where to look, do `find / -name 'header' 2>/dev/null`, I should really add this to my bashrc
3. Test controller input, since that's the only reason I went through all this.

If you think this is convoluted, try the [C.U.R.S.E.D](./readme.md) method.

## Notes
- Exiting doesn't seem to work currently, I think the game grabbing joysticks suppresses the usual keyboard input.
  Workaround: SSH into the deck and manually kill the process
