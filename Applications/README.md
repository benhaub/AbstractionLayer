# Abstraction Layer Applications
Applications are software components of the AbstractionLayer.

## Using the applications
Do not abuse Event-driven architecture components like the `ChainOfResponsibility` or `SignalsAndSlots`. They are not a catch all, asynchronous solution for all implementations and can be difficult to debug if over-done.
https://youtu.be/STKCRSUsyP0?t=795

## Applications Vs. Modules
Applications are platform independent implementations. They further extend a module or combine modules to provide further more enhanced functionality. They may also
sometimes implement an Abstraction but does not do so in a way that limits it to a certain platform (i.e. implementing an RTC for an external chip, not the processor internal RTC).

Modules implement Abstractions specifically for a certain type of platform.