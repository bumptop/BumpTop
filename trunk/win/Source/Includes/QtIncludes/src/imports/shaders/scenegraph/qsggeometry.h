/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QML Shaders plugin of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSGGEOMETRY_H
#define QSGGEOMETRY_H

#include <QtOpenGL/qgl.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QSGGeometry
{
public:
    struct Attribute
    {
        int position;
        int tupleSize;
        int type;
    };

    struct AttributeSet {
        int count;
        int stride;
        const Attribute *attributes;
    };

    struct Point2D { float x, y; };
    struct TexturedPoint2D { float x, y; float tx, ty; };
    struct ColoredPoint2D { float x, y; unsigned char r, g, b, a; };

    static const AttributeSet &defaultAttributes_Point2D();
    static const AttributeSet &defaultAttributes_TexturedPoint2D();
    static const AttributeSet &defaultAttributes_ColoredPoint2D();

    QSGGeometry(const QSGGeometry::AttributeSet &attribs,
                int vertexCount,
                int indexCount = 0,
                int indexType = GL_UNSIGNED_SHORT);
    ~QSGGeometry();

    void setDrawingMode(GLenum mode);
    inline GLenum drawingMode() const { return m_drawing_mode; }

    void allocate(int vertexCount, int indexCount = 0);

    int vertexCount() const { return m_vertex_count; }

    void *vertexData() { return m_data; }
    inline Point2D *vertexDataAsPoint2D();
    inline TexturedPoint2D *vertexDataAsTexturedPoint2D();
    inline ColoredPoint2D *vertexDataAsColoredPoint2D();

    inline const void *vertexData() const { return m_data; }
    inline const Point2D *vertexDataAsPoint2D() const;
    inline const TexturedPoint2D *vertexDataAsTexturedPoint2D() const;
    inline const ColoredPoint2D *vertexDataAsColoredPoint2D() const;

    inline int indexType() const { return m_index_type; }

    int indexCount() const { return m_index_count; }

    void *indexData();
    inline uint *indexDataAsUInt();
    inline quint16 *indexDataAsUShort();

    const void *indexData() const;
    inline const uint *indexDataAsUInt() const;
    inline const quint16 *indexDataAsUShort() const;

    inline int attributeCount() const { return m_attributes.count; }
    inline const Attribute *attributes() const { return m_attributes.attributes; }
    inline int stride() const { return m_attributes.stride; }

    static void updateRectGeometry(QSGGeometry *g, const QRectF &rect);
    static void updateTexturedRectGeometry(QSGGeometry *g, const QRectF &rect, const QRectF &sourceRect);

private:
    int m_drawing_mode;
    int m_vertex_count;
    int m_index_count;
    int m_index_type;
    const AttributeSet &m_attributes;
    void *m_data;
    int m_index_data_offset;

    void *m_reserved_pointer;

    uint m_owns_data : 1;
    uint m_reserved_bits : 31;

    float m_prealloc[16];
};

inline uint *QSGGeometry::indexDataAsUInt()
{
    Q_ASSERT(m_index_type == GL_UNSIGNED_INT);
    return (uint *) indexData();
}

inline quint16 *QSGGeometry::indexDataAsUShort()
{
    Q_ASSERT(m_index_type == GL_UNSIGNED_SHORT);
    return (quint16 *) indexData();
}

inline const uint *QSGGeometry::indexDataAsUInt() const
{
    Q_ASSERT(m_index_type == GL_UNSIGNED_INT);
    return (uint *) indexData();
}

inline const quint16 *QSGGeometry::indexDataAsUShort() const
{
    Q_ASSERT(m_index_type == GL_UNSIGNED_SHORT);
    return (quint16 *) indexData();
}

inline QSGGeometry::Point2D *QSGGeometry::vertexDataAsPoint2D()
{
    Q_ASSERT(m_attributes.count == 1);
    Q_ASSERT(m_attributes.stride == 2 * sizeof(float));
    Q_ASSERT(m_attributes.attributes[0].tupleSize == 2);
    Q_ASSERT(m_attributes.attributes[0].type == GL_FLOAT);
    Q_ASSERT(m_attributes.attributes[0].position == 0);
    return (Point2D *) m_data;
}

