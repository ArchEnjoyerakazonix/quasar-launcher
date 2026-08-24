#include "calculator.h"
#include <QRegularExpression>
#include <cmath>

Calculator::Calculator()
{
    QJSValue global = m_engine.globalObject();
    QJSValue math = m_engine.evaluate(QStringLiteral("Math"));
    global.setProperty(QStringLiteral("pi"), math.property(QStringLiteral("PI")));
    global.setProperty(QStringLiteral("PI"), math.property(QStringLiteral("PI")));
    global.setProperty(QStringLiteral("e"), math.property(QStringLiteral("E")));
    global.setProperty(QStringLiteral("E"), math.property(QStringLiteral("E")));
    global.setProperty(QStringLiteral("sin"), math.property(QStringLiteral("sin")));
    global.setProperty(QStringLiteral("cos"), math.property(QStringLiteral("cos")));
    global.setProperty(QStringLiteral("tan"), math.property(QStringLiteral("tan")));
    global.setProperty(QStringLiteral("asin"), math.property(QStringLiteral("asin")));
    global.setProperty(QStringLiteral("acos"), math.property(QStringLiteral("acos")));
    global.setProperty(QStringLiteral("atan"), math.property(QStringLiteral("atan")));
    global.setProperty(QStringLiteral("sqrt"), math.property(QStringLiteral("sqrt")));
    global.setProperty(QStringLiteral("cbrt"), math.property(QStringLiteral("cbrt")));
    global.setProperty(QStringLiteral("abs"), math.property(QStringLiteral("abs")));
    global.setProperty(QStringLiteral("log"), math.property(QStringLiteral("log10")));
    global.setProperty(QStringLiteral("ln"), math.property(QStringLiteral("log")));
    global.setProperty(QStringLiteral("exp"), math.property(QStringLiteral("exp")));
    global.setProperty(QStringLiteral("round"), math.property(QStringLiteral("round")));
    global.setProperty(QStringLiteral("floor"), math.property(QStringLiteral("floor")));
    global.setProperty(QStringLiteral("ceil"), math.property(QStringLiteral("ceil")));
    global.setProperty(QStringLiteral("pow"), math.property(QStringLiteral("pow")));
    global.setProperty(QStringLiteral("min"), math.property(QStringLiteral("min")));
    global.setProperty(QStringLiteral("max"), math.property(QStringLiteral("max")));
}

bool Calculator::looksLikeMath(const QString &text)
{
    QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) return false;

    // Explicit math prefix
    if (trimmed.startsWith(QLatin1Char('=')) || trimmed.startsWith(QLatin1String("calc "), Qt::CaseInsensitive)) {
        return true;
    }

    // Must contain at least one digit
    bool hasDigit = false;
    for (const QChar &ch : trimmed) {
        if (ch.isDigit()) {
            hasDigit = true;
            break;
        }
    }
    if (!hasDigit) return false;

    // Must contain math operators
    static const QRegularExpression opRegex(QStringLiteral(R"([\+\-\*\/\%\^\(\)]|\b(sqrt|sin|cos|tan|pi|abs|log|ln|pow)\b)"), QRegularExpression::CaseInsensitiveOption);
    return opRegex.match(trimmed).hasMatch();
}

std::optional<QString> Calculator::evaluate(const QString &expression)
{
    QString clean = expression.trimmed();
    if (clean.startsWith(QLatin1Char('='))) {
        clean = clean.mid(1).trimmed();
    } else if (clean.startsWith(QLatin1String("calc "), Qt::CaseInsensitive)) {
        clean = clean.mid(5).trimmed();
    }

    if (clean.isEmpty() || !looksLikeMath(clean)) {
        return std::nullopt;
    }

    // Replace ^ with ** for power
    clean.replace(QLatin1Char('^'), QStringLiteral("**"));

    // Guard against arbitrary characters
    static const QRegularExpression safeChars(QStringLiteral(R"(^[0-9a-zA-Z_\+\-\*\/\%\^\(\)\.\,\s]+$)"));
    if (!safeChars.match(clean).hasMatch()) {
        return std::nullopt;
    }

    // Strictly whitelist allowed identifier tokens to prevent JS prototype/constructor injection
    static const QRegularExpression identifierRegex(QStringLiteral(R"([a-zA-Z_][a-zA-Z0-9_]*)"));
    static const QSet<QString> allowedTokens = {
        QStringLiteral("sin"), QStringLiteral("cos"), QStringLiteral("tan"),
        QStringLiteral("asin"), QStringLiteral("acos"), QStringLiteral("atan"),
        QStringLiteral("sqrt"), QStringLiteral("cbrt"), QStringLiteral("abs"),
        QStringLiteral("log"), QStringLiteral("ln"), QStringLiteral("exp"),
        QStringLiteral("round"), QStringLiteral("floor"), QStringLiteral("ceil"),
        QStringLiteral("pow"), QStringLiteral("pi"), QStringLiteral("PI"),
        QStringLiteral("e"), QStringLiteral("E"), QStringLiteral("Math"),
        QStringLiteral("min"), QStringLiteral("max")
    };

    auto it = identifierRegex.globalMatch(clean);
    while (it.hasNext()) {
        auto match = it.next();
        if (!allowedTokens.contains(match.captured())) {
            return std::nullopt;
        }
    }

    QJSValue res = m_engine.evaluate(clean);
    if (res.isError() || !res.isNumber()) {
        return std::nullopt;
    }

    double num = res.toNumber();
    if (std::isnan(num) || std::isinf(num)) {
        return std::nullopt;
    }

    // Format number nicely
    QString formatted;
    if (std::floor(num) == num && std::abs(num) < 1e15) {
        formatted = QString::number(static_cast<qint64>(num));
    } else {
        formatted = QString::number(num, 'g', 10);
    }

    return formatted;
}
