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
#include "BT_CustomizeWizard.h"
#include "BT_FileSystemManager.h"
#include "BT_QtUtil.h"
#include "BT_WindowsOS.h"


#include "moc/moc_BT_CustomizeWizard.cpp"

//type can either be GADGET or THEME, this refers to what we want to customize
CustomizeWizard::CustomizeWizard(CustomizeThis type, QString defaultUrl)
{
	wizard = new QWizard();
	wizard->setAttribute(Qt::WA_DeleteOnClose);
	_type = type;

	// The screen width should be 1100 unless the resolution is too low
	int width = 1100;
	int windowWidth = winOS->GetWindowWidth();
	if (windowWidth > 0 && windowWidth < width)
		width = windowWidth;
	int height = 1000;
	int windowHeight = winOS->GetWindowHeight();
	if (windowHeight > 0 && windowHeight < height)
		height = windowHeight;
	wizard->resize(width, height * 3 / 4);

	QPalette palette;
	QBrush brush(QColor(243, 243, 227, 255));
	brush.setStyle(Qt::SolidPattern);
	palette.setBrush(QPalette::Active, QPalette::Base, brush);
	palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
	QBrush brush1(QColor(240, 240, 240, 255));
	brush1.setStyle(Qt::SolidPattern);
	palette.setBrush(QPalette::Disabled, QPalette::Base, brush1);
	wizard->setPalette(palette);
	wizard->setOptions(QWizard::HelpButtonOnRight);
	QWizardPage * page = new QWizardPage(wizard);

	QVBoxLayout * verticalLayout = new QVBoxLayout(page);

	if(_type == THEME) 
	{
		wizard->setWindowTitle(QT_TR_NOOP("Customize"));
		webView = new QWebView(page);
		webView->page()->setForwardUnsupportedContent(true);
		webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
		verticalLayout->addWidget(webView, 200);

		progressBar = new QProgressBar(page);
		progressBar->setVisible(false);
		progressBar->setMinimum(0);
		verticalLayout->addWidget(progressBar, 1);

		_defaultStatusMsg = QT_TR_NOOP("Download a theme above, and it will be installed");
		status = new QLabel(_defaultStatusMsg);
		status->setFont(QFont("Tahoma", 12));
		verticalLayout->addWidget(status,0,Qt::AlignRight);
	}

	if (_type == PRO)
	{
		wizard->setWindowTitle(QT_TR_NOOP("Upgrade to BumpTop Pro"));
		webView = new QWebView(page);
		verticalLayout->addWidget(webView, 200);

		progressBar = new QProgressBar(page);
		progressBar->setVisible(false);
		progressBar->setMinimum(0);
		verticalLayout->addWidget(progressBar, 1);
	}

	if(_type == GADGET) 
	{
		newsLabel = new QLabel(QT_TR_NOOP("News:"));
		weatherLabel = new QLabel(QT_TR_NOOP("Weather:"));
		sportsLabel = new QLabel(QT_TR_NOOP("Sports:"));
		socialLabel = new QLabel(QT_TR_NOOP("Social Media:"));
		customLabel = new QLabel(QT_TR_NOOP("Custom:"));

		//News defaults
		newsMenu = new QComboBox();
		newsMenu->addItem(QT_NT("http://www.bbc.co.uk"));
		newsMenu->addItem(QT_NT("http://www.cnn.com"));
		newsMenu->addItem(QT_NT("http://www.nytimes.com"));

		//Weather defaults
		weatherMenu = new QComboBox();
		weatherMenu->addItem(QT_NT("http://www.theweathernetwork.com"));
		weatherMenu->addItem(QT_NT("http://www.accuweather.com"));

		//Sports defaults
		sportsMenu = new QComboBox();
		sportsMenu->addItem(QT_NT("http://www.nhl.com"));
		sportsMenu->addItem(QT_NT("http://www.nba.com"));
		sportsMenu->addItem(QT_NT("http://www.nfl.com"));
		sportsMenu->addItem(QT_NT("http://www.mlb.com"));

		//Social media defaults
		socialMenu = new QComboBox();
		socialMenu->addItem(QT_NT("http://www.youtube.com"));
		socialMenu->addItem(QT_NT("http://www.reddit.com"));
		socialMenu->addItem(QT_NT("http://www.digg.com"));

		customEntry = new QLineEdit(QT_NT("http://www.google.com"));

		//Layouts
		newsLayout = new QHBoxLayout();
		weatherLayout = new QHBoxLayout();
		sportsLayout = new QHBoxLayout();
		socialLayout = new QHBoxLayout();
		customLayout = new QHBoxLayout();

		newsLayout->addWidget(newsLabel,0,Qt::AlignLeft);
		newsLayout->addWidget(newsMenu,1,Qt::AlignRight);

		weatherLayout->addWidget(weatherLabel,0,Qt::AlignLeft);
		weatherLayout->addWidget(weatherMenu,1,Qt::AlignRight);

		sportsLayout->addWidget(sportsLabel,0,Qt::AlignLeft);
		sportsLayout->addWidget(sportsMenu,1,Qt::AlignRight);

		socialLayout->addWidget(socialLabel,0,Qt::AlignLeft);
		socialLayout->addWidget(socialMenu,1,Qt::AlignRight);

		customLayout->addWidget(customLabel,0,Qt::AlignLeft);
		customLayout->addWidget(customEntry,1,Qt::AlignRight);

		verticalLayout->addLayout(newsLayout);
		verticalLayout->addLayout(weatherLayout);
		verticalLayout->addLayout(sportsLayout);
		verticalLayout->addLayout(socialLayout);
		verticalLayout->addLayout(customLayout);
	}

	page->setLayout(verticalLayout);
	wizard->addPage(page);
	
	buttonLayout = new QList<QWizard::WizardButton>();
	*buttonLayout << QWizard::Stretch << QWizard::CustomButton1 << QWizard::FinishButton;
	wizard->setButtonLayout(*buttonLayout);

	wizard->setButtonText(QWizard::FinishButton, QT_TR_NOOP("Close"));	
	wizard->setButtonText(QWizard::CustomButton1, QT_TR_NOOP("Go Back"));

	wizard->setWizardStyle(QWizard::ModernStyle);
	wizard->setOption(QWizard::HaveHelpButton, false);
	
	if(_type == THEME) {
		QMetaObject::connectSlotsByName(webView->page());
		connect(webView->page(), SIGNAL(loadStarted()), this, SLOT(loadStarted()));	
		connect(webView->page(), SIGNAL(loadProgress(int)), this, SLOT(loadProgress(int)));	
		connect(webView->page(), SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));	
		connect(webView->page(), SIGNAL(downloadRequested(const QNetworkRequest &)), this, SLOT(downloadRequested(const QNetworkRequest &)));
		connect(webView->page(), SIGNAL(unsupportedContent(QNetworkReply *)), this, SLOT(unsupportedContent(QNetworkReply *)));
		connect(webView->page(), SIGNAL(linkClicked(const QUrl &)), this, SLOT(linkClicked(const QUrl &)));
		connect(wizard, SIGNAL(customButtonClicked(int)), this, SLOT(customButtonClicked(int)));
		connect(wizard, SIGNAL(finished(int)), this, SLOT(exitDialog(int)));

		webView->setUrl(QUrl(QT_NT("http://bumptop.customize.org/")));

	}
	else if (_type == PRO)
	{
		QMetaObject::connectSlotsByName(webView->page());
		connect(wizard, SIGNAL(finished(int)), this, SLOT(exitDialog(int)));

		connect(webView->page(), SIGNAL(loadStarted()), this, SLOT(loadStarted()));	
		connect(webView->page(), SIGNAL(loadProgress(int)), this, SLOT(loadProgress(int)));	
		connect(webView->page(), SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));	
		connect(wizard, SIGNAL(customButtonClicked(int)), this, SLOT(customButtonClicked(int)));

		webView->setUrl(QUrl(QT_NT("http://bumptop.com/download.php")));
		
	}
}

