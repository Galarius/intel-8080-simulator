#include "modules.hpp"

#include <stdexcept>

auto modules::instance() -> modules& {
    static modules _modules {};
    return _modules;
}

modules::modules() :
    m_objects {}
{ }

void modules::shutdown() {
    instance().m_objects.clear();
}

void modules::addObject(const std::string& name, std::shared_ptr<void> obj) {
    if (auto it = m_objects.find(name); it == m_objects.cend()) {
        m_objects[name] = obj;
    } else {
        throw std::runtime_error("modules::add(): " + name + " module already exists");
    }
}

std::shared_ptr<void> modules::getObject(const std::string& name) {
    if (auto it = m_objects.find(name); it != m_objects.end()) {
        return it->second;
    }
    return nullptr;
}
