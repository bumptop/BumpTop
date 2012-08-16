/*  This file is part of the KDE project.

Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).

This library is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 or 3 of the License.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef OBJECTDUMP_H
#define OBJECTDUMP_H

#include <QObject>
#include <QList>
#include <QByteArray>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

namespace ObjectDump
{

/**
 * Abstract base for annotator classes invoked by QVisitor.
 */
class QAnnotator : public QObject
{
    Q_OBJECT
public:
    virtual ~QAnnotator();
    virtual QList<QByteArray> annotation(const QObject& object) = 0;
};

/**
 * Annotator which replicates QObject::dumpObjectTree functionality.
 */
class QAnnotatorBasic : public QAnnotator
{
    Q_OBJECT
public:
    QList<QByteArray> annotation(const QObject& object);
};

/**
 * Annotator which returns widget information.
 */
class QAnnotatorWidget : public QAnnotator
{
    Q_OBJECT
public:
    QList<QByteArray> annotation(const QObject& object);
};


class QDumperPrivate;

/**
 * Class used to dump information about individual QObjects.
 */
class QDumper : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QDumper)

public:
    QDumper();
    ~QDumper();

    /**
     * Specify a prefix, to be printed on each line of output.
     */
    void setPrefix(const QString& prefix);

    /**
     * Takes ownership of annotator.
     */
    void addAnnotator(QAnnotator* annotator);

    /**
     * Invoke each annotator on the object and write to debug output.
     */
    void dumpObject(const QObject& object);

private:
    QScopedPointer<QDumperPrivate> d_ptr;

};


class QVisitorPrivate;

/**
 * Visitor class which dumps information about nodes in the object tree.
 */
class QVisitor : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QVisitor)

public:
    QVisitor();
    ~QVisitor();

    /**
     * Specify a prefix, to be printed on each line of output.
     */
    void setPrefix(const QString& prefix);

    /**
     * Set number of spaces by which each level of the tree is indented.
     */
    void setIndent(unsigned indent);

    /**
     * Called by the visitor algorithm before starting the visit.
     */
    void visitPrepare();

    /**
     * Called by the visitor algorithm as each node is visited.
     */
    void visitNode(const QObject& object);

    /**
     * Called by the visitor algorithm when the visit is complete.
     */
    void visitComplete();

    /**
     * Takes ownership of annotator.
     */
    void addAnnotator(QAnnotator* annotator);

private:
    QScopedPointer<QVisitorPrivate> d_ptr;

};


//-----------------------------------------------------------------------------
// Utility functions
//-----------------------------------------------------------------------------

void addDefaultAnnotators(QDumper& dumper);
void addDefaultAnnotators(QVisitor& visitor);

void dumpTreeFromRoot(const QObject& root, QVisitor& visitor);
void dumpTreeFromLeaf(const QObject& leaf, QVisitor& visitor);
void dumpAncestors(const QObject& leaf, QVisitor& visitor);

} // namespace ObjectDump

QT_END_NAMESPACE

#endif
