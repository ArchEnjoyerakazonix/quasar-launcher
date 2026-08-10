#include "fuzzymatcher.h"
#include "frecencyranker.h"
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
}

QString FuzzyMatcher::query() const {
    return m_query;
}

void FuzzyMatcher::setQuery(const QString &newQuery) {
    if (m_query == newQuery)
        return;
    m_query = newQuery;
    m_scoreCache.clear();
    invalidate();
    sort(0, Qt::AscendingOrder);
    emit queryChanged();
}

void FuzzyMatcher::setSourceModel(QAbstractItemModel *sourceModel) {
    QSortFilterProxyModel::setSourceModel(sourceModel);
    updateRoleKeys();
}

void FuzzyMatcher::updateRoleKeys() {
    if (!sourceModel()) return;
    QHash<int, QByteArray> roles = sourceModel()->roleNames();
    for (auto it = roles.begin(); it != roles.end(); ++it) {
        if (it.value() == "name") m_nameRole = it.key();
        else if (it.value() == "genericName") m_genericNameRole = it.key();
        else if (it.value() == "comment") m_commentRole = it.key();
        else if (it.value() == "exec") m_execRole = it.key();
        else if (it.value() == "categories") m_categoriesRole = it.key();
        else if (it.value() == "keywords") m_keywordsRole = it.key();
        else if (it.value() == "desktopFile") m_desktopFileRole = it.key();
        else if (it.value() == "frecency") m_frecencyRole = it.key();
    }
}

int FuzzyMatcher::calculateScore(const QString &textStr, const QString &patternStr) const {
    if (patternStr.isEmpty()) return 1;
    if (textStr.isEmpty()) return 0;
    
    int m = patternStr.length();
    int n = textStr.length();
    
    std::vector<std::vector<std::array<int, 2>>> memo(m, std::vector<std::array<int, 2>>(n, {-10000, -10000}));
    
    std::function<int(int, int, bool)> solve = [&](int pIdx, int tIdx, bool prevMatched) -> int {
        if (pIdx == m) return 0;
        if (tIdx == n) return -10000;
        
        int pM = prevMatched ? 1 : 0;
        if (memo[pIdx][tIdx][pM] != -10000) {
            return memo[pIdx][tIdx][pM];
        }
        
        int score = solve(pIdx, tIdx + 1, false);
        if (score != -10000) {
            score -= 1;
        }
        
        if (patternStr[pIdx].toLower() == textStr[tIdx].toLower()) {
            int matchScore = 1;
            if (pIdx == 0 && tIdx == 0) {
                matchScore += 100;
            }
            if (tIdx > 0) {
                QChar prev = textStr[tIdx - 1];
                QChar curr = textStr[tIdx];
                if (prev.isSpace() || prev == '-' || prev == '_') {
                    matchScore += 8;
                } else if (prev.isLower() && curr.isUpper()) {
                    matchScore += 6;
                }
            }
            if (prevMatched) {
                matchScore += 10;
            }
            
            int restScore = solve(pIdx + 1, tIdx + 1, true);
            if (restScore != -10000) {
                score = std::max(score, matchScore + restScore);
            }
        }
        
        memo[pIdx][tIdx][pM] = score;
        return score;
    };
    
    int res = solve(0, 0, false);
    return res <= -5000 ? 0 : std::max(1, res);
}

