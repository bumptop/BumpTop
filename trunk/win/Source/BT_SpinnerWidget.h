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

#pragma once

class SpinnerWidget : public QWidget
{
	Q_OBJECT

private:
	QString _source;	//spinner file location
	QLabel *_label;		//spinner container widget
	QMovie *_movie;		//spinner
	QColor _background;	//background colour, alpha

	void init();

public:
	SpinnerWidget(QString source, QWidget *w);
	SpinnerWidget(QString source, QWidget *w, QColor c);
	~SpinnerWidget();

	void setBackground(QColor c);
	void start();
	void stop();

	void paintEvent(QPaintEvent *e);
	void show();
	void hide();
};