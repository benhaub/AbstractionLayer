# AbstractionLayer
An abstraction layer for building cross-platform, [component-based](https://en.wikipedia.org/wiki/Component-based_software_engineering) C++/CMake projects.

## How to include Abstraction Layer in your project
https://github.com/benhaub/AbstractionLayerExample

## Why use an abstraction layer
### 1. Reduce the amount of testing required for code changes
An abstraction layer allows you to build your main application code separate from common software components such as hardware peripherals, storage, networks, etc.
This allows you to swap out components without having to edit or retest your main application. Consequentially, this allows you to develop and test applications on systems such
as MacOS or Linux which have faster processing times and a more feature rich debugging environment, and then take that tested code and run it on an embedded target with your ported abstraction layer.

As an example, suppose I want to test the Hello World application:

```
//Ported code.
LoggingAbstraction *logger = new MacOSLogger();

//Main application code.
logger->log("LogTag", "Hello World", LogTpye::Info);
```

After testing on MacOS, my application works. So I bring it over to my embedded target:

```
//Ported code
LoggingAbstraction *logger = new EmbeddedLogger();

//Main application code.
logger->log("LogTag", "Hello World", LogTpye::Info);
```

The main application code hasn't changed so I know the logic is correct. It will print as long as the ported code works. If it doesn't print I can eliminate the main application code as a potential source of the bug since it's already been tested and has not changed.

### 2. Reduce dependancies on chip vendors
An abstraction layer allows easy(ier) changes away from drivers and build tools offered by your chip vendor. If you mix in vendor specific tools and drivers directly alongside your application, the application will develop a strong dependancy on the vendor as it's size and complexity increases. Some of these dependancies are:
- return types
- parameters
- function names
  
It will be difficult to port software like this to other platforms.

Never say never.

Anything can happen in a project with a long or even indefinite lifetime including having/wanting to support another processor or switch vendors which neccesitates the need for a software port. If you don't pay attention to how your application manages dependancies then even a smaller hardware change like a bluetooth chip, RTC, ADC, or network switch can surprise you with how embedded it's become in your applications correctness.

Alternatively, if you depend on an abstraction:
- the return type almost never changes.
- the parameters almost never change.
- the function name almost never changes.
  
We say almost because it can be challenging to develop an abstraction layer that can support every driver on the market, but _almost never change_ is better than _certainly will change_.