static QString convertRuToEn(const QString &input) {
    static const QHash<QChar, QChar> ruToEn = {
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
        {u'а', "a"}, {u'б', "b"}, {u'в', "v"}, {u'г', "g"}, {u'д', "d"}, {u'е', "e"}, {u'ё', "e"},
        {u'ж', "z"}, {u'з', "z"}, {u'и', "i"}, {u'й', "y"}, {u'к', "k"}, {u'л', "l"}, {u'м', "m"},
        {u'н', "n"}, {u'о', "o"}, {u'п', "p"}, {u'р', "r"}, {u'с', "s"}, {u'т', "t"}, {u'у', "u"},
        {u'ф', "f"}, {u'х', "h"}, {u'ц', "c"}, {u'ч', "ch"}, {u'ш', "sh"}, {u'щ', "sch"}, {u'ь', "'"},
        {u'ы', "y"}, {u'ъ', "'"}, {u'э', "e"}, {u'ю', "u"}, {u'я', "ya"},
        {u'А', "A"}, {u'Б', "B"}, {u'В', "V"}, {u'Г', "G"}, {u'Д', "D"}, {u'Е', "E"}, {u'Ё', "E"},
        {u'Ж', "Z"}, {u'З', "Z"}, {u'И', "I"}, {u'Й', "Y"}, {u'К', "K"}, {u'Л', "L"}, {u'М', "M"},
        {u'Н', "N"}, {u'О', "O"}, {u'П', "P"}, {u'Р', "R"}, {u'С', "S"}, {u'Т', "T"}, {u'У', "U"},
        {u'Ф', "F"}, {u'Х', "H"}, {u'Ц', "C"}, {u'Ч', "Ch"}, {u'Ш', "Sh"}, {u'Щ', "Sch"}, {u'Ь', "'"},
        {u'Ы', "Y"}, {u'Ъ', "'"}, {u'Э', "E"}, {u'Ю', "U"}, {u'Я', "Ya"}
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

static int boundedDamerauLevenshtein(const QString &s1, const QString &s2, int maxDist) {
    int len1 = s1.length();
    int len2 = s2.length();
    if (std::abs(len1 - len2) > maxDist) return maxDist + 1;

    std::vector<int> prev(len2 + 1), curr(len2 + 1);
    for (int j = 0; j <= len2; ++j) prev[j] = j;

    for (int i = 1; i <= len1; ++i) {
        curr[0] = i;
        int minInRow = curr[0];
        for (int j = 1; j <= len2; ++j) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            curr[j] = std::min({ prev[j] + 1, curr[j - 1] + 1, prev[j - 1] + cost });
            if (i > 1 && j > 1 && s1[i - 1] == s2[j - 2] && s1[i - 2] == s2[j - 1]) {
                curr[j] = std::min(curr[j], prev[j - 2] + 1);
            }
            minInRow = std::min(minInRow, curr[j]);
        }
        if (minInRow > maxDist) return maxDist + 1;
        prev = curr;
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

    // 7. High-Precision Damerau-Levenshtein Edit Distance (e.g. "stream" vs "Steam" - 1 insertion/typo)
    int maxAllowedDist = (qLen <= 4) ? 1 : 2;
    int editDist = boundedDamerauLevenshtein(qLower, nLower, maxAllowedDist);
    if (editDist <= maxAllowedDist) {
        return 35000 - (editDist * 5000) - (std::abs(nLen - qLen) * 200);
    }

    // 8. Strict Subsequence Match with Gap Penalties
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
        int subseqScore = 20000 - (extraGap * 1200) - (lenDiff * 300) - (firstMatch * 200);
        if (subseqScore > 0) return subseqScore;
    }

    // 9. GenericName or Comment Prefix/Substring (1,000+)
    if (gLower.startsWith(qLower) || cLower.startsWith(qLower)) return 1000;
    if (gLower.contains(qLower) || cLower.contains(qLower)) return 500;

    return 0;
}

int FuzzyMatcher::score(int sourceRow) const {
    if (m_query.isEmpty()) return 1;
    
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

    int scoreOrig = matchSingleQuery(name, genericName, comment, exec, keywords, m_query);
    
    QString qwertTrans = convertRuToEn(m_query);
    int scoreQwerty = (qwertTrans != m_query) ? matchSingleQuery(name, genericName, comment, exec, keywords, qwertTrans) : 0;

    QString mnemonTrans = convertRuToEnMnemonic(m_query);
    int scoreMnemonic = (mnemonTrans != m_query && mnemonTrans != qwertTrans) ? matchSingleQuery(name, genericName, comment, exec, keywords, mnemonTrans) : 0;

    int finalScore = std::max({scoreOrig, scoreQwerty, scoreMnemonic});

    // Add Frecency Ranker bonus for frequently/recently launched apps
    if (finalScore > 0 && !desktopFile.isEmpty()) {
        int frecencyScore = static_cast<int>(FrecencyRanker::instance()->getScore(desktopFile));
        finalScore += frecencyScore * 100;
    }

    m_scoreCache[sourceRow] = finalScore;
    return finalScore;
}

QString FuzzyMatcher::formatHighlightedName(const QString &name, const QString &query) const {
    if (query.isEmpty()) return name.toHtmlEscaped();

    QString activeQuery = query;
    int bestScore = matchSingleQuery(name, QString(), QString(), QString(), QStringList(), activeQuery);
    
    QString qwertTrans = convertRuToEn(query);
    int sQ = (qwertTrans != query) ? matchSingleQuery(name, QString(), QString(), QString(), QStringList(), qwertTrans) : 0;
    if (sQ > bestScore) {
        bestScore = sQ;
        activeQuery = qwertTrans;
    }

    QString mnemonTrans = convertRuToEnMnemonic(query);
    int sM = (mnemonTrans != query) ? matchSingleQuery(name, QString(), QString(), QString(), QStringList(), mnemonTrans) : 0;
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

QVariant FuzzyMatcher::data(const QModelIndex &index, int role) const {
    if (role == ScoreRole) {
        return score(index.row());
    }
    if (role == HighlightedNameRole) {
        QModelIndex sourceIdx = mapToSource(index);
        QString name = sourceModel()->data(sourceIdx, m_nameRole).toString();
        return formatHighlightedName(name, m_query);
    }
    return QSortFilterProxyModel::data(index, role);
}

QHash<int, QByteArray> FuzzyMatcher::roleNames() const {
    QHash<int, QByteArray> roles = QSortFilterProxyModel::roleNames();
    roles[ScoreRole] = "score";
    roles[HighlightedNameRole] = "highlightedName";
    return roles;
}

bool FuzzyMatcher::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    Q_UNUSED(source_parent);
    if (m_query.isEmpty()) return true;
    return score(source_row) > 0;
}

bool FuzzyMatcher::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const {
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
    
    double frecencyL = m_frecencyRole != -1 ? sourceModel()->data(source_left, m_frecencyRole).toDouble() : 0.0;
    double frecencyR = m_frecencyRole != -1 ? sourceModel()->data(source_right, m_frecencyRole).toDouble() : 0.0;
    
    if (frecencyL != frecencyR) {
        return frecencyL > frecencyR;
    }
    
    QString nameLeft = m_nameRole != -1 ? sourceModel()->data(source_left, m_nameRole).toString() : QString();
    QString nameRight = m_nameRole != -1 ? sourceModel()->data(source_right, m_nameRole).toString() : QString();
    return nameLeft.compare(nameRight, Qt::CaseInsensitive) < 0;
}
