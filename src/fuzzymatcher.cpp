#include "fuzzymatcher.h"
#include "frecencyranker.h"
#include "windowswitcher.h"
#include "actionmodel.h"
#include "emojimanager.h"
#include "clipboardmanager.h"
#include "calculator.h"
#include <vector>
#include <array>
#include <functional>
#include <algorithm>
#include <QRegularExpression>

FuzzyMatcher::FuzzyMatcher(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    sort(0, Qt::AscendingOrder);

    m_windowRefreshDebounceTimer.setSingleShot(true);
    m_windowRefreshDebounceTimer.setInterval(150);
    connect(&m_windowRefreshDebounceTimer, &QTimer::timeout, this, [this]() {
        if (auto *winModel = qobject_cast<WindowListModel*>(m_windowModel)) {
            winModel->refresh();
        }
    });
}

QString FuzzyMatcher::query() const {
    return m_query;
}

void FuzzyMatcher::setWindowModel(QAbstractItemModel *windowModel) {
    m_windowModel = windowModel;
}

void FuzzyMatcher::setActionModel(QAbstractItemModel *actionModel) {
    m_actionModel = actionModel;
    // A round-trip menu replaces the list: drop the stale remainder filter
    // so the follow-up menu is shown in full.
    if (auto *actions = qobject_cast<ActionModel*>(actionModel)) {
        connect(actions, &ActionModel::pipeMenuUpdated, this, [this]() {
            if (!m_pipeMode) return;
            m_pipeRemainder.clear();
            invalidateFilter();
        });
    }
}

void FuzzyMatcher::setEmojiModel(QAbstractItemModel *emojiModel) {
    m_emojiModel = emojiModel;
}

void FuzzyMatcher::setClipboardModel(QAbstractItemModel *clipboardModel) {
    m_clipboardModel = clipboardModel;
}

void FuzzyMatcher::setAppIndexerModel(QAbstractItemModel *appModel) {
    m_appIndexerModel = appModel;
    setSourceModel(appModel);
}

// Returns the pipe action name the query refers to, if any: the query must
// equal the action name ("/windows") or extend it ("/windows fire").
static QString matchingPipeAction(ActionModel *actions, const QString &trimmed) {
    if (!actions || !trimmed.startsWith(QLatin1Char('/')))
        return {};
    for (const QString &name : actions->pipeActionNames()) {
        if (trimmed == name || trimmed.startsWith(name + QLatin1Char(' ')))
            return name;
    }
    return {};
}

