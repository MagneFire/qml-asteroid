/*
 * Copyright (C) 2015 Florent Revest <revestflo@gmail.com>
 * All rights reserved.
 *
 * You may use this file under the terms of BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the author nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef APPLICATION_P_H
#define APPLICATION_P_H

#include <QQuickItem>
#include <QQuickWindow>
#include <QColor>

class Application_p : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QQuickWindow* window READ window CONSTANT)
    Q_PROPERTY(bool overridesSystemGestures READ overridesSystemGestures WRITE setOverridesSystemGestures NOTIFY overridesSystemGesturesChanged)
    Q_PROPERTY(QColor bgCenterColor READ getCenterColor NOTIFY centerColorChanged)
    Q_PROPERTY(QColor bgOuterColor READ getOuterColor NOTIFY outerColorChanged)

public:
    explicit Application_p();
    Q_INVOKABLE void setOverridesSystemGestures(bool enable);
    bool overridesSystemGestures();

    QColor getCenterColor() const { return m_centerColor; }
    QColor getOuterColor() const { return m_outerColor; }
private:
    bool m_overridesSystemGestures;
    QColor m_centerColor, m_outerColor;

signals:
    void overridesSystemGesturesChanged();
    void centerColorChanged();
    void outerColorChanged();
};

#endif // APPLICATION_P_H
