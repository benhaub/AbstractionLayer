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

### Memory
`SignalsAndSlots` uses more memory in both RAM and flash than the chain of responsibility.

The `Chain of Responsibility` keeps dynamically allocated queues of command objects that processing objects need to process. A bare bones command object with just a `LogicSignature` and no other member variables or functions is 16 bytes. So the more command objects you are creating the more
you are allocating memory and the more ram you use.

`SignalsAndSlots` also keeps a list of observers and the event queue and callback. The EventQueue reference is 8 bytes (see https://stackoverflow.com/questions/26631169/why-does-sizeof-a-reference-type-give-you-the-sizeof-the-type) and the callback is 32 bytes. So if you have an equal amount of command objects and an equal amount of observers then signals and slots will use more RAM. Additionally, since `SignalsAndSlots` also uses
template functions, the more `Signals` you have the more template functions (with concrete types) there are and so you will also have a larger
code size.

### Speed
`Chain of Responsibility` will be slower than `SignalsAndSlots` since each command must be allocated dynamically. This can be mitigated by using a
a memory pool.

### Summary
- If you need multiple observers, use `SignalsAndSlots`.
- If you strictly need one, and only one, observer, use `ChainOfResponsibility`.
- If speed is the most important, use `SignalsAndSlots`.
- If it doesn't matter, use `ChainOfResponsibility`.