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

#ifndef OBJECTTREE_H
#define OBJECTTREE_H

#include <QObject>
#include <QStack>

QT_BEGIN_NAMESPACE

namespace ObjectTree
{

/**
 * Depth-first iterator for QObject tree
 */
class DepthFirstConstIterator
{
public:
    DepthFirstConstIterator();
    DepthFirstConstIterator(const QObject& root);

    DepthFirstConstIterator& operator++();

    inline bool operator==(const DepthFirstConstIterator& other) const
    { return other.m_pointee == m_pointee; }

    inline bool operator!=(const DepthFirstConstIterator& other) const
    { return other.m_pointee != m_pointee; }

    inline const QObject* operator->() const { return m_pointee; }
    inline const QObject& operator*() const { return *m_pointee; }

private:
    void backtrack();

private:
    const QObject* m_pointee;
    QStack<int> m_history;
};

/**
 * Ancestor iterator for QObject tree
 */
class AncestorConstIterator
{
public:
    AncestorConstIterator();
    AncestorConstIterator(const QObject& root);

    inline AncestorConstIterator& operator++()
    { m_ancestors.pop(); return *this; }

    inline bool operator==(const AncestorConstIterator& other) const
    { return other.m_ancestors == m_ancestors; }

    inline bool operator!=(const AncestorConstIterator& other) const
    { return other.m_ancestors != m_ancestors; }

    inline const QObject* operator->() const { return m_ancestors.top(); }
    inline const QObject& operator*() const { return *m_ancestors.top(); }

private:
    QStack<const QObject*> m_ancestors;

};

/**
 * Generic algorithm for visiting nodes in an object tree.  Nodes in the
 * tree are visited in a const context, therefore they are not modified
 * by this algorithm.
 *
 * Visitor must provide functions with the following signatures:
 *
 *      Called before visit begins
 *          void visitPrepare()
 *
 *      Called on each node visited
 *          void visitNode(const QObject& object)
 *
 *      Called when visit is complete
 *          void visitComplete()
 */
template <class Iterator, class Visitor>
void visit(Iterator begin, Iterator end, Visitor& visitor)
{
    visitor.visitPrepare();

    for ( ; begin != end; ++begin)
        visitor.visitNode(*begin);

    visitor.visitComplete();
}

} // namespace ObjectTree

QT_END_NAMESPACE

#endif // OBJECTTREE_H
