/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   ChainOfResponsibility.hpp
* @details \b Synopsis: \n Stores all the command objects so that the processing objects can retreive them.
* Processing objects check one or more queues of a particular LogicSignature that matches signatures that they
* are capable of handling and extract those command objects from the queue in order to process them.
* @see https://en.wikipedia.org/wiki/Chain-of-responsibility_pattern
* @ingroup Applications
*******************************************************************************/
#ifndef __CHAIN_OF_RESPONSIBILITY_HPP__
#define __CHAIN_OF_RESPONSIBILITY_HPP__

//AbstractionLayer
#include "Error.hpp"
#include "Global.hpp"
#include "CommandObject.hpp"
//C++
#include <memory>
#include <map>
#include <vector>

/**
 * @namespace ChainOfResponsibilityTypes
 * @brief Chain of Responsibility types
 */
namespace ChainOfResponsibilityTypes {
    /**
     * @struct Status
     * @brief The status of the Chain of Responsibility
     */
    struct Status {
        Count commandObjectCount; ///< The number of command objects currently queue accross all queues.
    };
};

/**
 * @class ChainOfResponsibility
 * @brief Stores all the command objects so that the processing objects can retreive them
 * @details This implementation of the Chain of Responsibility is a little different than the tradional approach.
 * Instead of having every single processing object subscribing and looking at each command object and using
 * the logic signature to determine if it can process the command object or if it should pass it on, this
 * implementation places in each command object in a queue that is accessed only by the processing objects
 * that explictiely want to process a particular command. This allows threads to know if there are any objects to
 * process and allows them to sleep and let other threads execute if there are no command objects to process.
*/
class ChainOfResponsibility : public Global<ChainOfResponsibility> {
    
    public:
    /// @brief Default constructor
    ChainOfResponsibility();
    /// @brief Default destructor
    virtual ~ChainOfResponsibility() = default;

    /**
     * @brief Adds a command object to the chain of responsibility
     * @param[in] commandObject The command object to add
     * @sa CommandObject
     * @returns ErrorType::Success if the command object could be added.
     * @returns ErrorType::LimitReach if the maximum amount of command objects allowed in the queue is in the queue.
     * @returns ErrorType::Timeout if access to the queue could not be secured.
     * @post Ownership of commandObject is relinquished to the ChainOfResponsibility if, and only if, ErrorType::Success is returned.
    */
    ErrorType addCommandObject(std::unique_ptr<CommandObject> &commandObject);
    /**
     * @brief Get the next command object in the queue of command objects whose logic signature matches the signature given.
     * @param[in] signature The logic signature of the command object to get
     * @param[out] error The error that occurred if the command object was not found
     * @returns The next command object in the queue of command objects whose logic signature matches the signature given
     * @post Error returned may be ErrorType::LimitReached if there is no room for the command object.
     * @post Error returned may be ErrorType::Timeout if access to the queue could not be secured.
    */
    std::unique_ptr<CommandObject> getNextCommand(LogicSignature signature, ErrorType &error);

    private:
    /// @brief Tag for logging
    static constexpr char TAG[] = "ChainOfResponsibility";
    /// @brief The maximum amount of command objects that can be in a queue at the same time
    static constexpr Count MaxCommandObjectSize = 8;
    /// @brief The maximum amount of time to wait for a semaphore
    static constexpr Milliseconds SemaphoreTimeout = 0;
    /// @brief The name of the semaphore
    std::string binarySemaphore;
    /// @brief The status of the Chain of Responsibility
    ChainOfResponsibilityTypes::Status _status;

    /// @brief The queues of command objects
    std::map<LogicSignature, std::vector<std::unique_ptr<CommandObject>>> _commandObjects;

    /**
     * @brief Checks if a command object is waiting in the queue
     * @param[in] signature The logic signature of the command object to check
     * @returns True if the command object is waiting in the queue
     */
    bool isCommandWaiting(LogicSignature signature);

    /**
     * @brief Get a constant reference to the status.
     * @post Updates the status before it's returned.
     */
    const ChainOfResponsibilityTypes::Status &statusConst() {
        Count numberOfCommandObjects = 0;

        for (const auto &commandObjectQueue : _commandObjects) {
            numberOfCommandObjects += commandObjectQueue.second.size();
        }

        _status.commandObjectCount = numberOfCommandObjects;

        return _status;
    }
};

#endif // __CHAIN_OF_RESPONSIBILITY_HPP__
