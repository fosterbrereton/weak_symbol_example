#pragma once

#include "../include/base_types.h"
#include "../include/shared_class.h"
#include <memory>

// C++ interface for the shared library
namespace WeakSymbolExample {
    
    // Factory functions exported by the DLL
    // These functions return instances created within the DLL
    
    // Create a SharedWorker instance from within the DLL
    API_EXPORT std::unique_ptr<AbstractWorker> createDLLSharedWorker(int value);
    
    // Create a SharedWorker instance and return as base pointer
    API_EXPORT std::unique_ptr<IBaseObject> createDLLBaseObject(int value);
    
    // Create templated workers from within the DLL
    API_EXPORT std::unique_ptr<AbstractWorker> createDLLTemplatedWorkerInt(int value);
    API_EXPORT std::unique_ptr<AbstractWorker> createDLLTemplatedWorkerString(const std::string& value);
    
    // Utility functions to test RTTI across boundaries
    API_EXPORT bool testDynamicCast(IBaseObject* obj);
    API_EXPORT std::string getTypeInfo(IBaseObject* obj);
    API_EXPORT void printObjectInfo(IBaseObject* obj);
    
    // Function to demonstrate that weak symbols are unified
    API_EXPORT void demonstrateWeakSymbolUnification();
    
    // C-style interface for testing (alternative approach)
    extern "C" {
        // Create objects using C interface
        API_EXPORT IBaseObject* create_dll_object_c(int value);
        API_EXPORT void destroy_dll_object_c(IBaseObject* obj);
        
        // Test functions
        API_EXPORT int test_dynamic_cast_c(IBaseObject* obj);
        API_EXPORT const char* get_type_name_c(IBaseObject* obj);
        API_EXPORT void print_object_info_c(IBaseObject* obj);
    }
    
    // Internal DLL functions (not exported, but defined with weak symbols)
    // These will be used to demonstrate that the same functions can exist
    // in both the DLL and the host, but resolve to the same implementation
    
    namespace Internal {
        
        // Weak symbol function definitions
        // These will be defined in both the DLL and the host
        std::string __attribute__((weak)) getSharedFunctionResult();
        void __attribute__((weak)) performSharedOperation(int value);
        
    } // namespace Internal
    
} // namespace WeakSymbolExample 