void FuzzyMatcher::setQuery(const QString &newQuery) {
    if (m_query == newQuery)
        return;
    m_query = newQuery;
    m_scoreCache.clear();

    QString trimmed = m_query.trimmed();
    bool isWindowQuery = trimmed.startsWith("w:", Qt::CaseInsensitive) || 
                         trimmed.startsWith("w.", Qt::CaseInsensitive) ||
                         trimmed.startsWith("window:", Qt::CaseInsensitive) ||
                         trimmed.startsWith("w ", Qt::CaseInsensitive);
    bool isEmojiQuery = trimmed.startsWith("e:", Qt::CaseInsensitive) ||
                        trimmed.startsWith("e.", Qt::CaseInsensitive) ||
                        trimmed.startsWith("emoji:", Qt::CaseInsensitive) ||
                        trimmed.startsWith("e ", Qt::CaseInsensitive) ||
                        trimmed.startsWith(":");
    bool isClipboardQuery = trimmed.startsWith("c:", Qt::CaseInsensitive) ||
                            trimmed.startsWith("c.", Qt::CaseInsensitive) ||
                            trimmed.startsWith("clip:", Qt::CaseInsensitive) ||
                            trimmed.startsWith("cb:", Qt::CaseInsensitive) ||
                            trimmed.startsWith("c ", Qt::CaseInsensitive);
    bool isActionQuery = trimmed.startsWith("/");

    auto *actions = qobject_cast<ActionModel*>(m_actionModel);
    const QString pipeName = isActionQuery ? matchingPipeAction(actions, trimmed)
                                           : QString();

    if (!pipeName.isEmpty() && actions) {
        // dmenu-style pipe mode: run the script, list its stdout lines.
        // "input": "query" actions get the remainder as $1 and skip filtering.
        if (pipeName != m_pipeActionName || !m_pipeMode) {
            actions->triggerPipeAction(pipeName, trimmed.mid(pipeName.length()).trimmed());
        }
        m_pipeMode = true;
        m_pipeActionName = pipeName;
        m_pipeRemainder = trimmed.mid(pipeName.length()).trimmed();
        m_pipeFilterEnabled = actions->pipeFiltersByRemainder();
        if (sourceModel() != actions->pipeResultModel()) {
            setSourceModel(actions->pipeResultModel());
        }
    } else {
        m_pipeMode = false;
        m_pipeActionName.clear();
        m_pipeRemainder.clear();

        if (isWindowQuery && m_windowModel) {
            if (sourceModel() != m_windowModel) {
                setSourceModel(m_windowModel);
                if (auto *winModel = qobject_cast<WindowListModel*>(m_windowModel)) {
                    winModel->refresh();
                }
            } else {
                m_windowRefreshDebounceTimer.start();
            }
        } else if (isEmojiQuery && m_emojiModel) {
            if (sourceModel() != m_emojiModel) {
                setSourceModel(m_emojiModel);
            }
        } else if (isClipboardQuery && m_clipboardModel) {
            if (sourceModel() != m_clipboardModel) {
                setSourceModel(m_clipboardModel);
                if (auto *clipModel = qobject_cast<ClipboardManager*>(m_clipboardModel)) {
                    clipModel->refresh();
                }
            }
        } else if (isActionQuery && m_actionModel) {
            if (sourceModel() != m_actionModel) {
                setSourceModel(m_actionModel);
            }
        } else if (!isWindowQuery && !isEmojiQuery && !isClipboardQuery && !isActionQuery && m_appIndexerModel) {
            if (sourceModel() != m_appIndexerModel) {
                setSourceModel(m_appIndexerModel);
            }
        }
    }

    // Check for inline math calculation in default app search mode
    if (!isWindowQuery && !isEmojiQuery && !isClipboardQuery && !isActionQuery && !m_pipeMode) {
        auto res = m_calculator.evaluate(trimmed);
        if (res.has_value()) {
            m_mathResult = QStringLiteral("= ") + *res;
        } else {
            m_mathResult = std::nullopt;
        }
    } else {
        m_mathResult = std::nullopt;
    }

    m_scoreCache.clear();
    invalidate();
    sort(0, Qt::AscendingOrder);

    emit queryChanged();
}

void FuzzyMatcher::setSourceModel(QAbstractItemModel *sourceModel) {
    if (!m_appIndexerModel && sourceModel) {
        m_appIndexerModel = sourceModel;
    }
    m_scoreCache.clear();

    // Drop connections to the previous source model before switching,
    // otherwise every app/window/pipe flip piles up stale signal hooks.
    for (const QMetaObject::Connection &c : m_sourceConnections)
        disconnect(c);
    m_sourceConnections.clear();

    QSortFilterProxyModel::setSourceModel(sourceModel);
    updateRoleKeys();
    if (sourceModel) {
        auto track = [this](QMetaObject::Connection c) {
            m_sourceConnections.append(c);
        };
        track(connect(sourceModel, &QAbstractItemModel::modelReset, this, [this]() {
            m_scoreCache.clear();
            updateRoleKeys();
            invalidate();
        }));
        track(connect(sourceModel, &QAbstractItemModel::rowsInserted, this, [this]() {
            m_scoreCache.clear();
            invalidate();
        }));
        track(connect(sourceModel, &QAbstractItemModel::rowsRemoved, this, [this]() {
            m_scoreCache.clear();
            invalidate();
        }));
        track(connect(sourceModel, &QAbstractItemModel::dataChanged, this, [this]() {
            m_scoreCache.clear();
            invalidate();
        }));
    }
}

void FuzzyMatcher::updateRoleKeys() {
    m_nameRole = Qt::UserRole + 1;
    m_genericNameRole = Qt::UserRole + 2;
    m_commentRole = Qt::UserRole + 3;
    m_execRole = Qt::UserRole + 4;
    m_iconNameRole = Qt::UserRole + 5;
    m_categoriesRole = Qt::UserRole + 6;
    m_keywordsRole = Qt::UserRole + 7;
    m_desktopFileRole = Qt::UserRole + 8;

    if (!sourceModel()) return;
    m_corpusHasCyrillic = false;
    QHash<int, QByteArray> roles = sourceModel()->roleNames();
    for (auto it = roles.begin(); it != roles.end(); ++it) {
        if (it.value() == "name") m_nameRole = it.key();
        else if (it.value() == "genericName") m_genericNameRole = it.key();
        else if (it.value() == "comment") m_commentRole = it.key();
        else if (it.value() == "exec") m_execRole = it.key();
        else if (it.value() == "iconName") m_iconNameRole = it.key();
        else if (it.value() == "categories") m_categoriesRole = it.key();
        else if (it.value() == "keywords") m_keywordsRole = it.key();
        else if (it.value() == "desktopFile") m_desktopFileRole = it.key();
    }
    int rows = sourceModel()->rowCount();
    for (int i = 0; i < rows; ++i) {
        QString name = m_nameRole != -1 ? sourceModel()->data(sourceModel()->index(i, 0), m_nameRole).toString() : QString();
        for (QChar c : name) {
            ushort u = c.unicode();
            if (u >= 0x0400 && u <= 0x04FF) {
                m_corpusHasCyrillic = true;
                break;
            }
        }
        if (m_corpusHasCyrillic) break;
    }
}

