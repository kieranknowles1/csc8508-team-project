# Documentation

## Adding Controls

To add a new control, first add an action to either `AnalogueControl` or
`DigitalControl` in [Controller.h](../NCLCoreClasses/Controller.h). Next, map
it to a button in the following classes:
- [KeyboardMouseController.cpp](../NCLCoreClasses/KeyboardMouseController.cpp)
- [PS5Controller.cpp](../PS5Core/PS5Controller.cpp)

Implementations may map digital inputs to analogue controls and vice versa, for
example, firing uses a trigger on controller while movement uses **WASD** on
keyboard.

## PS5 Controller Workaround (C.U.R.S.E.D.)
*Controller Usage Replacement Somehow Even Doable*

Since we don't have any PS5 controllers to test with, the following workaround is
used instead. Note that this is not recommended, and only documented because it works
better than it has any right to.

- Add the PS5 remote viewer as a non-steam game to steam
- Stream remote viewer to a Steam Deck
- Configure remote viewer to pass through controller inputs.

This needs no additional configuration, and routes inputs as Steam Deck -> Steam ->
PS5 SDK -> PS5 dev kit.
