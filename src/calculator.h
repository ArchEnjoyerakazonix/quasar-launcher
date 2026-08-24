#pragma once

#include <QString>
#include <optional>
#include <QJSEngine>

class Calculator {
public:
    Calculator();
    ~Calculator() = default;

    // Evaluates a potential math expression. Returns result string if valid, nullopt otherwise.
    std::optional<QString> evaluate(const QString &expression);

    // Checks if the string looks like an intended calculation
    static bool looksLikeMath(const QString &text);

private:
    QJSEngine m_engine;
};
