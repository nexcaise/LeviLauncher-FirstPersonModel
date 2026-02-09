#include <array>
#include <memory>

class Option;

class IOptions {
public:

    virtual std::array<std::unique_ptr<Option>, 777> const& getAllRegisteredOptions() = 0;
};