static bool isPureLatin(const QString &str) {
    for (QChar c : str) {
        ushort u = c.unicode();
        if ((u >= 'a' && u <= 'z') || (u >= 'A' && u <= 'Z') || u == '[' || u == ']' || u == ';' || u == '\'' || u == ',' || u == '.') continue;
        if (c.isSpace() || c.isDigit()) continue;
        return false;
    }
    return true;
}

static QString convertRuToEn(const QString &input) {
    static const QHash<QChar, QChar> ruToEn = {
        {u'ё', '`'}, {u'Ё', '~'},
        {u'й', 'q'}, {u'ц', 'w'}, {u'у', 'e'}, {u'к', 'r'}, {u'е', 't'}, {u'н', 'y'}, {u'г', 'u'}, {u'ш', 'i'}, {u'щ', 'o'}, {u'з', 'p'}, {u'х', '['}, {u'ъ', ']'},
        {u'ф', 'a'}, {u'ы', 's'}, {u'в', 'd'}, {u'а', 'f'}, {u'п', 'g'}, {u'р', 'h'}, {u'о', 'j'}, {u'л', 'k'}, {u'д', 'l'}, {u'ж', ';'}, {u'э', '\''},
        {u'я', 'z'}, {u'ч', 'x'}, {u'с', 'c'}, {u'м', 'v'}, {u'и', 'b'}, {u'т', 'n'}, {u'ь', 'm'}, {u'б', ','}, {u'ю', '.'},
        {u'Й', 'Q'}, {u'Ц', 'W'}, {u'У', 'E'}, {u'К', 'R'}, {u'Е', 'T'}, {u'Н', 'Y'}, {u'Г', 'U'}, {u'Ш', 'I'}, {u'Щ', 'O'}, {u'З', 'P'}, {u'Х', '{'}, {u'Ъ', '}'},
        {u'Ф', 'A'}, {u'Ы', 'S'}, {u'В', 'D'}, {u'А', 'F'}, {u'П', 'G'}, {u'Р', 'H'}, {u'О', 'J'}, {u'Л', 'K'}, {u'Д', 'L'}, {u'Ж', ':'}, {u'Э', '"'},
        {u'Я', 'Z'}, {u'Ч', 'X'}, {u'С', 'C'}, {u'М', 'V'}, {u'И', 'B'}, {u'Т', 'N'}, {u'Ь', 'M'}, {u'Б', '<'}, {u'Ю', '>'}
    };

    QString result;
    result.reserve(input.length());
    for (QChar c : input) {
        if (ruToEn.contains(c)) {
            result.append(ruToEn.value(c));
        } else {
            result.append(c);
        }
    }
    return result;
}

static QString convertRuToEnMnemonic(const QString &input) {
    static const QHash<QChar, QString> ruToMnemonic = {
        {u'а', "a"}, {u'б', "b"}, {u'в', "v"}, {u'г', "g"}, {u'д', "d"}, {u'е', "e"}, {u'ё', "yo"},
        {u'ж', "zh"}, {u'з', "z"}, {u'и', "i"}, {u'й', "y"}, {u'к', "k"}, {u'л', "l"}, {u'м', "m"},
        {u'н', "n"}, {u'о', "o"}, {u'п', "p"}, {u'р', "r"}, {u'с', "s"}, {u'т', "t"}, {u'у', "u"},
        {u'ф', "f"}, {u'х', "h"}, {u'ц', "c"}, {u'ч', "ch"}, {u'ш', "sh"}, {u'щ', "sch"}, {u'ь', "'"},
        {u'ы', "y"}, {u'ъ', "'"}, {u'э', "e"}, {u'ю', "yu"}, {u'я', "ya"},
        {u'А', "A"}, {u'Б', "B"}, {u'В', "V"}, {u'Г', "G"}, {u'Д', "D"}, {u'Е', "E"}, {u'Ё', "Yo"},
        {u'Ж', "Zh"}, {u'З', "Z"}, {u'И', "I"}, {u'Й', "Y"}, {u'К', "K"}, {u'Л', "L"}, {u'М', "M"},
        {u'Н', "N"}, {u'О', "O"}, {u'П', "P"}, {u'Р', "R"}, {u'С', "S"}, {u'Т', "T"}, {u'У', "U"},
        {u'Ф', "F"}, {u'Х', "H"}, {u'Ц', "C"}, {u'Ч', "Ch"}, {u'Ш', "Sh"}, {u'Щ', "Sch"}, {u'Ь', "'"},
        {u'Ы', "Y"}, {u'Ъ', "'"}, {u'Э', "E"}, {u'Ю', "Yu"}, {u'Я', "Ya"}
    };

    QString result;
    result.reserve(input.length() * 2);
    for (QChar c : input) {
        if (ruToMnemonic.contains(c)) {
            result.append(ruToMnemonic.value(c));
        } else {
            result.append(c);
        }
    }
    return result;
}

