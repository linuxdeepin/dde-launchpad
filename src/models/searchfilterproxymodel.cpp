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

bool SearchFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex modelIndex = this->sourceModel()->index(sourceRow, 0, sourceParent);
    const QRegularExpression searchPattern = this->filterRegularExpression();

    const QString & displayName = modelIndex.data(Qt::DisplayRole).toString();
    const QString & name = modelIndex.data(AppsModel::NameRole).toString();
    const QString & transliterated = modelIndex.data(AppsModel::AllTransliteratedRole).toString();
    const QString & jianpin = Dtk::Core::firstLetters(displayName).join(',');

    auto nameCopy = name;
    nameCopy = nameCopy.toLower();
    nameCopy.replace(" ", "");

    QString searchPatternDelBlank = searchPattern.pattern().toLower().remove(" ");

    // Get first letters of each word in displayName
    QStringList words = displayName.split(" ", Qt::SkipEmptyParts);
    QString nameFirstLetters;
    for (const QString &word : words) {
        if (!word.isEmpty()) {
            QChar firstChar = word[0];
            if (firstChar.isLetter()) {
                nameFirstLetters += firstChar.toLower();
            }
        }
    }

    // Only check for number matches if search pattern contains numbers
    QRegularExpression searchNumberCheck("\\d+$");
    if (searchNumberCheck.match(searchPatternDelBlank).hasMatch()) {
        QRegularExpression numberRegex("\\d+");
        QRegularExpressionMatchIterator matches = numberRegex.globalMatch(displayName);
        bool hasNumberMatch = false;

        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            QString numberInDisplayName = match.captured(0);
            hasNumberMatch = true;
            if (numberInDisplayName.startsWith(searchPatternDelBlank)) {
                return true;
            }
        }
        
        // If there are numbers but none match the pattern
        if (hasNumberMatch) {
            return false;
        }
    }

    // Check if displayName contains only English letters
    QRegularExpression englishOnly("^[a-z\\s]+$");
    bool isPureEnglish = englishOnly.match(displayName).hasMatch();
    if (isPureEnglish) {
        return displayName.startsWith(searchPatternDelBlank) || 
               nameCopy.startsWith(searchPatternDelBlank) || 
               transliterated.startsWith(searchPatternDelBlank) || 
               jianpin.startsWith(searchPatternDelBlank) ||
               nameFirstLetters.startsWith(searchPatternDelBlank);
    }

    return displayName.contains(searchPatternDelBlank) || 
           nameCopy.contains(searchPatternDelBlank) || 
           transliterated.contains(searchPatternDelBlank) || 
           jianpin.contains(searchPatternDelBlank) ||
           nameFirstLetters.contains(searchPatternDelBlank);
}
