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

#ifndef _BT_PROXYSETTINGS_
#define _BT_PROXYSETTINGS_

enum ProxyMode
{
	NO_PROXY,
	MANUAL,
	AUTO_DETECT,
	AUTO_CONFIGURATION_URL,
};

class ProxySettings
{
	friend class ProxySettingsTest;
	ProxyMode _proxyMode;
	QString _autoProxyConfigurationUrl;
	QString _httpProxyUrl;

	HINTERNET openWinHTTPConnection();
	QString getProxyServer(ProxyMode proxyDetectionMethod, QString autoProxyConfigurationUrl, QString testURL);
	void getProxyServerUsingAutoDetect();
	void getProxyServerUsingAutoConfigurationURL(QString autoProxyConfigurationUrl);

public:
	ProxySettings();
	~ProxySettings();


	void errorLogWrite(QString message);

	void copyProxySettingsFromIE();

	void setProxyDetectionToAutoConfigurationUrl(QString autoProxyConfigurationUrl);
	void setProxyDetectionToAutoDetect();
	void setProxyManually(QString httpProxyURL);
	void setProxyToNone();
	void setProxySettings(ProxyMode proxyMode, QString autoProxyConfigurationUrl, QString httpProxyURL );

	QString getHttpProxyURL();
	QString getAutoProxyConfigurationURL();
	ProxyMode getProxyMode();
};

#endif // _BT_PROXYSETTINGS_
