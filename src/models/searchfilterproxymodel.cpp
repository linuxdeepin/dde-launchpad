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

    // Check for number or English prefix matches only
    QRegularExpression searchNumberCheck("\\d+$");
    QRegularExpression searchEnglishCheck("^[a-zA-Z\\s]+$");
    bool isNumberSearch = searchNumberCheck.match(searchPatternDelBlank).hasMatch();
    bool isEnglishSearch = searchEnglishCheck.match(searchPattern.pattern()).hasMatch();

    if (isNumberSearch || isEnglishSearch) {
        bool hasMatch = false;

        // Handle number prefix matching
        if (isNumberSearch) {
            QRegularExpression numberRegex("\\d+");
            QRegularExpressionMatchIterator matches = numberRegex.globalMatch(displayName);

            while (matches.hasNext()) {
                QRegularExpressionMatch match = matches.next();
                QString numberInDisplayName = match.captured(0);
                hasMatch = true;
                if (numberInDisplayName.startsWith(searchPatternDelBlank)) {
                    return true;
                }
            }
        }

        // Handle English prefix matching
        if (isEnglishSearch) {
            // Remove spaces and convert to lowercase for comparison
            QString displayNameLower = displayName.toLower().remove(" ");

            // Check prefix matching for various name formats
            if (displayNameLower.startsWith(searchPatternDelBlank) ||
                nameCopy.startsWith(searchPatternDelBlank) ||
                transliterated.startsWith(searchPatternDelBlank) ||
                jianpin.startsWith(searchPatternDelBlank) ||
                nameFirstLetters.startsWith(searchPatternDelBlank)) {
                return true;
            }

            // Also check if search pattern matches the prefix of any word in displayName
            for (const QString &word : words) {
                if (word.toLower().startsWith(searchPatternDelBlank)) {
                    return true;
                }
            }

            // Also check if search pattern matches the prefix of any word in name
            QStringList nameWords = name.split(" ", Qt::SkipEmptyParts);
            for (const QString &word : nameWords) {
                if (word.toLower().startsWith(searchPatternDelBlank)) {
                    return true;
                }
            }

            // Also check if search pattern matches the prefix of any word in transliterated
            QStringList transliteratedWords = transliterated.split(" ", Qt::SkipEmptyParts);
            for (const QString &word : transliteratedWords) {
                if (word.toLower().startsWith(searchPatternDelBlank)) {
                    return true;
                }
            }

            hasMatch = true; // English content was found
        }

        // If we had matches but none were prefix matches, return false
        if (hasMatch) {
            return false;
        }
    }

    return displayName.contains(searchPatternDelBlank) ||
           nameCopy.contains(searchPatternDelBlank) ||
           transliterated.contains(searchPatternDelBlank) ||
           jianpin.contains(searchPatternDelBlank) ||
           nameFirstLetters.contains(searchPatternDelBlank);
}
