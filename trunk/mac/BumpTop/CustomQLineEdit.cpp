/*
 *  Copyright 2012 Google Inc. All Rights Reserved.
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <QtGui/QKeyEvent>

#include "BumpTop/CustomQLineEdit.h"
#include "BumpTop/QStringHelpers.h"

CustomQLineEdit::CustomQLineEdit(QWidget * parent)
: QLineEdit(parent) {
}

void CustomQLineEdit::inputMethodEvent(QInputMethodEvent * event) {
  QLineEdit::inputMethodEvent(event);
  QString string_to_emit;
  if (event->preeditString().size() == 0) {
    QLineEdit::setText(QLineEdit::text());
    string_to_emit = QLineEdit::text();
  } else {
    string_to_emit = QLineEdit::text() + event->preeditString();
  }
  emit QLineEdit::textChanged(string_to_emit);
}

#include "BumpTop/moc/moc_CustomQLineEdit.cpp"
