// Copyright 2012 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "BT_Common.h"
#include "BT_SpinnerWidget.h"
#include "moc/moc_BT_SpinnerWidget.cpp"

SpinnerWidget::SpinnerWidget(QString source, QWidget *w)
: QWidget(w)
, _source(source)
{
	_background = QColor(255, 255, 255, 150);	//default white translucent
	init();
}

SpinnerWidget::SpinnerWidget(QString source, QWidget *w, QColor c)
: QWidget(w)
, _source(source)
, _background(c)
{
	init();
}

SpinnerWidget::~SpinnerWidget()
{
	SAFE_DELETE(_movie);
	SAFE_DELETE(_label);
}

void SpinnerWidget::init()
{
	resize(parentWidget()->width(), parentWidget()->height());

	_movie = new QMovie(_source);
	assert(_movie->isValid());

	_label = new QLabel(this);
	_label->move(parentWidget()->width()/2 - _movie->scaledSize().width()/2, parentWidget()->height()/2 - _movie->scaledSize().height()/2);
	_label->setMovie(_movie);
}

void SpinnerWidget::setBackground(QColor c)
{
	_background = c;
}

void SpinnerWidget::start()
{
	_movie->start();
}

void SpinnerWidget::stop()
{
	_movie->stop();
}

void SpinnerWidget::paintEvent(QPaintEvent *e)
{
	QPainter painter(this);
	painter.fillRect(QRect(0, 0, this->width(), this->height()), _background);
}

void SpinnerWidget::show()
{
	QWidget::show();
	start();
}

void SpinnerWidget::hide()
{
	QWidget::hide();
	stop();
}