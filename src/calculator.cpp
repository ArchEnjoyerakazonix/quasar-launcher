#include "calculator.h"
#include <QRegularExpression>
#include <cmath>

Calculator::Calculator()
{
    // Inject math helpers and constants into JS scope
    m_engine.evaluate(QStringLiteral(
        "const pi = Math.PI, PI = Math.PI, e = Math.E, E = Math.E;\n"
        "const sin = Math.sin, cos = Math.cos, tan = Math.tan;\n"
        "const asin = Math.asin, acos = Math.acos, atan = Math.atan;\n"
        "const sqrt = Math.sqrt, cbrt = Math.cbrt, abs = Math.abs;\n"
        "const log = Math.log10, ln = Math.log, exp = Math.exp;\n"
        "const round = Math.round, floor = Math.floor, ceil = Math.ceil;\n"
        "const pow = Math.pow;\n"
    ));
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

    // Guard against arbitrary dangerous JS
    static const QRegularExpression safeChars(QStringLiteral(R"(^[0-9a-zA-Z_\+\-\*\/\%\^\(\)\.\,\s]+$)"));
    if (!safeChars.match(clean).hasMatch()) {
        return std::nullopt;
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
