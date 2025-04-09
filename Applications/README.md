# Abstraction Layer Applications
Applications are applications of the AbstractionLayer. They are software components that use the AbstractionLayer.

## Using the applications
Do not abuse Event-driven architecture components like the `ChainOfResponsibility` or `SignalsAndSlots`. They are not a catch all, asynchronous solution for all implementations and can be difficult to debug if over-done.
https://youtu.be/STKCRSUsyP0?t=795

## SignalsAndSlots Vs. ChainOfResponsibility
Both of these applications are examples of observer patterns with a few key differences.

### Transfer of data
The `ChainOfResponsibility` is more flexible in the way that it can transfer data by allowing you to transfer entire classes using a single command object

In `SignalsAndSlots` the data is transferred through parameters and callbacks.

### First come, first serve
A command on the `ChainOfResponsibility` is processed on a first come, first serve basis. The first processing object that processes the command object removes it from the chain. Commands can not be processed by more than one processing object. You may rely on race conditions in order to process a command.

In `SignalsAndSlots`, all observers process the emitted signal no matter how many there are.

### Reception
`SignalsAndSlots` can be thought of as a notification while the `ChainOfResponsibility` can be thought of as a command. A command must be received but a notification can be ignored. A command sent to the `ChainOfResponsibility` will occupy memory in a queue until it is read.
A signal that is emitted with no observers will be discarded. If it is important that another module be alerted of an action and it is expected
that that module will respond to these alerts, then use the `ChainOfResponsibility`. If it is not important that anybody responds to some alert then use `SignalsAndSlots`.

### Summary
- If you need multiple observers, use `SignalsAndSlots`.
- If you strictly need one, and only one, observer to take action on an alert, use `ChainOfResponsibility`.
- If an alert must have a corresponding action, use the `ChainOfResponsibility`.
- If it does not matter whether an alert is acted upon, use `SignalsAndSlots`.
- If none of the above matter, use `ChainOfResponsibility`.