static QString convertEnToRu(const QString &input) {
    static const QHash<QChar, QChar> enToRu = {
        {'`', u'ё'}, {'~', u'Ё'},
        {'q', u'й'}, {'w', u'ц'}, {'e', u'у'}, {'r', u'к'}, {'t', u'е'}, {'y', u'н'}, {'u', u'г'}, {'i', u'ш'}, {'o', u'щ'}, {'p', u'з'}, {'[', u'х'}, {']', u'ъ'},
        {'a', u'ф'}, {'s', u'ы'}, {'d', u'в'}, {'f', u'а'}, {'g', u'п'}, {'h', u'р'}, {'j', u'о'}, {'k', u'л'}, {'l', u'д'}, {';', u'ж'}, {'\'', u'э'},
        {'z', u'я'}, {'x', u'ч'}, {'c', u'с'}, {'v', u'м'}, {'b', u'и'}, {'n', u'т'}, {'m', u'ь'}, {',', u'б'}, {'.', u'ю'},
        {'Q', u'Й'}, {'W', u'Ц'}, {'E', u'У'}, {'R', u'К'}, {'T', u'Е'}, {'Y', u'Н'}, {'U', u'Г'}, {'I', u'Ш'}, {'O', u'Щ'}, {'P', u'З'}, {'{', u'Х'}, {'}', u'Ъ'},
        {'A', u'Ф'}, {'S', u'Ы'}, {'D', u'В'}, {'F', u'А'}, {'G', u'П'}, {'H', u'Р'}, {'J', u'О'}, {'K', u'Л'}, {'L', u'Д'}, {':', u'Ж'}, {'"', u'Э'},
        {'Z', u'Я'}, {'X', u'Ч'}, {'C', u'С'}, {'V', u'М'}, {'B', u'И'}, {'N', u'Т'}, {'M', u'Ь'}, {'<', u'Б'}, {'>', u'Ю'}
    };

    QString result;
    result.reserve(input.length());
    for (QChar c : input) {
        if (enToRu.contains(c)) {
            result.append(enToRu.value(c));
        } else {
            result.append(c);
        }
    }
    return result;
}

static int boundedDamerauLevenshtein(const QString &s1, const QString &s2, int maxDist) {
    int len1 = s1.length();
    int len2 = s2.length();
    if (std::abs(len1 - len2) > maxDist) return maxDist + 1;

    std::vector<int> prev2(len2 + 1, 0), prev(len2 + 1), curr(len2 + 1);
    for (int j = 0; j <= len2; ++j) prev[j] = j;

    for (int i = 1; i <= len1; ++i) {
        curr[0] = i;
        int minInRow = curr[0];
        for (int j = 1; j <= len2; ++j) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            curr[j] = std::min({ prev[j] + 1, curr[j - 1] + 1, prev[j - 1] + cost });
            if (i > 1 && j > 1 && s1[i - 1] == s2[j - 2] && s1[i - 2] == s2[j - 1]) {
                curr[j] = std::min(curr[j], prev2[j - 2] + 1);
            }
            minInRow = std::min(minInRow, curr[j]);
        }
        if (minInRow > maxDist) return maxDist + 1;
        prev2 = prev;
        prev  = curr;
    }
    return prev[len2];
}