CustomizeWizard::~CustomizeWizard()
{		
	if (wizard)
		wizard->close();	
}

QString CustomizeWizard::exec()
{ 
	wizard->show();
	wizard->activateWindow();
	return _text;
}

void CustomizeWizard::exitDialog(int result) 
{
	webView->stop();
	wizard->close();
	wizard= NULL;
	deleteLater();
}

void CustomizeWizard::customButtonClicked(int which)
{
	if(_type == THEME || _type == PRO && which == QWizard::CustomButton1) 
	{
		if (webView->history()->canGoBack())
		{	
			webView->history()->back();
		}
	}
	else
	{
		_ASSERT(QWizard::CustomButton1 == which);
		if (QWizard::CustomButton1 != which)
			return;
		bool ok;
		QString text = QInputDialog::getText(NULL, QT_TR_NOOP("Custom Gadget"), QT_TR_NOOP("Enter URL or Google Gadget code"), QLineEdit::Normal, "http://www.google.com/ig", &ok);
		if (ok && !text.isEmpty())
		{
			_text = text;
			wizard->accept();
		}
	}
}

void CustomizeWizard::loadStarted()
{
	if (_type == THEME)
		status->setText(_defaultStatusMsg);
	progressBar->setMaximum(100);
	progressBar->setValue(0);
	progressBar->setVisible(true);
}

