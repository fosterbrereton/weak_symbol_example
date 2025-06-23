#pragma once

#include "base_types.h"
#include <iostream>
#include <sstream>

namespace WeakSymbolExample {

    // This class will be defined in both the host and the DLL
    // Using weak symbols to ensure they unify to the same type at runtime
    class API_EXPORT SharedWorker : public AbstractWorker {
    private:
        int m_value;
        std::string m_source;
        
    public:
        // Constructor that takes a value and source identifier
        SharedWorker(int value, const std::string& source) 
            : m_value(value), m_source(source) {}
        
        virtual ~SharedWorker() {}
        
        // Override virtual methods from base classes
        std::string getTypeName() const override {
            return "SharedWorker";
        }
        
        std::string getDescription() const override {
            std::stringstream ss;
            ss << "SharedWorker created from " << m_source 
               << " with value " << m_value;
            return ss.str();
        }
        
        int getValue() const override {
            return m_value;
        }
        
        void performAction() override {
            std::cout << "SharedWorker::performAction() called from " 
                      << m_source << " with value " << m_value << std::endl;
        }
        
        void doWork() override {
            std::cout << "SharedWorker::doWork() - Processing work from " 
                      << m_source << std::endl;
        }
        
        bool isReady() const override {
            return m_value > 0;
        }
        
        // Additional methods specific to SharedWorker
        void setValue(int newValue) {
            m_value = newValue;
        }
        
        const std::string& getSource() const {
            return m_source;
        }
        
        // Static method to demonstrate static dispatch
        static std::string getStaticInfo() {
            return "SharedWorker static method";
        }
    };

    // Template specialization to demonstrate weak symbol behavior with templates
    template<typename T>
    class TemplatedWorker : public AbstractWorker {
    private:
        T m_data;
        std::string m_source;
        
    public:
        TemplatedWorker(const T& data, const std::string& source)
            : m_data(data), m_source(source) {}
        
        virtual ~TemplatedWorker() {}
        
        std::string getTypeName() const override {
            return "TemplatedWorker<" + std::string(typeid(T).name()) + ">";
        }
        
        std::string getDescription() const override {
            std::stringstream ss;
            ss << "TemplatedWorker from " << m_source << " with data: " << m_data;
            return ss.str();
        }
        
        int getValue() const override {
            return static_cast<int>(reinterpret_cast<intptr_t>(&m_data));
        }
        
        void performAction() override {
            std::cout << "TemplatedWorker::performAction() from " << m_source << std::endl;
        }
        
        void doWork() override {
            std::cout << "TemplatedWorker::doWork() with data: " << m_data << std::endl;
        }
        
        const T& getData() const { return m_data; }
    };

    // Explicit instantiation declarations for common types
    // These will have weak symbol definitions in both host and DLL
    extern template class TemplatedWorker<int>;
    extern template class TemplatedWorker<std::string>;

} // namespace WeakSymbolExample 