static int matchSingleQuery(const QString &name, const QString &genericName, const QString &comment, const QString &exec, const QStringList &keywords, const QString &query) {
    if (query.isEmpty()) return 1;

    QString qLower = query.toLower();
    QString nLower = name.toLower();
    QString gLower = genericName.toLower();
    QString cLower = comment.toLower();

    int qLen = qLower.length();
    int nLen = nLower.length();

    // 1. Direct Name Prefix Match (HIGHEST PRIORITY: 100,000+)
    if (nLower.startsWith(qLower)) {
        return 100000 + (1000 - nLen);
    }

    // 2. Acronym Match (e.g. "vsc" for "Visual Studio Code" or "tb" for "Telegram Desktop" -> 90,000+)
    QStringList words = nLower.split(QRegularExpression("[\\s\\-_]+"));
    if (words.size() > 1 && qLen <= words.size()) {
        bool acronymMatch = true;
        for (int i = 0; i < qLen; ++i) {
            if (words[i].isEmpty() || words[i][0] != qLower[i]) {
                acronymMatch = false;
                break;
            }
        }
        if (acronymMatch) {
            return 90000 + (1000 - nLen);
        }
    }

    // 3. Word Boundary Prefix Match in Name (e.g. "Theme Selector" for "Sel" -> 80,000+)
    for (int i = 1; i < words.size(); ++i) {
        if (words[i].startsWith(qLower)) {
            return 80000 + (1000 - nLen) - (i * 10);
        }
    }

    // 4. Exec Binary Name Match (e.g. "pavucontrol" -> "PulseAudio Volume Control" -> 75,000+)
    if (!exec.isEmpty()) {
        QString cleanExec = exec;
        int spaceIdx = cleanExec.indexOf(QLatin1Char(' '));
        if (spaceIdx != -1) cleanExec = cleanExec.left(spaceIdx);
        int slashIdx = cleanExec.lastIndexOf(QLatin1Char('/'));
        if (slashIdx != -1) cleanExec = cleanExec.mid(slashIdx + 1);
        cleanExec = cleanExec.toLower();

        if (cleanExec.startsWith(qLower)) {
            return 75000 + (1000 - cleanExec.length());
        }
        if (cleanExec.contains(qLower)) {
            return 60000 + (1000 - cleanExec.length());
        }
    }

    // 5. Exact Substring Match in Name (e.g. "Chromium" for "ro" -> 50,000+)
    int subPos = nLower.indexOf(qLower);
    if (subPos != -1) {
        return 50000 + (1000 - subPos * 10 - nLen);
    }

    // 6. Keywords / Desktop Categories Match (45,000+)
    for (const QString &kw : keywords) {
        if (kw.toLower().startsWith(qLower)) {
            return 45000 + (1000 - kw.length());
        }
    }

    // 7. High-Precision Damerau-Levenshtein Edit Distance (e.g. "ca" vs "ac", "stema" vs "steam")
    int maxAllowedDist = (qLen <= 4) ? 1 : 2;
    int editDist = boundedDamerauLevenshtein(qLower, nLower, maxAllowedDist);
    if (editDist <= maxAllowedDist) {
        return 35000 - (editDist * 5000) - (std::abs(nLen - qLen) * 200);
    }

    // 8. Strict Subsequence Match with Compactness & Gap Checks
    if (qLen >= 2) {
        int qIdx = 0;
        int firstMatch = -1;
        int lastMatch = -1;
        for (int i = 0; i < nLen && qIdx < qLen; ++i) {
            if (nLower[i] == qLower[qIdx]) {
                if (firstMatch == -1) firstMatch = i;
                lastMatch = i;
                qIdx++;
            }
        }

        if (qIdx == qLen && firstMatch != -1) {
            int span = lastMatch - firstMatch + 1;
            int extraGap = span - qLen;
            int lenDiff = std::abs(nLen - qLen);
            if (extraGap <= std::max(2, qLen) && span <= (qLen * 2 + 1)) {
                int subseqScore = 25000 - (extraGap * 2500) - (lenDiff * 250) - (firstMatch * 300);
                if (subseqScore > 5000) return subseqScore;
            }
        }
    }

    // 9. GenericName or Comment Prefix/Substring (1,000+)
    if (gLower.startsWith(qLower) || cLower.startsWith(qLower)) return 1000;
    if (gLower.contains(qLower) || cLower.contains(qLower)) return 500;

    return 0;
}

