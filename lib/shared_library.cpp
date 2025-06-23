#include "shared_library.h"
#include "../include/base_types.h"
#include "../include/shared_class.h"
#include <iostream>
#include <typeinfo>
#include <memory>
#include <sstream>

namespace WeakSymbolExample {

    // Weak symbol implementations that will be defined in both DLL and host
    namespace Internal {
        
        // Weak function implementations
        std::string __attribute__((weak)) getSharedFunctionResult() {
            return "Shared function result from DLL";
        }
        
        void __attribute__((weak)) performSharedOperation(int value) {
            std::cout << "DLL: Performing shared operation with value: " << value << std::endl;
        }
        
    } // namespace Internal

    // Explicit template instantiations with weak symbols
    template class __attribute__((weak)) TemplatedWorker<int>;
    template class __attribute__((weak)) TemplatedWorker<std::string>;

    // Factory function implementations
    std::unique_ptr<AbstractWorker> createDLLSharedWorker(int value) {
        std::cout << "DLL: Creating SharedWorker with value " << value << std::endl;
        return std::make_unique<SharedWorker>(value, "DLL");
    }

    std::unique_ptr<IBaseObject> createDLLBaseObject(int value) {
        std::cout << "DLL: Creating BaseObject (SharedWorker) with value " << value << std::endl;
        return std::make_unique<SharedWorker>(value, "DLL-BaseObject");
    }

    std::unique_ptr<AbstractWorker> createDLLTemplatedWorkerInt(int value) {
        std::cout << "DLL: Creating TemplatedWorker<int> with value " << value << std::endl;
        return std::make_unique<TemplatedWorker<int>>(value, "DLL");
    }

    std::unique_ptr<AbstractWorker> createDLLTemplatedWorkerString(const std::string& value) {
        std::cout << "DLL: Creating TemplatedWorker<string> with value '" << value << "'" << std::endl;
        return std::make_unique<TemplatedWorker<std::string>>(value, "DLL");
    }

    // RTTI testing functions
    bool testDynamicCast(IBaseObject* obj) {
        if (!obj) return false;
        
        std::cout << "DLL: Testing dynamic_cast operations..." << std::endl;
        
        // Test casting to AbstractWorker
        AbstractWorker* worker = dynamic_cast<AbstractWorker*>(obj);
        std::cout << "  -> dynamic_cast<AbstractWorker*>: " 
                  << (worker ? "SUCCESS" : "FAILED") << std::endl;
        
        // Test casting to SharedWorker
        SharedWorker* sharedWorker = dynamic_cast<SharedWorker*>(obj);
        std::cout << "  -> dynamic_cast<SharedWorker*>: " 
                  << (sharedWorker ? "SUCCESS" : "FAILED") << std::endl;
        
        // Test casting to templated worker
        auto* templatedInt = dynamic_cast<TemplatedWorker<int>*>(obj);
        std::cout << "  -> dynamic_cast<TemplatedWorker<int>*>: " 
                  << (templatedInt ? "SUCCESS" : "FAILED") << std::endl;
        
        auto* templatedString = dynamic_cast<TemplatedWorker<std::string>*>(obj);
        std::cout << "  -> dynamic_cast<TemplatedWorker<string>*>: " 
                  << (templatedString ? "SUCCESS" : "FAILED") << std::endl;
        
        return worker != nullptr;
    }

    std::string getTypeInfo(IBaseObject* obj) {
        if (!obj) return "null";
        
        const std::type_info& ti = typeid(*obj);
        std::string result = "Type: ";
        result += ti.name();
        result += " (hash_code: 0x";
        
        // Convert hash_code to hex string
        std::stringstream ss;
        ss << std::hex << ti.hash_code();
        result += ss.str();
        result += ")";
        
        return result;
    }

    void printObjectInfo(IBaseObject* obj) {
        if (!obj) {
            std::cout << "DLL: Object is null" << std::endl;
            return;
        }
        
        std::cout << "DLL: Object Information:" << std::endl;
        std::cout << "  Type Name: " << obj->getTypeName() << std::endl;
        std::cout << "  Description: " << obj->getDescription() << std::endl;
        std::cout << "  Value: " << obj->getValue() << std::endl;
        std::cout << "  RTTI Info: " << getTypeInfo(obj) << std::endl;
        
        // Test virtual function call
        std::cout << "  Calling performAction():" << std::endl;
        obj->performAction();
    }

    void demonstrateWeakSymbolUnification() {
        std::cout << "\nDLL: Demonstrating Weak Symbol Unification" << std::endl;
        std::cout << "===========================================" << std::endl;
        
        // Call weak symbol functions
        std::cout << "DLL calling weak function: " << Internal::getSharedFunctionResult() << std::endl;
        Internal::performSharedOperation(42);
        
        // Create instances and show they use the same type
        auto worker1 = std::make_unique<SharedWorker>(100, "DLL-Local");
        auto worker2 = createDLLSharedWorker(200);
        
        // Store references to avoid typeid side effect warnings
        const auto& w1_ref = *worker1;
        const auto& w2_ref = *worker2;
        
        std::cout << "DLL: Local SharedWorker type: " << typeid(w1_ref).name() << std::endl;
        std::cout << "DLL: Factory SharedWorker type: " << typeid(w2_ref).name() << std::endl;
        std::cout << "DLL: Types match: " << (typeid(w1_ref) == typeid(w2_ref) ? "YES" : "NO") << std::endl;
        
        // Test template instances
        auto templated1 = std::make_unique<TemplatedWorker<int>>(123, "DLL-Direct");
        auto templated2 = createDLLTemplatedWorkerInt(456);
        
        // Store references to avoid typeid side effect warnings
        const auto& t1_ref = *templated1;
        const auto& t2_ref = *templated2;
        
        std::cout << "DLL: Direct TemplatedWorker<int> type: " << typeid(t1_ref).name() << std::endl;
        std::cout << "DLL: Factory TemplatedWorker<int> type: " << typeid(t2_ref).name() << std::endl;
        std::cout << "DLL: Template types match: " << (typeid(t1_ref) == typeid(t2_ref) ? "YES" : "NO") << std::endl;
    }

    // C-style interface implementations
    extern "C" {
        
        IBaseObject* create_dll_object_c(int value) {
            return new SharedWorker(value, "DLL-C-Interface");
        }
        
        void destroy_dll_object_c(IBaseObject* obj) {
            delete obj;
        }
        
        int test_dynamic_cast_c(IBaseObject* obj) {
            return testDynamicCast(obj) ? 1 : 0;
        }
        
        const char* get_type_name_c(IBaseObject* obj) {
            if (!obj) return nullptr;
            
            // Note: This returns a pointer to a static string in the type_info
            // It's safe to return as long as the type exists
            return typeid(*obj).name();
        }
        
        void print_object_info_c(IBaseObject* obj) {
            printObjectInfo(obj);
        }
        
    } // extern "C"

} // namespace WeakSymbolExample 