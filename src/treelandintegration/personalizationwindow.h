// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "qwayland-treeland-personalization-manager-v1.h"

#include <QtWaylandClient/QWaylandClientExtension>

class PersonalizationWindow : public QWaylandClientExtensionTemplate<PersonalizationWindow>,
                              public QtWayland::personalization_window_context_v1
{
    Q_OBJECT
public:
    explicit PersonalizationWindow(struct ::personalization_window_context_v1 *object);
};