static QString cleanQueryString(const QString &query) {
    QString clean = query.trimmed();
    if (clean.startsWith(QLatin1String("w:"), Qt::CaseInsensitive)) clean = clean.mid(2).trimmed();
    else if (clean.startsWith(QLatin1String("w."), Qt::CaseInsensitive)) clean = clean.mid(2).trimmed();
    else if (clean.startsWith(QLatin1String("window:"), Qt::CaseInsensitive)) clean = clean.mid(7).trimmed();
    else if (clean.startsWith(QLatin1String("w "), Qt::CaseInsensitive)) clean = clean.mid(2).trimmed();
    else if (clean.startsWith(QLatin1String("e:"), Qt::CaseInsensitive)) clean = clean.mid(2).trimmed();
    else if (clean.startsWith(QLatin1String("e."), Qt::CaseInsensitive)) clean = clean.mid(2).trimmed();
    else if (clean.startsWith(QLatin1String("emoji:"), Qt::CaseInsensitive)) clean = clean.mid(6).trimmed();
    else if (clean.startsWith(QLatin1String("e "), Qt::CaseInsensitive)) clean = clean.mid(2).trimmed();
    else if (clean.startsWith(QLatin1String("c:"), Qt::CaseInsensitive)) clean = clean.mid(2).trimmed();
    else if (clean.startsWith(QLatin1String("c."), Qt::CaseInsensitive)) clean = clean.mid(2).trimmed();
    else if (clean.startsWith(QLatin1String("clip:"), Qt::CaseInsensitive)) clean = clean.mid(5).trimmed();
    else if (clean.startsWith(QLatin1String("cb:"), Qt::CaseInsensitive)) clean = clean.mid(3).trimmed();
    else if (clean.startsWith(QLatin1String("c "), Qt::CaseInsensitive)) clean = clean.mid(2).trimmed();
    else if (clean.startsWith(QLatin1String("b:"), Qt::CaseInsensitive)) clean = clean.mid(2).trimmed();
    else if (clean.startsWith(QLatin1String("browser:"), Qt::CaseInsensitive)) clean = clean.mid(8).trimmed();
    else if (clean.startsWith(QLatin1String("google:"), Qt::CaseInsensitive)) clean = clean.mid(7).trimmed();
    else if (clean.startsWith(QLatin1String("chrome:"), Qt::CaseInsensitive)) clean = clean.mid(7).trimmed();
    else if (clean.startsWith(QLatin1String("g:"), Qt::CaseInsensitive)) clean = clean.mid(2).trimmed();
    else if (clean.startsWith(QLatin1String("web:"), Qt::CaseInsensitive)) clean = clean.mid(4).trimmed();
    else if (clean.startsWith(QLatin1Char('?'))) clean = clean.mid(1).trimmed();
    else if (clean.startsWith(QLatin1Char('/'))) clean = clean.mid(1).trimmed();
    else if (clean.startsWith(QLatin1Char(':'))) clean = clean.mid(1).trimmed();
    else if (clean.startsWith(QLatin1Char('='))) clean = clean.mid(1).trimmed();
    return clean;
}

int FuzzyMatcher::score(int sourceRow) const {
    QString cleanQuery = cleanQueryString(m_query);
    
    if (cleanQuery.isEmpty()) return 1000;
    
    if (m_scoreCache.count(sourceRow)) {
        return m_scoreCache[sourceRow];
    }
    
    QModelIndex idx = sourceModel()->index(sourceRow, 0);
    
    QString name = m_nameRole != -1 ? idx.data(m_nameRole).toString() : QString();
    QString genericName = m_genericNameRole != -1 ? idx.data(m_genericNameRole).toString() : QString();
    QString comment = m_commentRole != -1 ? idx.data(m_commentRole).toString() : QString();
    QString exec = m_execRole != -1 ? idx.data(m_execRole).toString() : QString();
    QStringList keywords = m_keywordsRole != -1 ? idx.data(m_keywordsRole).toStringList() : QStringList();
    QString desktopFile = m_desktopFileRole != -1 ? idx.data(m_desktopFileRole).toString() : QString();

    if (name.isEmpty() && genericName.isEmpty()) {
        m_scoreCache[sourceRow] = 0;
        return 0;
    }

    int scoreOrig = matchSingleQuery(name, genericName, comment, exec, keywords, cleanQuery);
    
    QString qwertTrans = convertRuToEn(cleanQuery);
    int scoreQwerty = (qwertTrans != cleanQuery) ? matchSingleQuery(name, genericName, comment, exec, keywords, qwertTrans) : 0;

    int scoreEnToRu = 0;
    if (m_corpusHasCyrillic && isPureLatin(cleanQuery)) {
        QString enToRuTrans = convertEnToRu(cleanQuery);
        if (enToRuTrans != cleanQuery) {
            scoreEnToRu = matchSingleQuery(name, genericName, comment, exec, keywords, enToRuTrans);
        }
    }

    QString mnemonTrans = convertRuToEnMnemonic(cleanQuery);
    int scoreMnemonic = (mnemonTrans != cleanQuery && mnemonTrans != qwertTrans) ? matchSingleQuery(name, genericName, comment, exec, keywords, mnemonTrans) : 0;

    int finalScore = std::max({scoreOrig, scoreQwerty, scoreEnToRu, scoreMnemonic});

    // Add Logarithmically Clamped Frecency Ranker bonus (max 900 points, never crosses 10,000 relevance tier boundary)
    // Exclude ephemeral window addresses (e.g. window:0x...) to prevent frecency pollution
    if (finalScore > 0 && !desktopFile.isEmpty() && !desktopFile.startsWith(QLatin1String("window:"))) {
        double rawFrecency = FrecencyRanker::instance()->getScore(desktopFile);
        int historyBonus = std::clamp(qRound(180.0 * std::log1p(rawFrecency)), 0, 900);
        finalScore += historyBonus;
    }

    m_scoreCache[sourceRow] = finalScore;
    return finalScore;
}

