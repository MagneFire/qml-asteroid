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

#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QSettings>
#include <QQuickWindow>

#include "flatmesh.h"
#include "flatmeshgeometry.h"

// ============================================================================
// GLES 2.0 SHADERS
// ============================================================================

static const char *vertexShaderSource =
    "attribute vec4 coord;\n"
    "attribute vec4 color;\n"
    "uniform mat4 matrix;\n"
    "varying vec4 fragColor;\n"
    "void main()\n"
    "{\n"
        "gl_Position = matrix * vec4(coord.xy, 0.0, 1.0);\n"
        "fragColor = color;\n"
    "}\n";

static const char *fragmentShaderSource =
    "#ifdef GL_ES\n"
    "precision mediump float;\n"
    "#endif\n"
    "varying vec4 fragColor;\n"
    "void main()\n"
    "{\n"
        "gl_FragColor = fragColor;\n"
    "}\n";

static QByteArray versionedShaderCode(const char *src)
{
    return (QOpenGLContext::currentContext()->isOpenGLES()
            ? QByteArrayLiteral("#version 100\n")
            : QByteArrayLiteral("#version 120\n"))
              + src;
}

// ============================================================================
// MATERIAL SHADER CLASS
// ============================================================================

class SGFlatMeshMaterialShader : public QSGMaterialShader
{
public:
    SGFlatMeshMaterialShader() : m_matrix_id(-1) {}
    const char *vertexShader() const override {
        static QByteArray source = versionedShaderCode(vertexShaderSource);
        return source.constData();
    }
    const char *fragmentShader() const override {
        static QByteArray source = versionedShaderCode(fragmentShaderSource);
        return source.constData();
    }
    void updateState(const RenderState &state, QSGMaterial *newEffect, QSGMaterial *oldEffect) override {
        // On every run, update the animation state uniforms
        SGFlatMeshMaterial *material = static_cast<SGFlatMeshMaterial *>(newEffect);

        if (state.isMatrixDirty()) {
            // Vertices coordinates are always in the [-0.5, 0.5] range, modify QtQuick's projection matrix to do the scaling for us
            QMatrix4x4 combinedMatrix = state.combinedMatrix();
            combinedMatrix.scale(material->width(), material->height());
            combinedMatrix.translate(0.5, 0.5);
            combinedMatrix.scale(material->screenScaleFactor());
            program()->setUniformValue(m_matrix_id, combinedMatrix);
        }
    }
    char const *const *attributeNames() const override {
        // Map attribute numbers to attribute names in the vertex shader
        static const char *const attr[] = { "coord", "color", nullptr };
        return attr;
    }
    
    void initialize() override {
        m_matrix_id = program()->uniformLocation("matrix");
    }
    
private:
    int m_matrix_id;
};

QSGMaterialShader *SGFlatMeshMaterial::createShader() const
{
    return new SGFlatMeshMaterialShader;
}

// ============================================================================
// FLATMESH IMPLEMENTATION
// ============================================================================

FlatMesh::FlatMesh(QQuickItem *parent) 
    : QQuickItem(parent)
    , m_geometry(QSGGeometry::defaultAttributes_ColoredPoint2D(), 0, 0)
    , m_animated(false)
    , m_geometryDirty(false)
{
    setClip(true);

    QSettings machineConf("/etc/asteroid/machine.conf", QSettings::IniFormat);
    m_material.setScreenScaleFactor(machineConf.value("Display/ROUND", false).toBool() ? 1.2 : 1.7);

    // === CHANGED: Indices are now already independent triangles ===
    // No strip conversion needed - use indices directly with GL_TRIANGLES
    
    int totalVertices = flatmesh_indices_sz;  // Already 3 indices per triangle
    
    m_geometry.allocate(totalVertices, 0); // 0 indices, we use DrawArrays
    m_geometry.setDrawingMode(GL_TRIANGLES);
    m_geometry.setVertexDataPattern(QSGGeometry::DynamicPattern);

    // Store source indices for animation/color updates
    m_vertexSourceIndices.reserve(totalVertices);
    for (int i = 0; i < flatmesh_indices_sz; ++i) {
        m_vertexSourceIndices.append(flatmesh_indices[i]);
    }

    // Initialize base vertex positions
    QSGGeometry::ColoredPoint2D *vertices = m_geometry.vertexDataAsColoredPoint2D();
    for (int i = 0; i < totalVertices; i++) {
        unsigned short srcIdx = m_vertexSourceIndices[i];
        vertices[i].x = flatmesh_vertices[srcIdx].x();
        vertices[i].y = flatmesh_vertices[srcIdx].y();
        vertices[i].a = 255;
    }

    setColors(QColor("#ffaa39"), QColor("#df4829"));


    // m_animation interpolates the shiftMix, a float between 0.0 and 1.0
    // This is used by the vertex shader as the mix ratio between two shifts
    m_animation.setStartValue(0.0);
    m_animation.setEndValue(1.0);
    m_animation.setDuration(4000);
    m_animation.setLoopCount(-1);
    m_animation.setEasingCurve(QEasingCurve::InOutQuad);

    QObject::connect(&m_animation, &QVariantAnimation::valueChanged, [this](const QVariant& value) {
        Q_UNUSED(value);
        updateGeometry();
        update();
    });

    // Run m_animation depending on the item's visibility
    connect(this, SIGNAL(visibleChanged()), this, SLOT(maybeEnableAnimation()));
    setAnimated(true);

    // Tell QtQuick we have graphic content and that updatePaintNode() needs to run
    setFlag(ItemHasContents);
}