inline QSGGeometry::TexturedPoint2D *QSGGeometry::vertexDataAsTexturedPoint2D()
{
    Q_ASSERT(m_attributes.count == 2);
    Q_ASSERT(m_attributes.stride == 4 * sizeof(float));
    Q_ASSERT(m_attributes.attributes[0].position == 0);
    Q_ASSERT(m_attributes.attributes[0].tupleSize == 2);
    Q_ASSERT(m_attributes.attributes[0].type == GL_FLOAT);
    Q_ASSERT(m_attributes.attributes[1].position == 1);
    Q_ASSERT(m_attributes.attributes[1].tupleSize == 2);
    Q_ASSERT(m_attributes.attributes[1].type == GL_FLOAT);
    return (TexturedPoint2D *) m_data;
}

inline QSGGeometry::ColoredPoint2D *QSGGeometry::vertexDataAsColoredPoint2D()
{
    Q_ASSERT(m_attributes.count == 2);
    Q_ASSERT(m_attributes.stride == 2 * sizeof(float) + 4 * sizeof(char));
    Q_ASSERT(m_attributes.attributes[0].position == 0);
    Q_ASSERT(m_attributes.attributes[0].tupleSize == 2);
    Q_ASSERT(m_attributes.attributes[0].type == GL_FLOAT);
    Q_ASSERT(m_attributes.attributes[1].position == 1);
    Q_ASSERT(m_attributes.attributes[1].tupleSize == 4);
    Q_ASSERT(m_attributes.attributes[1].type == GL_UNSIGNED_BYTE);
    return (ColoredPoint2D *) m_data;
}

inline const QSGGeometry::Point2D *QSGGeometry::vertexDataAsPoint2D() const
{
    Q_ASSERT(m_attributes.count == 1);
    Q_ASSERT(m_attributes.stride == 2 * sizeof(float));
    Q_ASSERT(m_attributes.attributes[0].tupleSize == 2);
    Q_ASSERT(m_attributes.attributes[0].type == GL_FLOAT);
    Q_ASSERT(m_attributes.attributes[0].position == 0);
    return (const Point2D *) m_data;
}

inline const QSGGeometry::TexturedPoint2D *QSGGeometry::vertexDataAsTexturedPoint2D() const
{
    Q_ASSERT(m_attributes.count == 2);
    Q_ASSERT(m_attributes.stride == 4 * sizeof(float));
    Q_ASSERT(m_attributes.attributes[0].position == 0);
    Q_ASSERT(m_attributes.attributes[0].tupleSize == 2);
    Q_ASSERT(m_attributes.attributes[0].type == GL_FLOAT);
    Q_ASSERT(m_attributes.attributes[1].position == 1);
    Q_ASSERT(m_attributes.attributes[1].tupleSize == 2);
    Q_ASSERT(m_attributes.attributes[1].type == GL_FLOAT);
    return (const TexturedPoint2D *) m_data;
}

inline const QSGGeometry::ColoredPoint2D *QSGGeometry::vertexDataAsColoredPoint2D() const
{
    Q_ASSERT(m_attributes.count == 2);
    Q_ASSERT(m_attributes.stride == 2 * sizeof(float) + 4 * sizeof(char));
    Q_ASSERT(m_attributes.attributes[0].position == 0);
    Q_ASSERT(m_attributes.attributes[0].tupleSize == 2);
    Q_ASSERT(m_attributes.attributes[0].type == GL_FLOAT);
    Q_ASSERT(m_attributes.attributes[1].position == 1);
    Q_ASSERT(m_attributes.attributes[1].tupleSize == 4);
    Q_ASSERT(m_attributes.attributes[1].type == GL_UNSIGNED_BYTE);
    return (const ColoredPoint2D *) m_data;
}

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSGGEOMETRY_H