QString FuzzyMatcher::formatHighlightedName(const QString &name, const QString &query) const {
    QString clean = cleanQueryString(query);
    if (clean.isEmpty()) return name.toHtmlEscaped();

    QString activeQuery = clean;
    int bestScore = matchSingleQuery(name, QString(), QString(), QString(), QStringList(), activeQuery);
    
    QString qwertTrans = convertRuToEn(clean);
    int sQ = (qwertTrans != clean) ? matchSingleQuery(name, QString(), QString(), QString(), QStringList(), qwertTrans) : 0;
    if (sQ > bestScore) {
        bestScore = sQ;
        activeQuery = qwertTrans;
    }

    if (m_corpusHasCyrillic && isPureLatin(clean)) {
        QString enToRuTrans = convertEnToRu(clean);
        int sE = (enToRuTrans != clean) ? matchSingleQuery(name, QString(), QString(), QString(), QStringList(), enToRuTrans) : 0;
        if (sE > bestScore) {
            bestScore = sE;
            activeQuery = enToRuTrans;
        }
    }

    QString mnemonTrans = convertRuToEnMnemonic(clean);
    int sM = (mnemonTrans != clean && mnemonTrans != qwertTrans) ? matchSingleQuery(name, QString(), QString(), QString(), QStringList(), mnemonTrans) : 0;
    if (sM > bestScore) {
        bestScore = sM;
        activeQuery = mnemonTrans;
    }

    QString qLower = activeQuery.toLower();
    QString nLower = name.toLower();

    int pos = nLower.indexOf(qLower);
    if (pos != -1) {
        QString before = name.left(pos).toHtmlEscaped();
        QString matched = name.mid(pos, activeQuery.length()).toHtmlEscaped();
        QString after = name.mid(pos + activeQuery.length()).toHtmlEscaped();
        return QString("%1<u>%2</u>%3").arg(before, matched, after);
    }

    QString result;
    int qIdx = 0;
    int qLen = activeQuery.length();
    for (int i = 0; i < name.length(); ++i) {
        if (qIdx < qLen && name[i].toLower() == activeQuery[qIdx].toLower()) {
            result += QString("<u>%1</u>").arg(QString(name[i]).toHtmlEscaped());
            qIdx++;
        } else {
            result += QString(name[i]).toHtmlEscaped();
        }
    }
    return result;
}

int FuzzyMatcher::rowCount(const QModelIndex &parent) const {
    int base = QSortFilterProxyModel::rowCount(parent);
    if (!parent.isValid() && m_mathResult.has_value()) {
        return base + 1;
    }
    return base;
}

QModelIndex FuzzyMatcher::index(int row, int column, const QModelIndex &parent) const {
    if (row < 0 || column < 0 || parent.isValid())
        return QModelIndex();
    if (m_mathResult.has_value() && row == 0) {
        return createIndex(row, column);
    }
    int sourceRow = m_mathResult.has_value() ? row - 1 : row;
    return QSortFilterProxyModel::index(sourceRow, column, parent);
}

QModelIndex FuzzyMatcher::mapToSource(const QModelIndex &proxyIndex) const {
    if (!proxyIndex.isValid()) return QModelIndex();
    if (m_mathResult.has_value()) {
        if (proxyIndex.row() == 0) return QModelIndex();
        int proxyRow = proxyIndex.row() - 1;
        if (proxyRow < 0) return QModelIndex();
        QModelIndex proxySub = QSortFilterProxyModel::index(proxyRow, proxyIndex.column());
        return QSortFilterProxyModel::mapToSource(proxySub);
    }
    return QSortFilterProxyModel::mapToSource(proxyIndex);
}

