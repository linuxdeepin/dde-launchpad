// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appsmodel.h"
#include "searchfilterproxymodel.h"

#include <QDebug>
#include <DPinyin>
#include <QMap>
#include <functional>
#include <algorithm>
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

    // 计算匹配索引
    int matchIndex = calculateWeight(modelIndex);

    // 如果索引为0，表示不匹配
    return matchIndex >= 0;
}

bool SearchFilterProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    int leftIndex = calculateWeight(source_left);
    int rightIndex = calculateWeight(source_right);

    if (leftIndex != rightIndex) {
        // 索引值越小优先级越高，在降序排序中应该排在前面
        return leftIndex > rightIndex; // 索引小的返回false（排在前面）
    }

    // 索引相同时，按启动次数排序：高频使用 > 低频使用
    int leftLaunchedTimes = source_left.data(AppItem::LaunchedTimesRole).toInt();
    int rightLaunchedTimes = source_right.data(AppItem::LaunchedTimesRole).toInt();

    if (leftLaunchedTimes != rightLaunchedTimes) {
        bool result = leftLaunchedTimes < rightLaunchedTimes;
        return result;
    }

    // 索引和启动次数都相同时，按照原有的排序规则
    return QSortFilterProxyModel::lessThan(source_left, source_right);
}

