# Abstraction Layer Applications
Applications are applications of the AbstractionLayer. They are software components that use the AbstractionLayer. They are general purpose and have no outside dependancies other than the C++ standard library so they can be used on any platform.

## SignalsAndSlots Vs. ChainOfResponsibility
Both of these applications are examples of observer patterns with a few key differences.

### Transfer of data
The `Chain of Responsibility` is more flexible in the way that is can transfer data by allowing you to transfer entire classes using a single command object

In `SignalsAndSlots` the data is transferred through parameters and callbacks.

### First come, first serve
A command on the `Chain of Responsibility` is processed on a first come, first server basis. The first processing object that processes the command object removes it from the chain. Commands can not be processed by more than one processing object. You may rely on race conditions in order to process a command.

In `SignalsAndSlots`, all observers process the emitted signal no matter how many there are.