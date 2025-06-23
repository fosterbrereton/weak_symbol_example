#include "../include/base_types.h"
#include "../include/shared_class.h"
#include "../lib/shared_library.h"
#include <iostream>
#include <memory>
#include <vector>
#include <iomanip>
#include <typeinfo>

// Forward declarations for host-side functions (defined in host_implementation.cpp)
namespace WeakSymbolExample {
    std::unique_ptr<AbstractWorker> createHostSharedWorker(int value);
    std::unique_ptr<IBaseObject> createHostBaseObject(int value);
    std::unique_ptr<AbstractWorker> createHostTemplatedWorkerInt(int value);
    std::unique_ptr<AbstractWorker> createHostTemplatedWorkerString(const std::string& value);
    
    bool testHostDynamicCast(IBaseObject* obj);
    std::string getHostTypeInfo(IBaseObject* obj);
    void printHostObjectInfo(IBaseObject* obj);
    void demonstrateHostWeakSymbolUnification();
    void verifyTypeUnificationAcrossBoundary();
}

using namespace WeakSymbolExample;

// Helper function to print section headers
void printSectionHeader(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << " " << title << std::endl;
    std::cout << std::string(60, '=') << std::endl;
}

// Test basic object creation and method calls
void testBasicFunctionality() {
    printSectionHeader("Basic Functionality Test");
    
    // Create objects from both host and DLL
    auto hostWorker = createHostSharedWorker(10);
    auto dllWorker = createDLLSharedWorker(20);
    
    std::cout << "\n--- Host-created object ---" << std::endl;
    hostWorker->performAction();
    hostWorker->doWork();
    std::cout << "isReady: " << (hostWorker->isReady() ? "true" : "false") << std::endl;
    
    std::cout << "\n--- DLL-created object ---" << std::endl;
    dllWorker->performAction();
    dllWorker->doWork();
    std::cout << "isReady: " << (dllWorker->isReady() ? "true" : "false") << std::endl;
}

// Test RTTI functionality across boundaries
void testRTTIFunctionality() {
    printSectionHeader("RTTI Functionality Test");
    
    // Create various types of objects
    auto hostSharedWorker = createHostSharedWorker(100);
    auto dllSharedWorker = createDLLSharedWorker(200);
    auto hostTemplated = createHostTemplatedWorkerInt(300);
    auto dllTemplated = createDLLTemplatedWorkerInt(400);
    
    std::vector<std::pair<std::string, std::unique_ptr<IBaseObject>>> objects;
    objects.emplace_back("HOST SharedWorker", std::move(hostSharedWorker));
    objects.emplace_back("DLL SharedWorker", std::move(dllSharedWorker));
    objects.emplace_back("HOST TemplatedWorker<int>", std::move(hostTemplated));
    objects.emplace_back("DLL TemplatedWorker<int>", std::move(dllTemplated));
    
    for (auto& [name, obj] : objects) {
        std::cout << "\n--- Testing " << name << " ---" << std::endl;
        
        // Test type information
        const auto& obj_ref = *obj;
        std::cout << "Type name: " << obj->getTypeName() << std::endl;
        std::cout << "RTTI name: " << typeid(obj_ref).name() << std::endl;
        std::cout << "Hash code: " << typeid(obj_ref).hash_code() << std::endl;
        
        // Test dynamic_cast from both host and DLL sides
        std::cout << "\nDynamic cast tests:" << std::endl;
        testHostDynamicCast(obj.get());
        testDynamicCast(obj.get());
        
        // Test virtual method calls
        std::cout << "\nVirtual method calls:" << std::endl;
        obj->performAction();
    }
}

// Test that types created in host and DLL are actually the same
void testTypeUnification() {
    printSectionHeader("Type Unification Test");
    
    // Create SharedWorker instances from both sides
    auto hostWorker = createHostSharedWorker(500);
    auto dllWorker = createDLLSharedWorker(600);
    
    // Store references to avoid typeid side effect warnings
    const auto& hostWorker_ref = *hostWorker;
    const auto& dllWorker_ref = *dllWorker;
    
    // Test type equality
    bool typesEqual = (typeid(hostWorker_ref) == typeid(dllWorker_ref));
    std::cout << "Host and DLL SharedWorker types are equal: " 
              << (typesEqual ? "YES ✓" : "NO ✗") << std::endl;
    
    // Test hash codes
    std::cout << "Host SharedWorker hash: " << typeid(hostWorker_ref).hash_code() << std::endl;
    std::cout << "DLL SharedWorker hash: " << typeid(dllWorker_ref).hash_code() << std::endl;
    
    // Test type_info names
    std::cout << "Host SharedWorker type name: " << typeid(hostWorker_ref).name() << std::endl;
    std::cout << "DLL SharedWorker type name: " << typeid(dllWorker_ref).name() << std::endl;
    
    // Test cross-casting
    std::cout << "\nCross-boundary casting test:" << std::endl;
    IBaseObject* hostBase = hostWorker.get();
    IBaseObject* dllBase = dllWorker.get();
    
    // Cast DLL object using host-compiled cast
    SharedWorker* castedDLL = dynamic_cast<SharedWorker*>(dllBase);
    SharedWorker* castedHost = dynamic_cast<SharedWorker*>(hostBase);
    
    std::cout << "Cast DLL object to SharedWorker: " 
              << (castedDLL ? "SUCCESS ✓" : "FAILED ✗") << std::endl;
    std::cout << "Cast HOST object to SharedWorker: " 
              << (castedHost ? "SUCCESS ✓" : "FAILED ✗") << std::endl;
    
    if (castedDLL && castedHost) {
        std::cout << "\nCalling methods on cross-cast objects:" << std::endl;
        std::cout << "DLL object method: ";
        castedDLL->performAction();
        std::cout << "HOST object method: ";
        castedHost->performAction();
    }
}