int SearchFilterProxyModel::calculateWeight(const QModelIndex &modelIndex) const
{
    const QRegularExpression searchPattern = this->filterRegularExpression();
    if (searchPattern.pattern().isEmpty()) {
        return 0;
    }

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

    QRegularExpression searchEnglishCheck("^[a-zA-Z0-9\\s\\-\\.]+$");
    bool isEnglishSearch = searchEnglishCheck.match(searchPattern.pattern()).hasMatch();

    // 计算匹配权重
    QString searchPatternLower = searchPatternDelBlank.toLower();
    QString displayNameLower = displayName.toLower().remove(" ");
    QString targetNameLower = targetName.toLower().remove(" ");
    QString transliteratedLower = transliterated.toLower();
    QString jianpinLower = jianpin.toLower();
    QString nameFirstLettersLower = nameFirstLetters.toLower();

    // 使用 QVector 存储匹配类型和对应的函数，按优先级顺序插入
    QVector<QPair<QString, std::function<bool()>>> matchTypes;

    // 完全匹配
    matchTypes.push_back(qMakePair(QString("displayName_exact"), [&]() -> bool {
        return (displayNameLower == searchPatternLower);
    }));

    matchTypes.push_back(qMakePair(QString("targetName_exact"), [&]() -> bool {
        return (targetNameLower == searchPatternLower);
    }));

    // 中文拼音匹配
    matchTypes.push_back(qMakePair(QString("transliterated_start"), [&]() -> bool {
        return transliteratedLower.startsWith(searchPatternLower);
    }));

    matchTypes.push_back(qMakePair(QString("jianpin_exact"), [&]() -> bool {
        QString jianpinNormalized = QString(jianpinLower).remove(",").remove(" ");
        return (jianpinNormalized == searchPatternLower);
    }));

    matchTypes.push_back(qMakePair(QString("jianpin_start"), [&]() -> bool {
        QString jianpinNormalized = QString(jianpinLower).remove(",").remove(" ");
        return jianpinNormalized.startsWith(searchPatternLower);
    }));

    // 检查是否为中文应用（不以英文字母开头）
    matchTypes.push_back(qMakePair(QString("displayName_start_chinese"), [&]() -> bool {
        if (!displayNameLower.startsWith(searchPatternLower)) return false;
        QRegularExpression startsWithEnglishCheck("^[a-zA-Z][a-zA-Z0-9]*");
        return !startsWithEnglishCheck.match(displayName).hasMatch();
    }));

    // 英文应用的 displayName 开头匹配
    matchTypes.push_back(qMakePair(QString("displayName_start_english"), [&]() -> bool {
        if (!displayNameLower.startsWith(searchPatternLower)) return false;
        QRegularExpression startsWithEnglishCheck("^[a-zA-Z][a-zA-Z0-9]*");
        return startsWithEnglishCheck.match(displayName).hasMatch();
    }));

    matchTypes.push_back(qMakePair(QString("displayName_word_start"), [&]() -> bool {
        if (displayNameLower.contains(searchPatternLower)) {
            QStringList displayWords = displayName.split(" ", Qt::SkipEmptyParts);
            for (const QString &word : displayWords) {
                if (word.toLower().startsWith(searchPatternLower)) {
                    return true;
                }
            }
        }
        return false;
    }));

    matchTypes.push_back(qMakePair(QString("nameFirstLetters_start"), [&]() -> bool {
        return nameFirstLettersLower.startsWith(searchPatternLower);
    }));

    matchTypes.push_back(qMakePair(QString("targetName_start"), [&]() -> bool {
        return targetNameLower.startsWith(searchPatternLower);
    }));

    matchTypes.push_back(qMakePair(QString("targetName_word_start"), [&]() -> bool {
        if (targetNameLower.contains(searchPatternLower)) {
            QStringList words = targetName.split(" ", Qt::SkipEmptyParts);
            for (const QString &word : words) {
                if (word.toLower().startsWith(searchPatternLower)) {
                    return true;
                }
            }
        }
        return false;
    }));

    matchTypes.push_back(qMakePair(QString("displayName_middle"), [&]() -> bool {
        if (displayNameLower.contains(searchPatternLower)) {
            QStringList displayWords = displayName.split(" ", Qt::SkipEmptyParts);
            bool isDisplayWordStart = false;
            for (const QString &word : displayWords) {
                if (word.toLower().startsWith(searchPatternLower)) {
                    isDisplayWordStart = true;
                    break;
                }
            }
            if (!isDisplayWordStart) {
                return true;
            }
        }
        return false;
    }));

    matchTypes.push_back(qMakePair(QString("transliterated_word_start"), [&]() -> bool {
        if (transliteratedLower.contains(searchPatternLower)) {
            QStringList transliteratedWords = transliterated.split(" ", Qt::SkipEmptyParts);
            for (const QString &word : transliteratedWords) {
                if (word.toLower().startsWith(searchPatternLower)) {
                    return true;
                }
            }
        }
        return false;
    }));

    matchTypes.push_back(qMakePair(QString("targetName_middle"), [&]() -> bool {
        return (targetNameLower.contains(searchPatternLower));
    }));

    matchTypes.push_back(qMakePair(QString("transliterated_middle"), [&]() -> bool {
        return transliteratedLower.contains(searchPatternLower);
    }));

    matchTypes.push_back(qMakePair(QString("nameFirstLetters_middle"), [&]() -> bool {
        return nameFirstLettersLower.contains(searchPatternLower);
    }));

    matchTypes.push_back(qMakePair(QString("jianpin_middle"), [&]() -> bool {
        QString jianpinNormalized = QString(jianpinLower).remove(",").remove(" ");
        return jianpinNormalized.contains(searchPatternLower);
    }));

    // 英文搜索特殊情况处理
    auto getCapitalizedWords = [&]() -> QString {
        QRegularExpression capitalizedWordRegex("\\b[A-Z][A-Za-z0-9]*");
        QStringList capitalizedWords;

        auto matches = capitalizedWordRegex.globalMatch(targetName);
        while (matches.hasNext()) {
            capitalizedWords << matches.next().captured(0).toLower();
        }

        return capitalizedWords.join(" ");
    };

    matchTypes.push_back(qMakePair(QString("capitalized_word_start"), [&]() -> bool {
        if (!isEnglishSearch) return false;
        return getCapitalizedWords().startsWith(searchPatternLower);
    }));

    matchTypes.push_back(qMakePair(QString("capitalized_word_middle"), [&]() -> bool {
        if (!isEnglishSearch) return false;
        return getCapitalizedWords().contains(searchPatternLower);
    }));

    // 计算匹配索引（索引越小优先级越高)
    auto it = std::find_if(matchTypes.begin(), matchTypes.end(),
                          [](const auto& matchType) { return matchType.second(); });
                          
    // 如果没有匹配，返回-1表示不匹配
    if (it == matchTypes.end())
        return -1;
        
    const int matchIndex = std::distance(matchTypes.begin(), it);

    // 返回索引值+1，确保返回值大于0（0表示不匹配）
    return matchIndex;
}
