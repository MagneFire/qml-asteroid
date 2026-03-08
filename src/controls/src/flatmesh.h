/*
 * Copyright (C) 2016 Florent Revest <revestflo@gmail.com>
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

#ifndef FLATMESH_H
#define FLATMESH_H

#include <QQuickItem>
#include <QSGGeometryNode>
#include <QSGMaterial>
#include <QVariantAnimation>
#include <QColor>
#include <QVector>

class SGFlatMeshMaterial : public QSGMaterial
{
public:
    SGFlatMeshMaterial() : m_width(1.0f), m_height(1.0f), m_screenScale(1.0f) {}
    
    QSGMaterialShader *createShader() const override;
    int compare(const QSGMaterial *other) const override { return 0; }
    QSGMaterialType *type() const override { static QSGMaterialType type; return &type; }
    
    void setSize(float w, float h) { m_width = w; m_height = h; }
    float width() const { return m_width; }
    float height() const { return m_height; }
    
    void setScreenScaleFactor(float s) { m_screenScale = s; }
    float screenScaleFactor() const { return m_screenScale; }
    
private:
    float m_width;
    float m_height;
    float m_screenScale;
};

class FlatMesh : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QColor centerColor WRITE setCenterColor READ getCenterColor)
    Q_PROPERTY(QColor outerColor WRITE setOuterColor READ getOuterColor)
    Q_PROPERTY(bool animated WRITE setAnimated READ getAnimated NOTIFY animatedChanged)

public:
    FlatMesh(QQuickItem *parent = 0);

    bool getAnimated() const { return m_animated; }
    void setAnimated(bool animated);

    QColor getCenterColor() const { return m_centerColor; }
    void setCenterColor(QColor c);

    QColor getOuterColor() const { return m_outerColor; }
    void setOuterColor(QColor c);
    
protected:
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    QSGNode *updatePaintNode(QSGNode *old, UpdatePaintNodeData *) override;
    
signals:
    void animatedChanged();
    
private slots:
    void maybeEnableAnimation();
    
private:
    void updateColors();
    void updateGeometry();
    void setColors(QColor center, QColor outer);

    QSGGeometry m_geometry;
    SGFlatMeshMaterial m_material;
    QVariantAnimation m_animation;
    QColor m_centerColor;
    QColor m_outerColor;
    bool m_animated;
    bool m_geometryDirty;
    
    // Stores the index into flatmesh_vertices for each expanded vertex in m_geometry
    // This allows us to update positions/animations correctly for unshared vertices
    QVector<unsigned short> m_vertexSourceIndices;
};

#endif // FLATMESH_H