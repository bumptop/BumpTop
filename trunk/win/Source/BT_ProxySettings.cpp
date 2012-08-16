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
#include "BT_ProxySettings.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"
#include "BT_Logger.h"
#include "BT_QtUtil.h"
 
// so the proxy settings right now:
// 
// - loaded from IE on startup
// - if you have non empty strings in both http proxy field and ftp proxy field, BumpTop will ignore the IE settings and just use those
// - otherwise, it will try to use the IE settings and popup a message box if that fails
// - at the moment, proxy authentication is not supported

ProxySettings::ProxySettings()
{
	_proxyMode = NO_PROXY;
	_httpProxyUrl = "";
}

ProxySettings::~ProxySettings()
{}

void ProxySettings::copyProxySettingsFromIE()
{
	LOG("copyProxySettingsFromIE");
	WINHTTP_CURRENT_USER_IE_PROXY_CONFIG IEProxyConfigs;
	ZeroMemory(&IEProxyConfigs, sizeof(IEProxyConfigs));

	if (::WinHttpGetIEProxyConfigForCurrentUser(&IEProxyConfigs) == FALSE)
	{
		errorLogWrite("Failed to get IE proxy config info; ::WinHttpGetIEProxyConfigForCurrentUser() failed; error = " + QString::number(::GetLastError()) + ".\n");
	}
	else
	{
		if (IEProxyConfigs.lpszAutoConfigUrl != NULL)
		{
			setProxyDetectionToAutoConfigurationUrl(QString::fromUtf16((const ushort *) IEProxyConfigs.lpszAutoConfigUrl));
		}
		else if (IEProxyConfigs.fAutoDetect)
		{
			setProxyDetectionToAutoDetect();
		}
		else if (IEProxyConfigs.lpszProxy != NULL)
		{
			setProxyManually(QString::fromUtf16((const ushort *) IEProxyConfigs.lpszProxy));
		}
		else
		{
			setProxyToNone();
		}

		if (IEProxyConfigs.lpszProxyBypass != NULL)
			GlobalFree(IEProxyConfigs.lpszProxyBypass);

		if (IEProxyConfigs.lpszProxy != NULL)
			GlobalFree(IEProxyConfigs.lpszProxy);

		if (IEProxyConfigs.lpszAutoConfigUrl != NULL)
			GlobalFree(IEProxyConfigs.lpszAutoConfigUrl);

	}
}

QString ProxySettings::getProxyServer( ProxyMode proxyDetectionMethod, QString autoConfigurationUrl, QString testURL )
{
	LOG("getProxyServer");
	// Copied more or less verbatim from Windows SDK example
	WINHTTP_PROXY_INFO proxyInfo = {0};
	WINHTTP_AUTOPROXY_OPTIONS AutoProxyOptions = {0};

	if (proxyDetectionMethod == AUTO_DETECT)
	{
		AutoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
		AutoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
		AutoProxyOptions.lpszAutoConfigUrl = NULL;
	}
	else if (proxyDetectionMethod == AUTO_CONFIGURATION_URL)
	{	
		AutoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
		AutoProxyOptions.dwAutoDetectFlags = 0;
		AutoProxyOptions.lpszAutoConfigUrl = (LPCWSTR) autoConfigurationUrl.utf16();

	}
	AutoProxyOptions.fAutoLogonIfChallenged = TRUE;

	HINTERNET hSession = openWinHTTPConnection();

	QString proxyServer = NULL;

	if (::WinHttpGetProxyForUrl(hSession, (LPCWSTR) testURL.utf16(),
		&AutoProxyOptions, 
		&proxyInfo) == FALSE)
	{


		errorLogWrite("Failed to discover proxy info; ::WinHttpGetProxyForUrl() failed; error = " + QString::number(::GetLastError()) + ".\n");

		if (proxyInfo.lpszProxy != NULL)
			GlobalFree(proxyInfo.lpszProxy);
		if (proxyInfo.lpszProxyBypass != NULL)
			GlobalFree(proxyInfo.lpszProxyBypass);

		// else, pick up the WinHTTP proxy configuration set by the WinHTTP Proxycfg.exe utility.
		WINHTTP_PROXY_INFO pi;
		pi.dwAccessType = WINHTTP_ACCESS_TYPE_DEFAULT_PROXY;
		pi.lpszProxy = NULL;
		pi.lpszProxyBypass = NULL;

		if (::WinHttpSetOption(hSession,
			WINHTTP_OPTION_PROXY,
			&pi,
			sizeof(pi)) == FALSE)
		{
			errorLogWrite("Failed to discover proxy info; ::WinHttpSetOption() failed; error = " + QString::number(::GetLastError()) + ".\n");
		}
		else
		{
			proxyServer = QString::fromUtf16((const ushort *) pi.lpszProxy);

			if (pi.lpszProxyBypass != NULL)
				GlobalFree(pi.lpszProxyBypass);
		}
	}
	else
	{
		proxyServer = QString::fromUtf16((const ushort *) proxyInfo.lpszProxy);

		if (proxyInfo.lpszProxyBypass != NULL)
			GlobalFree(proxyInfo.lpszProxyBypass);
	}



	WinHttpCloseHandle(hSession);

	if (proxyServer != NULL)
		return QString(proxyServer);
	else
		return QString();
}

