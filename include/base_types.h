#pragma once

#include <memory>
#include <string>
#include <typeinfo>

// Symbol visibility macros
#ifdef BUILDING_DLL
    #define API_EXPORT __attribute__((visibility("default")))
#else
    #define API_EXPORT
#endif

namespace WeakSymbolExample {

    // Base interface that all our objects will inherit from
    class API_EXPORT IBaseObject {
    public:
        virtual ~IBaseObject() {}
        
        // Virtual methods that derived classes can override
        virtual std::string getTypeName() const = 0;
        virtual std::string getDescription() const = 0;
        virtual int getValue() const = 0;
        
        // Method to demonstrate virtual function calls across boundary
        virtual void performAction() = 0;
    };

    // An intermediate base class to demonstrate inheritance hierarchy
    class API_EXPORT AbstractWorker : public IBaseObject {
    public:
        virtual ~AbstractWorker() {}
        
        // Common implementation
        std::string getDescription() const override {
            return "AbstractWorker base implementation";
        }
        
        // Pure virtual method that must be implemented
        virtual void doWork() = 0;
        
        // Virtual method with default implementation
        virtual bool isReady() const {
            return true;
        }
    };

    // Forward declaration of our shared class
    class SharedWorker;
    
    // Type alias for convenience
    using BaseObjectPtr = std::unique_ptr<IBaseObject>;
    using WorkerPtr = std::unique_ptr<AbstractWorker>;

} // namespace WeakSymbolExample 