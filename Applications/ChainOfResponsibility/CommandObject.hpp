/**************************************************************************//**
* @author Ben Haubrich                                        
* @file   CommandObject.hpp
* @details \b Synopsis: \n Command object that can be processed by a processing object.
* @ingroup Applications
*******************************************************************************/
#ifndef __COMMAND_OBJECT_HPP__
#define __COMMAND_OBJECT_HPP__

//AbstractionLayer
#include "Types.hpp"
#include "Error.hpp"
//C++
#include <string>
#include <cassert>
#include <type_traits> //For making sure the LogicSignature type is compatible with the type you've set it to.

/// @typedef LogicSignature
/// @brief A unique identifier for command objects that processing objects can check for.
using LogicSignature = uint32_t;

/**
 * @class CommandObject
 * @brief Command object that can be processed by a processing object.
 * @note It's easiest to create a class with the required data and subclass this class.
*/
class CommandObject {
    public:
    /**
     * @brief Constructor.
     * @param signature The logic signature of this command object. Processing objects compare against this to see if they can process it.
     */
    CommandObject(LogicSignature signature) : _logicSignature(signature) { assert(_InvalidLogicSignature != signature); }
    /// @brief Destructor.
    virtual ~CommandObject() = default;

    /// @brief The invalid logic signature
    static constexpr LogicSignature _InvalidLogicSignature = __UINT32_MAX__;

    /// @brief Get a constant reference to the logic signature.
    const LogicSignature &logicSignatureConst() const { return _logicSignature; }
    /// @brief Get a mutable reference to the logic signature.
    LogicSignature &logicSignature() { return _logicSignature; }

    /**
     * @brief Get the next unique logic signature.
     * @returns The next unique logic signature on success.
     * @returns CommandObject::_InvalidLogicSignature on failure.
     * @note Logic signatures are best stored globally so that individual software components can easily access them.
     */
    static LogicSignature NextUniqueLogicSignature() {
        //System wide unique logic signature for chain of responsibility.
        static LogicSignature _uniqueLogicSignature = __UINT32_MAX__;

        //In case you update the logic signature type, this will catch it. You can of course change
        //these checks to support the current type required.
        static_assert(std::is_same<LogicSignature, uint32_t>::value, "LogicSignature must be uint32_t");

        _uniqueLogicSignature += 1;

        return _uniqueLogicSignature;
    }

    private:
    /// @brief The logic signature of the command object
    LogicSignature _logicSignature;
};

#endif // __COMMAND_OBJECT_HPP__