HINTERNET ProxySettings::openWinHTTPConnection()
{
	LOG("openWinHTTPConnection");
	HINTERNET hSession = ::WinHttpOpen(L"BumpTop-Proxy-Checker/1.0",
		WINHTTP_ACCESS_TYPE_NO_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		WINHTTP_FLAG_ASYNC);

	if (hSession == NULL)
	{
		// error handling?
	}
	return hSession;
}

void ProxySettings::getProxyServerUsingAutoDetect()
{
	LOG("getProxyServerUsingAutoDetect");
	_httpProxyUrl = getProxyServer( AUTO_DETECT, "", "http://bumptop.com");
}

void ProxySettings::getProxyServerUsingAutoConfigurationURL( QString configurationURL )
{
	LOG("getProxyServerUsingAutoConfigurationURL");
	_httpProxyUrl = getProxyServer( AUTO_CONFIGURATION_URL, configurationURL, "http://bumptop.com");
}

void ProxySettings::setProxyDetectionToAutoConfigurationUrl( QString autoConfigurationUrl )
{
	LOG("setProxyDetectionToAutoConfigurationUrl");
	_proxyMode = AUTO_CONFIGURATION_URL;
	_autoProxyConfigurationUrl = autoConfigurationUrl;
	getProxyServerUsingAutoConfigurationURL(autoConfigurationUrl);
}

void ProxySettings::setProxyDetectionToAutoDetect()
{
	LOG("setProxyDetectionToAutoDetect");
	_proxyMode = AUTO_DETECT;
	getProxyServerUsingAutoDetect();
}

void ProxySettings::setProxyManually( QString httpProxyURL)
{
	LOG("setProxyManually");
	_proxyMode = MANUAL;
	_httpProxyUrl = httpProxyURL;
}

void ProxySettings::setProxyToNone()
{
	LOG("setProxyToNone");
	_proxyMode = NO_PROXY;
	_httpProxyUrl = "";
}

void ProxySettings::setProxySettings(ProxyMode proxySetting, QString autoConfigurationUrl, QString httpProxyURL)
{
	LOG("setProxySettings");
	switch (proxySetting)
	{
	case NO_PROXY:
		setProxyToNone();
		break;
	case MANUAL:
		setProxyManually(httpProxyURL);
		break;
	case AUTO_DETECT:
		setProxyDetectionToAutoDetect();
		break;
	case AUTO_CONFIGURATION_URL:
		setProxyDetectionToAutoConfigurationUrl(autoConfigurationUrl);
		break;
	}
}

QString ProxySettings::getHttpProxyURL()
{
	return _httpProxyUrl;
}

QString ProxySettings::getAutoProxyConfigurationURL()
{
	return _autoProxyConfigurationUrl;
}

ProxyMode ProxySettings::getProxyMode()
{
	return _proxyMode;
}

void ProxySettings::errorLogWrite( QString message )
{
	append_file_utf8(message, native(winOS->GetDataDirectory() / "proxy_errors_log.txt"));
}