void FlatMesh::updateColors()
{
    QSGGeometry::ColoredPoint2D *vertices = m_geometry.vertexDataAsColoredPoint2D();
    int totalTriangles = m_vertexSourceIndices.size() / 3;

    for (int i = 0; i < totalTriangles; ++i) {
        // All 3 vertices of a triangle have the same mix (stored in Z of first vertex)
        int firstVertexIdx = i * 3;
        unsigned short srcIdx = m_vertexSourceIndices[firstVertexIdx];
        float ratio = flatmesh_vertices[srcIdx].z();
        float ir = 1.0f - ratio;
        
        uchar r = static_cast<uchar>(m_centerColor.red() * ir + m_outerColor.red() * ratio);
        uchar g = static_cast<uchar>(m_centerColor.green() * ir + m_outerColor.green() * ratio);
        uchar b = static_cast<uchar>(m_centerColor.blue() * ir + m_outerColor.blue() * ratio);

        // Apply the same color to all 3 vertices of this triangle
        for (int j = 0; j < 3; ++j) {
            int vertIdx = i * 3 + j;
            vertices[vertIdx].r = r;
            vertices[vertIdx].g = g;
            vertices[vertIdx].b = b;
        }
    }

    m_geometryDirty = true;
}

void FlatMesh::updateGeometry()
{
    QSGGeometry::ColoredPoint2D *vertices = m_geometry.vertexDataAsColoredPoint2D();
    int totalVertices = m_vertexSourceIndices.size();

    int loopNb = m_animation.currentLoop();
    float shiftMix = m_animation.currentValue().toFloat();

    for (int i = 0; i < totalVertices; ++i) {
        // Look up the original source vertex data
        unsigned short srcIdx = m_vertexSourceIndices[i];
        
        float baseX = flatmesh_vertices[srcIdx].x();
        float baseY = flatmesh_vertices[srcIdx].y();

        // Calculate Shift
        int xHash = static_cast<int>(baseX * 100.0f);
        int yHash = static_cast<int>(baseY * 100.0f);
        int shiftIndex = loopNb + xHash + yHash;

        int idxA = (shiftIndex % flatmesh_shifts_nb + flatmesh_shifts_nb) % flatmesh_shifts_nb;
        int idxB = ((shiftIndex + 1) % flatmesh_shifts_nb + flatmesh_shifts_nb) % flatmesh_shifts_nb;

        float shiftX = flatmesh_shifts[idxA * 2] + (flatmesh_shifts[idxB * 2] - flatmesh_shifts[idxA * 2]) * shiftMix;
        float shiftY = flatmesh_shifts[idxA * 2 + 1] + (flatmesh_shifts[idxB * 2 + 1] - flatmesh_shifts[idxA * 2 + 1]) * shiftMix;

        vertices[i].x = baseX + shiftX;
        vertices[i].y = baseY + shiftY;
    }

    m_geometry.markVertexDataDirty();
    m_geometryDirty = true;
}

void FlatMesh::setColors(QColor center, QColor outer)
{
    if (center == m_centerColor && outer == m_outerColor)
        return;
    m_centerColor = center;
    m_outerColor = outer;
    updateColors();
    update();
}

void FlatMesh::setCenterColor(QColor c)
{
    setColors(c, m_outerColor);
}

void FlatMesh::setOuterColor(QColor c)
{
    setColors(m_centerColor, c);
}

void FlatMesh::maybeEnableAnimation()
{
    // Only run the animation if the item is visible. No point running the shaders if this is hidden
    if (isVisible() && m_animated)
        m_animation.start();
    else
        m_animation.pause();
}

void FlatMesh::setAnimated(bool animated)
{
    if (animated == m_animated)
        return;
    m_animated = animated;
    emit animatedChanged();
    maybeEnableAnimation();
}

void FlatMesh::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    // On resizes, tell the vertex shader about the new size so the transformation matrix compensates it
    m_material.setSize(newGeometry.width(), newGeometry.height());

    QQuickItem::geometryChanged(newGeometry, oldGeometry);
}

// Called by the SceneGraph on every update()
QSGNode *FlatMesh::updatePaintNode(QSGNode *old, UpdatePaintNodeData *)
{
    // On the first update(), create a scene graph node for the mesh
    QSGGeometryNode *n = static_cast<QSGGeometryNode *>(old);
    if (!n) {
        n = new QSGGeometryNode;
        n->setOpaqueMaterial(&m_material);
        n->setGeometry(&m_geometry);
        updateGeometry();
        updateColors();
    }

    // On every update(), mark the material dirty so the shaders run again
    n->markDirty(QSGNode::DirtyMaterial);
    // And if colors changed, mark the geometry dirty so the new vertex attributes are sent to the GPU
    if (m_geometryDirty) {
        n->markDirty(QSGNode::DirtyGeometry);
        m_geometryDirty = false;
    }

    return n;
}
