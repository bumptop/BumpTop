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

#ifndef BT_NETWORKACCESSMANAGER
#define BT_NETWORKACCESSMANAGER

#ifdef ENABLE_WEBKIT

// we need to override the QNetworkCookieJar so that we can handle cookie serialization
class NetworkCookieJar : public QNetworkCookieJar
{
	QMutex _memberMutex;
	QString _cookieJarFilePath;

	friend class Singleton<NetworkCookieJar>;
	NetworkCookieJar();
	
public:
	virtual ~NetworkCookieJar();

	void serialize();
	void deserialize();
};

// we need to override the QNetworkManager so that we can handle ssl errors
class NetworkAccessManager : public QNetworkAccessManager
{
	Q_OBJECT

public:
    NetworkAccessManager(QObject * parent, QNetworkCookieJar * jar);

private slots:
    void skipSslErrors(QNetworkReply *reply, const QList<QSslError> &error);
};

#endif // ENABLE WEBKIT
#endif // BT_NETWORKACCESSMANAGER
