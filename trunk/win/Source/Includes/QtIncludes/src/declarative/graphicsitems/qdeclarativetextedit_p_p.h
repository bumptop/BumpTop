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

#ifndef QDECLARATIVETEXTEDIT_P_H
#define QDECLARATIVETEXTEDIT_P_H

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

#include "qdeclarativeitem.h"
#include "private/qdeclarativeimplicitsizeitem_p_p.h"

#include <qdeclarative.h>

QT_BEGIN_NAMESPACE
class QTextLayout;
class QTextDocument;
class QTextControl;
class QDeclarativeTextEditPrivate : public QDeclarativeImplicitSizePaintedItemPrivate
{
    Q_DECLARE_PUBLIC(QDeclarativeTextEdit)

public:
    QDeclarativeTextEditPrivate()
      : color("black"), hAlign(QDeclarativeTextEdit::AlignLeft), vAlign(QDeclarativeTextEdit::AlignTop),
      imgDirty(true), dirty(false), richText(false), cursorVisible(false), focusOnPress(true),
      showInputPanelOnFocus(true), clickCausedFocus(false), persistentSelection(true), requireImplicitWidth(false),
      hAlignImplicit(true), rightToLeftText(false), textMargin(0.0), lastSelectionStart(0), lastSelectionEnd(0),
      cursorComponent(0), cursor(0), format(QDeclarativeTextEdit::AutoText), document(0), wrapMode(QDeclarativeTextEdit::NoWrap),
      mouseSelectionMode(QDeclarativeTextEdit::SelectCharacters), selectByMouse(false), canPaste(false),
      yoff(0)
    {
#ifdef Q_OS_SYMBIAN
        if (QSysInfo::symbianVersion() >= QSysInfo::SV_SF_1) {
            showInputPanelOnFocus = false;
        }
#endif
    }

    void init();

    void updateDefaultTextOption();
    void relayoutDocument();
    void updateSelection();
    bool determineHorizontalAlignment();
    bool setHAlign(QDeclarativeTextEdit::HAlignment, bool forceAlign = false);
    void mirrorChange();
    qreal implicitWidth() const;
    void focusChanged(bool);

    QString text;
    QFont font;
    QFont sourceFont;
    QColor  color;
    QColor  selectionColor;
    QColor  selectedTextColor;
    QString style;
    QColor  styleColor;
    QPixmap imgCache;
    QPixmap imgStyleCache;
    QDeclarativeTextEdit::HAlignment hAlign;
    QDeclarativeTextEdit::VAlignment vAlign;
    bool imgDirty : 1;
    bool dirty : 1;
    bool richText : 1;
    bool cursorVisible : 1;
    bool focusOnPress : 1;
    bool showInputPanelOnFocus : 1;
    bool clickCausedFocus : 1;
    bool persistentSelection : 1;
    bool requireImplicitWidth:1;
    bool hAlignImplicit:1;
    bool rightToLeftText:1;
    qreal textMargin;
    int lastSelectionStart;
    int lastSelectionEnd;
    QDeclarativeComponent* cursorComponent;
    QDeclarativeItem* cursor;
    QDeclarativeTextEdit::TextFormat format;
    QTextDocument *document;
    QTextControl *control;
    QDeclarativeTextEdit::WrapMode wrapMode;
    QDeclarativeTextEdit::SelectionMode mouseSelectionMode;
    int lineCount;
    bool selectByMouse;
    bool canPaste;
    int yoff;
    QSize paintedSize;
};

QT_END_NAMESPACE
#endif
