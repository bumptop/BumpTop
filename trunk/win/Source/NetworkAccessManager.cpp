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

#ifdef ENABLE_WEBKIT
#include "BT_QtUtil.h"
#include "BT_WindowsOS.h"
#include "NetworkAccessManager.h"
#include "moc/moc_NetworkAccessManager.cpp"

NetworkCookieJar::NetworkCookieJar()
{
	_cookieJarFilePath = native(winOS->GetCacheDirectory() / "Webkit" / "cookies.json");
	QDir().mkdir(native(QFileInfo(_cookieJarFilePath).absoluteDir()));
	deserialize();
}

NetworkCookieJar::~NetworkCookieJar()
{
	serialize();
}

void NetworkCookieJar::serialize()
{
	QMutexLocker m(&_memberMutex);

	// write the cookies to the specified file
	Json::Value cookieRoot;
	
	QList<QNetworkCookie> cookies = allCookies();
	QListIterator<QNetworkCookie> iter(cookies);
	while (iter.hasNext())
	{
		const QNetworkCookie& cookie = iter.next();
		if (cookie.isSessionCookie() || cookie.expirationDate() < QDateTime::currentDateTime())
			continue;
		QByteArray bytes = cookie.toRawForm();
		cookieRoot.append(QString(bytes).toUtf8().constData());		
	}
	Json::StyledWriter jsonWriter;
	QString outStr = QString::fromUtf8(jsonWriter.write(cookieRoot).c_str());
	write_file_utf8(outStr, _cookieJarFilePath);
}

void NetworkCookieJar::deserialize()
{
	QMutexLocker m(&_memberMutex);

	// read the cookies from the specified file
	QList<QNetworkCookie> cookies;
	Json::Value cookieRoot;
	Json::Reader reader;
	QByteArray tmp = read_file_utf8(_cookieJarFilePath).toUtf8();
	if (reader.parse(tmp.constData(), cookieRoot))
	{
		for (int i = 0; i < cookieRoot.size(); ++i)
		{
			const Json::Value& value = cookieRoot[i];
			if (value.isString())
			{
				QString cookieStr = QString::fromUtf8(value.asString().c_str());
				if (!cookieStr.isEmpty())
				{
					QList<QNetworkCookie> cookiesTmp = QNetworkCookie::parseCookies(cookieStr.toUtf8());
					QListIterator<QNetworkCookie> iter(cookiesTmp);
					while (iter.hasNext())
					{
						cookies.append(iter.next());
					}
				}
			}
		}
	}

	QNetworkCookieJar::setAllCookies(cookies);
}


// NetworkAccessManager implementation
NetworkAccessManager::NetworkAccessManager(QObject * parent, QNetworkCookieJar * jar)
: QNetworkAccessManager(parent)
{
    connect(this, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)),
			this, SLOT(skipSslErrors(QNetworkReply*, const QList<QSslError>&)));
	setCookieJar(jar);
}

void NetworkAccessManager::skipSslErrors(QNetworkReply *reply, const QList<QSslError> &error)
{
	reply->ignoreSslErrors();
	return;
}

#endif