// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPixmap>

namespace IconUtils {
bool getThemeIcon(QPixmap &pixmap, const QString & iconName, const int size);
void tryUpdateIconCache();
int perfectIconSize(const int size);
bool createCalendarIcon(const QString &fileName);
const QPixmap loadSvg(const QString &fileName, int size);
const QPixmap loadSvg(const QString &fileName, const QSize &size);
}