void CustomizeWizard::loadProgress(int progress)
{
	if (wizard)
	{
		if (webView->history()->canGoBack())
		{
			wizard->button(QWizard::CustomButton1)->setEnabled(true);
		}
		else
		{
			wizard->button(QWizard::CustomButton1)->setDisabled(true);
		}
	}
	
	progressBar->setMaximum(100);
	progressBar->setValue(progress);
}

void CustomizeWizard::loadFinished(bool ok)
{
	progressBar->setVisible(false);
}

void CustomizeWizard::linkClicked(const QUrl & url)
{
	if (url.toString().contains(".rar", Qt::CaseInsensitive))	
		status->setText(QT_TR_NOOP("Can't load .rar files!"));
	
	else if (url.toString().contains(".bumptheme", Qt::CaseInsensitive))	
		downloadFile(url);
	
	else if (url.toString().contains(".zip", Qt::CaseInsensitive))
		downloadFile(url);

	else		
		webView->setUrl(url);
	
}

void CustomizeWizard::downloadRequested(const QNetworkRequest & request) 
{
	downloadFile(request.url());
}

void CustomizeWizard::unsupportedContent(QNetworkReply * reply) 
{
	downloadFile(reply->url());
}

void CustomizeWizard::downloadFile(const QUrl & url)
{
	switch (_type) {
		case GADGET:
			break;
		case THEME:
			status->setText(QT_TR_NOOP("Downloading File..."));
			break;
	}

	//http://myhowtosandprojects.blogspot.com/2008/08/render-html-in-your-applications-cc.html
	QString fileName = native(winOS->GetCacheDirectory() / "webkit" / QFileInfo(url.toString()).fileName());
	if (url.toString().contains("customize.org", Qt::CaseInsensitive))
		if (QFileInfo(url.toString()).fileName().contains(".zip", Qt::CaseInsensitive))
			fileName.replace(QT_NT(".zip"), QT_NT(".bumptheme"), Qt::CaseInsensitive);
	QNetworkRequest request(url);
	request.setAttribute(QNetworkRequest::User, fileName);
	//TODO: use proxy settings for QNetworkAccessManager
	QNetworkAccessManager * networkManager = webView->page()->networkAccessManager();
	QNetworkReply * reply = networkManager->get(request);
	connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
	connect(reply, SIGNAL(finished()), this, SLOT(finished()));
}

void CustomizeWizard::downloadProgress(qint64 received , qint64 total)
{
	progressBar->setMaximum(int(total / 1024));
	progressBar->setValue(int(received / 1024));
	progressBar->setVisible(true);
}

void CustomizeWizard::finished()
{
	progressBar->setVisible(false);
	QNetworkReply * reply = (QNetworkReply *)sender();
	QString fileName = reply->request().attribute(QNetworkRequest::User).toString();
	if (!fileName.isEmpty())
	{
		switch(_type) {
			case GADGET:
				break;
			case THEME:
				status->setText(QT_TR_NOOP("Theme Installed!"));
				break;
		}

		
		QDir().mkdir(native(QFileInfo(fileName).absoluteDir()));
		QFile file(fileName);
		if (file.open(QFile::ReadWrite))
		{
			file.write(reply->readAll());
			file.close();
			fsManager->launchFile(fileName);
		} else {
			LOG("CustomizeWizard::Unable to open theme file for read/write");
			LOG(file.fileName());
		}
	} else {
		LOG("CustomizeWizard::Download failed from:");
		LOG(reply->request().url().toString());
		LOG(reply->errorString());
	}
}