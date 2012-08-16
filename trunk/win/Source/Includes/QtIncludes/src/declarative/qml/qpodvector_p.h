/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
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

#ifndef QPODVECTOR_P_H
#define QPODVECTOR_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qglobal.h>
#include <QDebug>

QT_BEGIN_NAMESPACE

template<class T, int Increment=1024>
class QPODVector 
{
public:
    QPODVector()
    : m_count(0), m_capacity(0), m_data(0) {}
    ~QPODVector() { if (m_data) ::free(m_data); } 

    const T &at(int idx) const {
        return m_data[idx];
    }

    T &operator[](int idx) {
        return m_data[idx];
    }

    void clear() {
        m_count = 0;
    }

    void prepend(const T &v) {
        insert(0, v);
    }

    void append(const T &v) {
        insert(m_count, v);
    }

    void insert(int idx, const T &v) {
        if (m_count == m_capacity) {
            m_capacity += Increment;
            m_data = (T *)realloc(m_data, m_capacity * sizeof(T));
        }
        int moveCount = m_count - idx;
        if (moveCount)
            ::memmove(m_data + idx + 1, m_data + idx, moveCount * sizeof(T));
        m_count++;
        m_data[idx] = v;
    }

    void reserve(int count) {
        if (count >= m_capacity) {
            m_capacity = (count + (Increment-1)) & (0xFFFFFFFF - Increment + 1);
            m_data = (T *)realloc(m_data, m_capacity * sizeof(T));
        }
    }

    void insertBlank(int idx, int count) {
        int newSize = m_count + count;
        reserve(newSize);
        int moveCount = m_count - idx;
        if (moveCount) 
            ::memmove(m_data + idx + count,  m_data + idx, 
                      moveCount * sizeof(T));
        m_count = newSize;
    }

    void remove(int idx, int count = 1) {
        int moveCount = m_count - (idx + count);
        if (moveCount)
            ::memmove(m_data + idx, m_data + idx + count, 
                      moveCount * sizeof(T));
        m_count -= count;
    }

    void removeOne(const T &v) {
        int idx = 0;
        while (idx < m_count) {
            if (m_data[idx] == v) {
                remove(idx);
                return;
            }
            ++idx;
        }
    }

    int find(const T &v) {
        for (int idx = 0; idx < m_count; ++idx)
            if (m_data[idx] == v)
                return idx;
        return -1;
    }

    bool contains(const T &v) {
        return find(v) != -1;
    }

    int count() const {
        return m_count;
    }

    void copyAndClear(QPODVector<T,Increment> &other) {
        if (other.m_data) ::free(other.m_data);
        other.m_count = m_count;
        other.m_capacity = m_capacity;
        other.m_data = m_data;
        m_count = 0;
        m_capacity = 0;
        m_data = 0;
    }

    QPODVector<T,Increment> &operator<<(const T &v) { append(v); return *this; }
private:
    QPODVector(const QPODVector &);
    QPODVector &operator=(const QPODVector &);
    int m_count;
    int m_capacity;
    T *m_data;
};

QT_END_NAMESPACE

#endif
