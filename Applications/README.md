# Abstraction Layer Applications
Applications are peices of code that use abstractions. They are general purpose and have no outside dependancies other than the C++ standard library.

## SignalsAndSlots Vs. ChainOfResponsibility
Both of these application are examples of observer patterns, but have a few key differences.

### Transfer of data
Chain of responsibility can transfer data a lot more flexibly by being able to transfer entire classes using a single command object whereas
SignalsAndSlots transfer data through parameters used in callbacks.

### First come, first serve
The chain of responsibilty command just needs to be processed by any available processing object on a first come, first server basis. The first
processing objects that takes the command object removes it from the chain and is no longer accessible to any other processing objects.

In signals and slots, all observers process the emitted signal.