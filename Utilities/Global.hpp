/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   Global.hpp
* @details \b Synopsis: \n Makes a class globally accessible when inherited from
* @ingroup Utilities
*******************************************************************************/
#ifndef __GLOBAL_HPP__
#define __GLOBAL_HPP__

//C++
#include <memory>
#include <cassert>

/**
* @class Global
* @brief Makes a class globally accessible when inherited from
* @details Could easily be called a Singleton as well since there is only one instance of the class
*          but it is generally used more to make a class that is needed in a lot of peices of code.
*          An example of this is the operating system, storage, logging, etc.
* @tparam T The class type to make globally accessible
* @tparam Args The argument types that need to be passed to the constructor of T
* 
* @attention Do not abuse this. Global access is not cohesive, connascent, or loosely coupled and can easily lead
*            to spaghetti code. If you're having trouble accessing certain data, consider adding parameters to your
*            constructor and member variables to your class or think about alternative places to access the data. 
*            Don't immediately think of globals as the solution. A good question to ask yourself is "Should this class
*            have a [class] member variable? For example, should the network have an OperatingSystem? Well yeah, everything
*            needs an OperatingSystem. So the OperatingSystem should be global."
* @code
*  class MyClass : public Fnd::Global<MyClass, int> {
*     public:
*        MyClass(int arg) : _arg(arg) {}
* };
* 
* //Call and create as a global
* MyClass::Init(10);
* //Call into the class
* MyClass::Instance().foo();
* //Create another class but not as a global. Call directly to the ctor and bypass the Init function
* MyClass *myClass = new MyClass(10);
* @endcode
*/ 
template <class T, typename... Args> class Global {
    public:
    /**
     * @brief Constructor. Creates your class as a global.
     * @post If the class did not already exist, then it will be created.
    */
    static void Init(Args... args) {
        if (nullptr == self().get()) {
            self() = std::make_unique<T>(args...);
            assert(nullptr != self().get());
        }
    }

    /**
     * @brief Get the instance of the class.
     * @return The instance of the class.
     * @pre You must have called Init first.
     * @sa Init
    */
    static T &Instance() {
        assert(nullptr != self().get());
        return *(self().get());
    }

    /**
     * @brief Check if the global has been initialized.
     * @sa Init
     * @returns true if the global has been initialized.
     * @returns false otherwise.
     */
    static bool IsInitialized() {
        return nullptr != self().get();
    }

    Global(Global const&) = delete;
    void operator=(Global const&) = delete;

    protected:
    Global() = default;

    private:
    //Returns a reference of the unique pointer so that ownership is not transferred.
    static std::unique_ptr<T> &self() {
        static std::unique_ptr<T> _self;
        return _self;
    }
};

#endif // __GLOBAL_HPP__
