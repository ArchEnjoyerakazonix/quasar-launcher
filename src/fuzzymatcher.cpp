#include "fuzzymatcher.h"
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
        else if (it.value() == "categories") m_categoriesRole = it.key();
        else if (it.value() == "keywords") m_keywordsRole = it.key();
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

int FuzzyMatcher::score(int sourceRow) const {
    if (m_query.isEmpty()) return 1;
    
    if (m_scoreCache.count(sourceRow)) {
        return m_scoreCache[sourceRow];
    }
    
    QModelIndex idx = sourceModel()->index(sourceRow, 0);
    
    QString name = m_nameRole != -1 ? idx.data(m_nameRole).toString() : QString();
    QString genericName = m_genericNameRole != -1 ? idx.data(m_genericNameRole).toString() : QString();
    QString comment = m_commentRole != -1 ? idx.data(m_commentRole).toString() : QString();

    if (name.isEmpty() && genericName.isEmpty()) {
        m_scoreCache[sourceRow] = 0;
        return 0;
    }

    QString qLower = m_query.toLower();
    QString nLower = name.toLower();

    int finalScore = 0;

    // 1. Direct Name Prefix Match (HIGHEST PRIORITY: 100,000+)
    if (nLower.startsWith(qLower)) {
        finalScore = 100000 + (1000 - name.length());
    }
    // 2. Word Boundary Prefix Match in Name (e.g. "Theme Selector" matches "Sel" -> 50,000+)
    else {
        QStringList words = nLower.split(QRegularExpression("[\\s\\-_]+"));
        bool wordMatch = false;
        for (int i = 1; i < words.size(); ++i) {
            if (words[i].startsWith(qLower)) {
                finalScore = 50000 + (1000 - name.length()) - (i * 10);
                wordMatch = true;
                break;
            }
        }
        
        if (!wordMatch) {
            // 3. Exact Substring Match in Name (e.g. "Chromium" for "ro" -> 10,000+)
            int pos = nLower.indexOf(qLower);
            if (pos != -1) {
                finalScore = 10000 + (1000 - pos * 10 - name.length());
            }
            // 4. Prefix Match in GenericName or Comment (5,000+)
            else if (genericName.toLower().startsWith(qLower) || comment.toLower().startsWith(qLower)) {
                finalScore = 5000;
            }
            // 5. Substring Match in GenericName or Comment (2,000+)
            else if (genericName.toLower().contains(qLower) || comment.toLower().contains(qLower)) {
                finalScore = 2000;
            }
            // 6. Subsequence Fuzzy Match
            else {
                int s = calculateScore(name, m_query);
                if (s > 0) finalScore = s;
            }
        }
    }
    
    m_scoreCache[sourceRow] = finalScore;
    return finalScore;
}

QString FuzzyMatcher::formatHighlightedName(const QString &name, const QString &query) const {
    if (query.isEmpty()) return name;

    QString qLower = query.toLower();
    QString nLower = name.toLower();

    int pos = nLower.indexOf(qLower);
    if (pos != -1) {
        QString before = name.left(pos);
        QString matched = name.mid(pos, query.length());
        QString after = name.mid(pos + query.length());
        return QString("%1<u><font color=\"#8aadf4\">%2</font></u>%3").arg(before, matched, after);
    }

    QString result;
    int qIdx = 0;
    int qLen = query.length();
    for (int i = 0; i < name.length(); ++i) {
        if (qIdx < qLen && name[i].toLower() == query[qIdx].toLower()) {
            result += QString("<u><font color=\"#8aadf4\">%1</font></u>").arg(name[i]);
            qIdx++;
        } else {
            result += name[i];
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
