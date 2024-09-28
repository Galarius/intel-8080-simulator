#pragma once

#include <unordered_map>
#include <typeinfo>
#include <string>
#include <memory>

class modules final {
public:
    template<typename T, typename ...Args>
    class add final {
    public:
        add(Args&&... args);
        add(const std::string& name, Args&&... args);
    };

    template<typename T>
    static std::shared_ptr<T> get(const std::string& name = "");
    
    static void shutdown();
private:
    static modules& instance();

    modules();
    modules(const modules& other) = delete;
    modules& operator=(const modules& other) = delete;

    void addObject(const std::string& name, std::shared_ptr<void> obj);
    std::shared_ptr<void> getObject(const std::string& name);

    std::unordered_map<std::string, std::shared_ptr<void>> m_objects;
};

template<typename T, typename ...Args>
modules::add<T, Args...>::add(Args&&... args) {
    add(typeid(T).name(), std::forward<Args>(args)...);
}

template<typename T, typename ...Args>
modules::add<T, Args...>::add(const std::string& name, Args&&... args) {
    modules::instance().addObject(name, std::make_shared<T>(std::forward<Args>(args)...));
}

template<typename T>
std::shared_ptr<T> modules::get(const std::string& name) {
    return std::static_pointer_cast<T>(modules::instance().getObject( name.empty() ? typeid(T).name() : name));
}