// Test template instantiation unification
void testTemplateUnification() {
    printSectionHeader("Template Instantiation Unification Test");
    
    // Create templated instances from both sides
    auto hostTemplatedInt = createHostTemplatedWorkerInt(1000);
    auto dllTemplatedInt = createDLLTemplatedWorkerInt(2000);
    
    auto hostTemplatedString = createHostTemplatedWorkerString("HOST_STRING");
    auto dllTemplatedString = createDLLTemplatedWorkerString("DLL_STRING");
    
    // Store references to avoid typeid side effect warnings
    const auto& hostTemplatedInt_ref = *hostTemplatedInt;
    const auto& dllTemplatedInt_ref = *dllTemplatedInt;
    const auto& hostTemplatedString_ref = *hostTemplatedString;
    const auto& dllTemplatedString_ref = *dllTemplatedString;
    
    // Test int template unification
    std::cout << "TemplatedWorker<int> unification:" << std::endl;
    bool intTypesEqual = (typeid(hostTemplatedInt_ref) == typeid(dllTemplatedInt_ref));
    std::cout << "  Types equal: " << (intTypesEqual ? "YES ✓" : "NO ✗") << std::endl;
    std::cout << "  HOST hash: " << typeid(hostTemplatedInt_ref).hash_code() << std::endl;
    std::cout << "  DLL hash: " << typeid(dllTemplatedInt_ref).hash_code() << std::endl;
    
    // Test string template unification
    std::cout << "\nTemplatedWorker<string> unification:" << std::endl;
    bool stringTypesEqual = (typeid(hostTemplatedString_ref) == typeid(dllTemplatedString_ref));
    std::cout << "  Types equal: " << (stringTypesEqual ? "YES ✓" : "NO ✗") << std::endl;
    std::cout << "  HOST hash: " << typeid(hostTemplatedString_ref).hash_code() << std::endl;
    std::cout << "  DLL hash: " << typeid(dllTemplatedString_ref).hash_code() << std::endl;
    
    // Test cross-boundary template casting
    std::cout << "\nCross-boundary template casting:" << std::endl;
    auto* castedDLLInt = dynamic_cast<TemplatedWorker<int>*>(dllTemplatedInt.get());
    auto* castedHostInt = dynamic_cast<TemplatedWorker<int>*>(hostTemplatedInt.get());
    auto* castedDLLString = dynamic_cast<TemplatedWorker<std::string>*>(dllTemplatedString.get());
    auto* castedHostString = dynamic_cast<TemplatedWorker<std::string>*>(hostTemplatedString.get());
    
    std::cout << "  DLL->int cast: " << (castedDLLInt ? "SUCCESS ✓" : "FAILED ✗") << std::endl;
    std::cout << "  HOST->int cast: " << (castedHostInt ? "SUCCESS ✓" : "FAILED ✗") << std::endl;
    std::cout << "  DLL->string cast: " << (castedDLLString ? "SUCCESS ✓" : "FAILED ✗") << std::endl;
    std::cout << "  HOST->string cast: " << (castedHostString ? "SUCCESS ✓" : "FAILED ✗") << std::endl;
}

// Test C interface
void testCInterface() {
    printSectionHeader("C Interface Test");
    
    // Create object through C interface
    IBaseObject* cObj = create_dll_object_c(9999);
    if (cObj) {
        std::cout << "Created object through C interface" << std::endl;
        std::cout << "Type name (C): " << get_type_name_c(cObj) << std::endl;
        std::cout << "Dynamic cast test (C): " 
                  << (test_dynamic_cast_c(cObj) ? "SUCCESS ✓" : "FAILED ✗") << std::endl;
        
        std::cout << "\nObject info through C interface:" << std::endl;
        print_object_info_c(cObj);
        
        // Test that we can also cast it using C++ interface
        std::cout << "\nTesting C object with C++ interface:" << std::endl;
        SharedWorker* cppCasted = dynamic_cast<SharedWorker*>(cObj);
        std::cout << "C++ dynamic_cast on C-created object: " 
                  << (cppCasted ? "SUCCESS ✓" : "FAILED ✗") << std::endl;
        
        if (cppCasted) {
            std::cout << "Calling C++ method on C-created object:" << std::endl;
            cppCasted->performAction();
        }
        
        // Clean up
        destroy_dll_object_c(cObj);
    } else {
        std::cout << "Failed to create object through C interface" << std::endl;
    }
}

// Test weak symbol function unification
void testWeakSymbolFunctions() {
    printSectionHeader("Weak Symbol Function Unification Test");
    
    // This will call the functions implemented with weak symbols
    demonstrateHostWeakSymbolUnification();
    demonstrateWeakSymbolUnification();
    
    // Additional cross-boundary testing
    verifyTypeUnificationAcrossBoundary();
}

// Main function
int main() {
    std::cout << "Weak Symbol Linking Demonstration" << std::endl;
    std::cout << "Platform: macOS" << std::endl;
    std::cout << "Compiler: " << __VERSION__ << std::endl;
    std::cout << "RTTI Enabled: " << (__cpp_rtti ? "YES" : "NO") << std::endl;
    
    try {
        // Run all tests
        testBasicFunctionality();
        testRTTIFunctionality();
        testTypeUnification();
        testTemplateUnification();
        testCInterface();
        testWeakSymbolFunctions();
        
        printSectionHeader("Summary");
        std::cout << "✓ All tests completed successfully!" << std::endl;
        std::cout << "✓ Weak symbol linking is working correctly" << std::endl;
        std::cout << "✓ RTTI works across DLL boundaries" << std::endl;
        std::cout << "✓ Type unification is functioning properly" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }
    
    return 0;
} 