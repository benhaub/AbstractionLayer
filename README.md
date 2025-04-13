# AbstractionLayer
An abstraction layer for building cross-platform, [component-based](https://en.wikipedia.org/wiki/Component-based_software_engineering) C++/CMake projects.

## How to include Abstraction Layer in your project
https://github.com/benhaub/sm10001slidePotentiometer

## Helpful boilerplates
https://github.com/benhaub/vscode

https://github.com/benhaub/buildScript

https://github.com/benhaub/abstractionLayerTopLevelCMake

## Why use an abstraction layer
### 1. Reduce the amount of testing required for code changes
An abstraction layer allows you to build your main application code separate from common software components such as hardware peripherals, storage, networks, etc.
This allows you to swap out components without having to edit or retest your main application. Consequentially, this allows you to develop and test applications on systems such
as MacOS or Linux which have faster processing times and a more feature rich debugging environment, and then take that tested code and run it on an embedded target with your ported abstraction layer.

As an example, suppose we want to test the Hello World application:

```
//Ported code.
LoggingAbstraction *logger = new MacOSLogger();

//Main application code.
logger->log("LogTag", "Hello World", LogType::Info);
```

After testing on MacOS, the application works. So we bring it over to the embedded target:

```
//Ported code
LoggingAbstraction *logger = new EmbeddedLogger();

//Main application code.
logger->log("LogTag", "Hello World", LogType::Info);
```

The main application code hasn't changed so we know the logic is correct. It will print as long as the ported code works. If it doesn't print we can eliminate the main application code as a potential source of the bug since it's already been tested and has not changed.

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

### 3. Find bugs
- Desktop platforms offer a more feature rich and faster debugging environment. Sanitizors and leak checkers are not available for embedded platforms so gaining access to these can greatly increase the robustness of your application.
- Sometimes, what works on one platform, will break on another. This can be very beneficial because you can have hidden race conditions that are just waiting for the timing to be correct. A race condition that just works on one platform
  may immediately break on the next.
- Using different compilers may show different warnings which can help find bugs that were previously not reported.

### 4. Defer implementation details
An abstraction layer increases your ability to [defer implementation details](https://youtu.be/2dKZ-dWaCiU?t=3612)
- You could develop the code for the processor you have immediately available, get a sense of it's memory requirements, performance, and required features and port the code later to a more suitable processor. Remember from point 1 that once your application is working, porting the code will not effect it's correctness. If something goes wrong you know it's not at the application level.

## Factors that will complicate cross-platform design

Complicate does not mean prohibit. It is still possible to manage these complications but there may not be an elegant solution.

### 1. ROM code
If you build an application around some ROM code features (like a built in http server that handles requests for you) then your app may lack the required calls to work on other platforms that don't have this ROM code. This means that you will have to include calls in your application code that will run on some platforms, but not others and can complicate your code.
### 2. Memory usage
Some drivers might demand more memory (like stack) than other platform drivers so you may find that as you switch to other platforms your stack sizes need to increase.
### 3. Vendor tools
Using some vendor tools (like flashers or partition managers) might not have a portable way of operating. You may need to setup various file/directory structures for tools on one platform to use while another tool uses a different set. Some of these tools might also introduce some hardcoded names that may need to be included in your application code (like the name of a partition on storage) which could be recognized by one platform but not the other. It is in the vendors interest to create dependencies on their tools so that you are more likely to continue to buy ICs from them so take care when choosing what vendor tools you will use and consider their portability ahead of time.
### 4. Hardware dependant values
Depending on your hardware, some values like pin numbers may need to change from platform to platform. The best way to handle this is to use compile time macros to change the constants depending on the platform you are compiling for.
### 5. Generality
The goal of the AbstractionLayer is to easily support multiple devices with firmware authoured by varying vendors. In order to achieve this the abstractions only include APIs for the most common use cases. Some vendors may offer hardware IP with proprietary functionality that is not found on other devices. If you use the AbstractionLayer you will not have access to these features since it would cause too many inconsistencies between which functions are and are not available when switching from platform to platform. While a main goal is only for the AbstractionLayer to be able to compile and at least run in a limited fashion when switching to a device the application is not directly intended to use, having to return `ErrorType::NotAvailable` and `ErrorType::NotSupported` is desired to be at a minimum. It is suggested to fork the repository and add features to a module without modiyfying the abstraction if you need advanced functionality not offered by the Abstraction API by using private functions.