QVariant FuzzyMatcher::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();

    if (m_mathResult.has_value() && index.row() == 0) {
        switch (role) {
        case Qt::DisplayRole:
        case Qt::UserRole + 1: // NameRole
            return m_mathResult.value();
        case Qt::UserRole + 2: // GenericNameRole
            return QStringLiteral("Math Result • Press Enter to copy to clipboard");
        case Qt::UserRole + 3: // CommentRole
            return QStringLiteral("Calculator");
        case Qt::UserRole + 4: // ExecRole
            return QStringLiteral("__copy__:") + m_mathResult.value().mid(2).trimmed();
        case Qt::UserRole + 5: // IconNameRole
            return QStringLiteral("accessories-calculator");
        case Qt::UserRole + 6: // CategoriesRole
            return QStringList{QStringLiteral("Calculator")};
        case Qt::UserRole + 7: // KeywordsRole
            return QStringList{QStringLiteral("calc"), QStringLiteral("math")};
        case Qt::UserRole + 8: // DesktopFileRole
            return QStringLiteral("math:result");
        case ScoreRole:
            return 999999;
        case HighlightedNameRole:
            return m_mathResult.value();
        default:
            return QVariant();
        }
    }

    if (role == ScoreRole) {
        int actualRow = (m_mathResult.has_value() && index.row() > 0) ? index.row() - 1 : index.row();
        return score(actualRow);
    }
    if (role == HighlightedNameRole) {
        QModelIndex src = mapToSource(index);
        if (!src.isValid() || !sourceModel()) return QVariant();
        QString name = sourceModel()->data(src, m_nameRole).toString();
        return formatHighlightedName(name, m_query);
    }

    if (m_mathResult.has_value() && index.row() > 0) {
        QModelIndex proxySub = QSortFilterProxyModel::index(index.row() - 1, index.column());
        return QSortFilterProxyModel::data(proxySub, role);
    }

    return QSortFilterProxyModel::data(index, role);
}

QHash<int, QByteArray> FuzzyMatcher::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::UserRole + 1] = "name";
    roles[Qt::UserRole + 2] = "genericName";
    roles[Qt::UserRole + 3] = "comment";
    roles[Qt::UserRole + 4] = "exec";
    roles[Qt::UserRole + 5] = "iconName";
    roles[Qt::UserRole + 6] = "categories";
    roles[Qt::UserRole + 7] = "keywords";
    roles[Qt::UserRole + 8] = "desktopFile";
    roles[ScoreRole] = "score";
    roles[HighlightedNameRole] = "highlightedName";
    return roles;
}

bool FuzzyMatcher::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    Q_UNUSED(source_parent);
    if (m_pipeMode) {
        // Pipe results filter on the remainder after the action name:
        // "/windows fire" keeps only lines containing "fire". Actions that
        // consume the query themselves ("input": "query") show everything.
        if (m_pipeRemainder.isEmpty() || !m_pipeFilterEnabled) return true;
        QString name = m_nameRole != -1
            ? sourceModel()->data(sourceModel()->index(source_row, 0), m_nameRole).toString()
            : QString();
        return name.contains(m_pipeRemainder, Qt::CaseInsensitive);
    }
    if (m_query.isEmpty()) return true;
    return score(source_row) > 0;
}

bool FuzzyMatcher::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const {
    if (m_pipeMode) {
        // Keep the script's stdout order while filtering.
        return source_left.row() < source_right.row();
    }
    if (m_query.isEmpty()) {
        QString nameLeft = m_nameRole != -1 ? sourceModel()->data(source_left, m_nameRole).toString() : QString();
        QString nameRight = m_nameRole != -1 ? sourceModel()->data(source_right, m_nameRole).toString() : QString();
        return nameLeft.compare(nameRight, Qt::CaseInsensitive) < 0;
    }
    
    int scoreL = score(source_left.row());
    int scoreR = score(source_right.row());
    
    if (scoreL != scoreR) {
        return scoreL > scoreR;
    }
    
    QString nameLeft = m_nameRole != -1 ? sourceModel()->data(source_left, m_nameRole).toString() : QString();
    QString nameRight = m_nameRole != -1 ? sourceModel()->data(source_right, m_nameRole).toString() : QString();
    return nameLeft.compare(nameRight, Qt::CaseInsensitive) < 0;
}
