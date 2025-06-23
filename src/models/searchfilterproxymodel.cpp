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
    const QString & vendor = modelIndex.data(AppItem::VendorRole).toString();
    const QString & genericName = modelIndex.data(AppItem::GenericNameRole).toString();
    const QString & transliterated = modelIndex.data(AppsModel::AllTransliteratedRole).toString();
    const QString & jianpin = Dtk::Core::firstLetters(displayName).join(',');

    QString searchPatternDelBlank = searchPattern.pattern().toLower().remove(" ");

    // Choose which name to use for matching based on search input and vendor
    QString targetName;
    if(vendor == "deepin") {
        targetName = genericName;
        if(targetName.isEmpty()) {
            targetName = name; 
        }
    }else{
        targetName = name;
    }

    // Get first letters of each word in targetName  eg: visual studio code -> vsc
    QStringList words = targetName.split(" ", Qt::SkipEmptyParts);
    QString nameFirstLetters;
    for (const QString &word : words) {
        if (!word.isEmpty()) {
            QChar firstChar = word[0];
            if (firstChar.isLetter()) {
                nameFirstLetters += firstChar.toLower();
            }
        }
    }

    QRegularExpression searchNumberCheck("^\\d+$");
    bool isNumberSearch = searchNumberCheck.match(searchPatternDelBlank).hasMatch();

    QRegularExpression searchEnglishCheck("^[a-zA-Z0-9\\s]+$");
    bool isEnglishSearch = searchEnglishCheck.match(searchPattern.pattern()).hasMatch();

    QRegularExpression searchHasLetterCheck("[a-zA-Z]");
    bool hasLetter = searchHasLetterCheck.match(searchPattern.pattern()).hasMatch();

    if (isNumberSearch || isEnglishSearch) {
        bool hasMatch = false;
        // Handle number prefix matching  eg: 360zip
        if (isNumberSearch) {
            QRegularExpression numberRegex("\\d+");
            QRegularExpressionMatchIterator matches = numberRegex.globalMatch(targetName);

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
            QString targetNameLower = targetName.toLower().remove(" ");

            // Extract all capitalized words from displayName (both at beginning and middle)
            //eg: x11Vnc Server -> VNC
            QString targetNameUpper;
            QRegularExpression capitalizedWordRegex("\\b[A-Z][A-Za-z0-9]*");
            QRegularExpressionMatchIterator capitalizedMatches = capitalizedWordRegex.globalMatch(targetName);
            
            while (capitalizedMatches.hasNext()) {
                QRegularExpressionMatch match = capitalizedMatches.next();
                QString capitalizedWord = match.captured(0).toLower();
                if (!targetNameUpper.isEmpty()) {
                    targetNameUpper += " ";
                }
                targetNameUpper += capitalizedWord;
            }
            
            
            // Check prefix matching for various name formats
            if (
                displayName.startsWith(searchPatternDelBlank) ||
                targetNameLower.startsWith(searchPatternDelBlank) ||
                transliterated.startsWith(searchPatternDelBlank) ||
                jianpin.startsWith(searchPatternDelBlank) ||
                targetNameUpper.startsWith(searchPatternDelBlank) ||
                nameFirstLetters.startsWith(searchPatternDelBlank)) {
                return true;
            }

            // Also check if search pattern matches the prefix of any word in targetName
            for (const QString &word : words) {
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

            // For English searches with letters, if prefix matching fails, fall back to contains matching 
            if (hasLetter && (displayName.contains(searchPatternDelBlank) ||
                targetNameLower.contains(searchPatternDelBlank) ||
                transliterated.contains(searchPatternDelBlank) ||
                jianpin.contains(searchPatternDelBlank) ||
                nameFirstLetters.contains(searchPatternDelBlank))) {
                return true;
            }

            hasMatch = true; 
        }

        // If we had number matches but none were prefix matches, return false
        if (hasMatch && isNumberSearch) {
            return false;
        }
    }

    return displayName.contains(searchPatternDelBlank) ||
           targetName.contains(searchPatternDelBlank) ||
           transliterated.contains(searchPatternDelBlank) ||
           jianpin.contains(searchPatternDelBlank) ||
           nameFirstLetters.contains(searchPatternDelBlank);
}
