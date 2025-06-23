#include "../include/base_types.h"
#include "../include/shared_class.h"
#include "../lib/shared_library.h"
#include <iostream>
#include <memory>
#include <sstream>

namespace WeakSymbolExample {

    // Host-side weak symbol implementations
    // These should be the same as in the DLL, and the linker will unify them
    namespace Internal {
        
        // Weak function implementations (same as DLL)
        std::string __attribute__((weak)) getSharedFunctionResult() {
            return "Shared function result from HOST";
        }
        
        void __attribute__((weak)) performSharedOperation(int value) {
            std::cout << "HOST: Performing shared operation with value: " << value << std::endl;
        }
        
    } // namespace Internal

    // Host-side explicit template instantiations with weak symbols
    template class __attribute__((weak)) TemplatedWorker<int>;
    template class __attribute__((weak)) TemplatedWorker<std::string>;

    // Host-side factory functions (for local creation)
    std::unique_ptr<AbstractWorker> createHostSharedWorker(int value) {
        std::cout << "HOST: Creating SharedWorker with value " << value << std::endl;
        return std::make_unique<SharedWorker>(value, "HOST");
    }

    std::unique_ptr<IBaseObject> createHostBaseObject(int value) {
        std::cout << "HOST: Creating BaseObject (SharedWorker) with value " << value << std::endl;
        return std::make_unique<SharedWorker>(value, "HOST-BaseObject");
    }

    std::unique_ptr<AbstractWorker> createHostTemplatedWorkerInt(int value) {
        std::cout << "HOST: Creating TemplatedWorker<int> with value " << value << std::endl;
        return std::make_unique<TemplatedWorker<int>>(value, "HOST");
    }

    std::unique_ptr<AbstractWorker> createHostTemplatedWorkerString(const std::string& value) {
        std::cout << "HOST: Creating TemplatedWorker<string> with value '" << value << "'" << std::endl;
        return std::make_unique<TemplatedWorker<std::string>>(value, "HOST");
    }

    // Host-side RTTI testing functions
    bool testHostDynamicCast(IBaseObject* obj) {
        if (!obj) return false;
        
        std::cout << "HOST: Testing dynamic_cast operations..." << std::endl;
        
        // Test casting to AbstractWorker
        AbstractWorker* worker = dynamic_cast<AbstractWorker*>(obj);
        std::cout << "  -> dynamic_cast<AbstractWorker*>: " 
                  << (worker ? "SUCCESS" : "FAILED") << std::endl;
        
        // Test casting to SharedWorker
        SharedWorker* sharedWorker = dynamic_cast<SharedWorker*>(obj);
        std::cout << "  -> dynamic_cast<SharedWorker*>: " 
                  << (sharedWorker ? "SUCCESS" : "FAILED") << std::endl;
        
        // Test casting to templated workers
        auto* templatedInt = dynamic_cast<TemplatedWorker<int>*>(obj);
        std::cout << "  -> dynamic_cast<TemplatedWorker<int>*>: " 
                  << (templatedInt ? "SUCCESS" : "FAILED") << std::endl;
        
        auto* templatedString = dynamic_cast<TemplatedWorker<std::string>*>(obj);
        std::cout << "  -> dynamic_cast<TemplatedWorker<string>*>: " 
                  << (templatedString ? "SUCCESS" : "FAILED") << std::endl;
        
        return worker != nullptr;
    }

    std::string getHostTypeInfo(IBaseObject* obj) {
        if (!obj) return "null";
        
        const std::type_info& ti = typeid(*obj);
        std::string result = "Type: ";
        result += ti.name();
        result += " (hash_code: ";
        result += std::to_string(ti.hash_code());
        result += ")";
        
        return result;
    }

    void printHostObjectInfo(IBaseObject* obj) {
        if (!obj) {
            std::cout << "HOST: Object is null" << std::endl;
            return;
        }
        
        std::cout << "HOST: Object Information:" << std::endl;
        std::cout << "  Type Name: " << obj->getTypeName() << std::endl;
        std::cout << "  Description: " << obj->getDescription() << std::endl;
        std::cout << "  Value: " << obj->getValue() << std::endl;
        std::cout << "  RTTI Info: " << getHostTypeInfo(obj) << std::endl;
        
        // Test virtual function call
        std::cout << "  Calling performAction():" << std::endl;
        obj->performAction();
    }

    void demonstrateHostWeakSymbolUnification() {
        std::cout << "\nHOST: Demonstrating Weak Symbol Unification" << std::endl;
        std::cout << "===========================================" << std::endl;
        
        // Call weak symbol functions
        std::cout << "HOST calling weak function: " << Internal::getSharedFunctionResult() << std::endl;
        Internal::performSharedOperation(99);
        
        // Create instances and show they use the same type
        auto worker1 = std::make_unique<SharedWorker>(300, "HOST-Local");
        auto worker2 = createHostSharedWorker(400);
        
        std::cout << "HOST: Local SharedWorker type: " << typeid(*worker1).name() << std::endl;
        std::cout << "HOST: Factory SharedWorker type: " << typeid(*worker2).name() << std::endl;
        std::cout << "HOST: Types match: " << (typeid(*worker1) == typeid(*worker2) ? "YES" : "NO") << std::endl;
        
        // Test template instances
        auto templated1 = std::make_unique<TemplatedWorker<int>>(789, "HOST-Direct");
        auto templated2 = createHostTemplatedWorkerInt(101112);
        
        std::cout << "HOST: Direct TemplatedWorker<int> type: " << typeid(*templated1).name() << std::endl;
        std::cout << "HOST: Factory TemplatedWorker<int> type: " << typeid(*templated2).name() << std::endl;
        std::cout << "HOST: Template types match: " << (typeid(*templated1) == typeid(*templated2) ? "YES" : "NO") << std::endl;
    }

    // Cross-boundary type verification
    void verifyTypeUnificationAcrossBoundary() {
        std::cout << "\nCross-Boundary Type Unification Test" << std::endl;
        std::cout << "=====================================" << std::endl;
        
        // Create objects from both host and DLL
        auto hostWorker = createHostSharedWorker(500);
        auto dllWorker = createDLLSharedWorker(600);
        
        // Compare their types
        std::cout << "HOST SharedWorker type: " << typeid(*hostWorker).name() << std::endl;
        std::cout << "DLL SharedWorker type: " << typeid(*dllWorker).name() << std::endl;
        std::cout << "Cross-boundary types match: " << (typeid(*hostWorker) == typeid(*dllWorker) ? "YES" : "NO") << std::endl;
        
        // Test dynamic_cast between them
        IBaseObject* hostBase = hostWorker.get();
        IBaseObject* dllBase = dllWorker.get();
        
        // Try casting DLL object using host-side cast
        SharedWorker* hostCastedDLL = dynamic_cast<SharedWorker*>(dllBase);
        SharedWorker* dllCastedHost = dynamic_cast<SharedWorker*>(hostBase);
        
        std::cout << "HOST dynamic_cast on DLL object: " << (hostCastedDLL ? "SUCCESS" : "FAILED") << std::endl;
        std::cout << "DLL dynamic_cast on HOST object: " << (dllCastedHost ? "SUCCESS" : "FAILED") << std::endl;
        
        // Test virtual function calls
        if (hostCastedDLL) {
            std::cout << "Calling method on DLL object from HOST:" << std::endl;
            hostCastedDLL->performAction();
        }
        
        if (dllCastedHost) {
            std::cout << "Calling method on HOST object from HOST:" << std::endl;
            dllCastedHost->performAction();
        }
    }

} // namespace WeakSymbolExample 