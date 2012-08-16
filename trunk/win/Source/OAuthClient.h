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

#ifndef OAUTHCLIENT_H
#define OAUTHCLIENT_H

class OAuthClient
{
	QString _cKey;
	QString _cSecret;
	QString _tKey;
	QString _tSecret;
	QString _requestTokenRequestUri;
	QString _accessTokenRequestUri;
	QString _authorizationLaunchUrl;

private:
	QString toBase64(QString str);
	QString fromBase64(QString base64Str);
	QString encodeStr(QString str);
	QString unencodeStr(QString encodedStr);
	QString signHMAC_SHA1(QString message, QString key);
	QString signRSA_SHA1(QString message, QString key);
	int verifyRSA_SHA1(QString message, QString cert, QString signedText);
	void prepareParameters(QString url, QString method, QHash<QString, QString>& params);
	QString generateSignature(QString url, QString method, const QHash<QString, QString>& params);
	QString generateNonce();

	QString postHttpRequest(QString method, QHash<QString, QString> params);

	bool extractParams(QString reply, QHash<QString, QString>& paramsOut);
	bool extractTokens(const QHash<QString, QString>& params, bool retainInSettings=false);

public:
	OAuthClient();

	bool launchAuthorizationUrl(QString authorizationLaunchUrl);
	bool getRequestToken(QString requestTokenRequestUri, QString cKey, QString cSecret);
	bool getAccessToken(QString accessTokenRequestUri, QString oauthVerifierToken);
	void setTokens(QString cKey, QString cSecret, QString tKey, QString tSecret);
	QString postRequest(QString url, QString method, QHash<QString, QString> params);
};

#endif // OAUTHCLIENT_H