// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appsmodel.h"
#include "searchfilterproxymodel.h"

#include <QDebug>
#include <DPinyin>
DCORE_USE_NAMESPACE

SearchFilterProxyModel::SearchFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);

    setSourceModel(&AppsModel::instance());
    sort(0, Qt::DescendingOrder);
}

bool SearchFilterProxyModel::fuzzyMatch(const QString &modelData, const QString &pattern) const
{
    if (modelData.contains(pattern, Qt::CaseInsensitive)) {
        return true;
    }
    QString processedText = modelData.toLower().simplified();
    int textLen = processedText.length();
    int patternLen = pattern.length();
    std::vector<std::vector<int>> dp(textLen + 1, std::vector<int>(patternLen + 1, 0));
    for (int i = 1; i <= textLen; i++) {
        for (int j = 1; j <= patternLen; j++) {
            if (processedText[i - 1] == pattern[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            } else {
                dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
            }
        }
    }

    float matchScore = static_cast<float>(dp[textLen][patternLen]) / patternLen;
    return matchScore >= m_fuzzyThreshold;
}

bool SearchFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex modelIndex = this->sourceModel()->index(sourceRow, 0, sourceParent);
    const QRegularExpression searchPattern = this->filterRegularExpression();

    const QString & displayName = modelIndex.data(Qt::DisplayRole).toString();
    const QString & name = modelIndex.data(AppsModel::NameRole).toString();
    const QString & transliterated = modelIndex.data(AppsModel::AllTransliteratedRole).toString();

    QString pattern = searchPattern.pattern().toLower().remove(" ");

    return fuzzyMatch(displayName, pattern) || fuzzyMatch(name, pattern) || fuzzyMatch(transliterated, pattern);
}
