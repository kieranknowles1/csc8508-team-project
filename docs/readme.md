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
