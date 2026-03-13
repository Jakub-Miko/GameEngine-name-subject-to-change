#pragma once
#include <string>
#include <vector>

#include <algorithm>
#include <stdexcept>

class MultiChoice {
public:
    MultiChoice(const std::vector<std::string>& choices, const std::string& default_choice) : choices(choices), choice(0) {
        auto fnd = std::find(choices.begin(), choices.end(), default_choice);
        if (fnd != choices.end()) {
            this->choice = std::distance(choices.begin(), fnd);
        } else {
            throw std::runtime_error("Default choice not found in choices");
        }
    }

    void SetValue(const std::string& value) {
        auto fnd = std::find(choices.begin(), choices.end(), value);
        if (fnd != choices.end()) {
            this->choice = std::distance(choices.begin(), fnd);
        } else {
            throw std::runtime_error("Value not found in choices");
        }
    }

    void SetValue(int index) {
        if(index >= 0 && index < choices.size()) {
            choice = index;
        } else {
            throw std::runtime_error("Index out of range");
        }
    }

    std::string GetValue() const { return choices[choice]; }
    int GetIndex() const { return choice; }

    std::vector<std::string> GetChoices() const { return choices; }

private:
    std::vector<std::string> choices;
    int choice;
};

/**
 * Used for triggering an action by setting this property.
 */
class DynamicPropertyAction {
public:
    DynamicPropertyAction() {

    }

    void Activate() { should_activate = true; }
    bool ShouldActivate() const { return should_activate; }
    void Reset() { should_activate = false; }

private:
    bool should_activate = false;
};