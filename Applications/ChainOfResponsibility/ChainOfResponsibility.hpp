/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   ChainOfResponsibility.hpp
* @details \b Synopsis: \n Stores all the command objects so that the processing objects can retreive them.
* Processing objects check one or more queues of a particular LogicSignature that matches signatures that they
* are capable of handling and extract those command objects from the queue in order to process them.
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
 * @class ChainOfResponsibility
 * @brief Stores all the command objects so that the processing objects can retreive them
*/
class ChainOfResponsibility : public Global<ChainOfResponsibility> {
    public:
    ChainOfResponsibility();
    virtual ~ChainOfResponsibility() = default;

    /**
     * @brief Adds a command object to the chain of responsibility
     * @param[in] commandObject The command object to add
     * @sa CommandObject
     * @returns ErrorType::Success if the command object could be added.
     * @returns ErrorType::LimitReach if the maximum amount of command objects allowed in the queue is in the queue.
     * @returns ErrorType::Timeout if access to the queue could not be secured.
     * @post Ownership of commandObject is relinquished to the ChainOfResponsibility if, and only if, the return value is ErrorType::Success.
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
    static constexpr char TAG[] = "ChainOfResponsibility";
    static constexpr Count MaxCommandObjectSize = 8;
    static constexpr Milliseconds SemaphoreTimeout = 0;
    std::string binarySemaphore;

    std::map<LogicSignature, std::vector<std::unique_ptr<CommandObject>>> _commandObjects;
    bool isCommandWaiting(LogicSignature signature);

};

#endif // __CHAIN_OF_RESPONSIBILITY_HPP__
