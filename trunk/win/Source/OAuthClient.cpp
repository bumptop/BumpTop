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
#include "BT_FileSystemManager.h"
#include "BT_FileTransferManager.h"
#include "BT_SceneManager.h"
#include "BT_Util.h"
#include "BT_WindowsOS.h"
#include "OAuthClient.h"

OAuthClient::OAuthClient()
{}

bool OAuthClient::extractParams( QString reply, QHash<QString, QString>& paramsOut )
{
	QString replyStr(reply);
	QStringList paramsList = replyStr.split("&", QString::SkipEmptyParts);
	for (int i = 0; i < paramsList.size(); ++i)
	{
		QStringList paramList = paramsList[i].split("=", QString::SkipEmptyParts);
		if (paramList.size() == 2)
		{
			paramsOut.insert(paramList[0].toLower(), paramList[1]);
		}
	}
	return true;
}

bool OAuthClient::extractTokens( const QHash<QString, QString>& params, bool retainInSettings )
{
	if (params.contains("oauth_token") &&
		params.contains("oauth_token_secret"))
	{
		_tKey = params["oauth_token"];
		_tSecret = params["oauth_token_secret"];
		if (retainInSettings)
		{
			GLOBAL(settings).tw_oauth_key = _tKey;
			GLOBAL(settings).tw_oauth_secret = _tSecret;
			winOS->SaveSettingsFile();
		}
		return true;
	}
	return false;
}

QString OAuthClient::postHttpRequest( QString url, QHash<QString, QString> params )
{
	QString queryParams = ftManager->encodeParams(params);
	QString serverAddr = QString("%1?%2")
		.arg(url)
		.arg(queryParams);
	FileTransfer ft(FileTransfer::Download, 0);
	ft.setUrl(serverAddr);
	ft.setTemporaryString();
	ft.setTimeout(10);
	bool succeeded = ftManager->addPostTransfer(ft, true);
	return ft.getTemporaryString();
}

QString OAuthClient::postRequest( QString url, QString method, QHash<QString, QString> params )
{
	assert(!_tKey.isEmpty());
	assert(!_tSecret.isEmpty());

	// otherwise, get the request token to obtain the access token
	prepareParameters(url, method, params);
	return postHttpRequest(url, params);
}

bool OAuthClient::getRequestToken( QString requestTokenRequestUri, QString cKey, QString cSecret )
{
	_cKey = cKey;
	_cSecret = cSecret;
	_requestTokenRequestUri = requestTokenRequestUri;

	// skip this whole process if we already have an access token
	if (!GLOBAL(settings).tw_oauth_key.isEmpty() &&
		!GLOBAL(settings).tw_oauth_secret.isEmpty())
		return true;

	// otherwise, get the request token to obtain the access token
	QHash<QString, QString> params;
	prepareParameters(_requestTokenRequestUri, "POST", params);
	QString result = postHttpRequest(_requestTokenRequestUri, params);
	if (!result.isEmpty())
	{
		// example: oauth_token=hh5s93j4hdidpola&oauth_token_secret=hdhd0244k9j7ao03
		QHash<QString, QString> newParams;
		if (extractParams(result, newParams))
		{
			if (extractTokens(newParams))
			{
				return true;
			}
		}
	}
	return false;
}

bool OAuthClient::getAccessToken( QString accessTokenRequestUri, QString oauthVerifierToken )
{
	_accessTokenRequestUri = accessTokenRequestUri;

	// skip this whole process if we already have an access token
	if (!GLOBAL(settings).tw_oauth_key.isEmpty() &&
		!GLOBAL(settings).tw_oauth_secret.isEmpty())
	{
		_tKey = GLOBAL(settings).tw_oauth_key;
		_tSecret = GLOBAL(settings).tw_oauth_secret;
		return true;
	}

	// otherwise, obtain the access token
	QHash<QString, QString> params;
	params.insert("oauth_verifier", oauthVerifierToken);
	prepareParameters(_accessTokenRequestUri, "POST", params);
	QString result = postHttpRequest(_accessTokenRequestUri, params);
	if (!result.isEmpty())
	{
		// example: oauth_token=hh5s93j4hdidpola&oauth_token_secret=hdhd0244k9j7ao03
		QHash<QString, QString> newParams;
		if (extractParams(result, newParams))
		{
			if (extractTokens(newParams, true))
			{
				return true;
			}
		}
	}
	return false;
}

bool OAuthClient::launchAuthorizationUrl( QString authorizationLaunchUrl )
{
	assert(!_requestTokenRequestUri.isEmpty());
	assert(!_tKey.isEmpty());
	assert(!_tSecret.isEmpty());

	_authorizationLaunchUrl = authorizationLaunchUrl;

	// skip this whole process if we already have an access token
	if (!GLOBAL(settings).tw_oauth_key.isEmpty() &&
		!GLOBAL(settings).tw_oauth_secret.isEmpty())
		return true;

	// otherwise, launch the authorization url so that the user can 
	// authorize BumpTop to act on their behalf and obtain an access token
	QHash<QString, QString> params;
	prepareParameters(_accessTokenRequestUri, "POST", params);
	QString queryParams = ftManager->encodeParams(params);
	QString serverAddr = QString("%1?%2")
		.arg(_authorizationLaunchUrl)
		.arg(queryParams);
	fsManager->launchFile(serverAddr);
	return true;
}

QString OAuthClient::toBase64( QString str )
{
	return QString(str.toAscii().toBase64());
}

QString OAuthClient::fromBase64( QString base64Str )
{
	return QString(QByteArray::fromBase64(base64Str.toAscii()));
}

QString OAuthClient::encodeStr( QString str )
{
	return QUrl::toPercentEncoding(str);
}

QString OAuthClient::unencodeStr( QString encodedStr )
{
	return QUrl::fromEncoded(encodedStr.toAscii()).toString();
}

QString OAuthClient::signHMAC_SHA1( QString message, QString key )
{
#ifdef TWITTER_OAUTH
	unsigned char result[EVP_MAX_MD_SIZE];
	unsigned int resultLen = 0;

	HMAC(EVP_sha1(), key.toAscii().constData(), key.size(), 
		(unsigned char *) message.toAscii().constData(), message.size(),
		result, &resultLen);

	return toBase64(QString(QByteArray((char *) result, resultLen)));
#else
	return QString();
#endif
}

QString OAuthClient::signRSA_SHA1( QString message, QString key )
{
	return QString(); // not being used
	/*
	unsigned char * sig = NULL;
	unsigned char * passphrase = NULL;
	unsigned int len = 0;
	EVP_MD_CTX md_ctx;
	EVP_PKEY * pkey;
	BIO * in;
	in = BIO_new_mem_buf((unsigned char *) key.toAscii().constData(), key.size());
	pkey = PEM_read_bio_PrivateKey(in, NULL, 0, passphrase); // generate sign
	BIO_free(in);

	if (pkey == NULL) 
		return QString();  // "liboauth/ssl: can not read private key\n";

	len = EVP_PKEY_size(pkey);
	sig = (unsigned char *) malloc((len+1)*sizeof(char));

	EVP_SignInit(&md_ctx, EVP_sha1());
	EVP_SignUpdate(&md_ctx, message.toAscii().constData(), message.size());
	if (EVP_SignFinal (&md_ctx, sig, &len, pkey)) 
	{
		sig[len] = '\0';
		QString sigb64Str = toBase64(QString((char *) sig));
		OPENSSL_free(sig);
		EVP_PKEY_free(pkey);
		return sigb64Str;
	}
	return QString();
	*/
}

int OAuthClient::verifyRSA_SHA1( QString message, QString certStr, QString signedText )
{
	return -1; // not being used
	/*
	EVP_MD_CTX md_ctx;
	EVP_PKEY * pkey;
	BIO * in;
	X509 * cert = NULL;
	int err;

	in = BIO_new_mem_buf((unsigned char *) certStr.toAscii().constData(), certStr.size());
	cert = PEM_read_bio_X509(in, NULL, 0, NULL);
	if (cert)  
	{
		pkey = (EVP_PKEY *) X509_get_pubkey(cert); 
		X509_free(cert);
	} 
	else 
	{
		pkey = PEM_read_bio_PUBKEY(in, NULL, 0, NULL);
	}
	BIO_free(in);
	if (pkey == NULL) 
		return -2;

	QString text = fromBase64(signedText);
	EVP_VerifyInit(&md_ctx, EVP_sha1());
	EVP_VerifyUpdate(&md_ctx, message.toAscii().constData(), message.size());
	err = EVP_VerifyFinal(&md_ctx, (unsigned char *) text.toAscii().constData(), text.size(), pkey);
	EVP_MD_CTX_cleanup(&md_ctx);
	EVP_PKEY_free(pkey);
	return (err);
	*/
}

void OAuthClient::prepareParameters( QString url, QString method, QHash<QString, QString>& params )
{
	// append the oauth authentication params
	params.insert("oauth_consumer_key", _cKey);
	if (!_tKey.isEmpty())
		params.insert("oauth_token", _tKey);
	params.insert("oauth_signature_method", "HMAC-SHA1");
	params.insert("oauth_timestamp", QString::number(time(NULL)));
	params.insert("oauth_nonce", generateNonce());
	params.insert("oauth_version", "1.0");
	params.insert("oauth_signature", generateSignature(url, method, params));
}

QString OAuthClient::generateSignature( QString url, QString method, const QHash<QString, QString>& params )
{
	// sort the params so that we can sign the request
	QStringList paramsList;
	QHashIterator<QString, QString> iter(params);
	while (iter.hasNext())
	{
		iter.next();
		paramsList.append(QString("%1=%2").arg(iter.key()).arg(encodeStr(iter.value())));
	}
	paramsList.sort();

	// sign the params and return the signature
	QString encUrl = encodeStr(url);
	QString encParamsList = encodeStr(paramsList.join("&"));
	QString paramsStr = method + "&" + encUrl + "&" + encParamsList;
	QString key = encodeStr(_cSecret) + "&";
	if (!_tSecret.isEmpty())
		key += encodeStr(_tSecret);
	return signHMAC_SHA1(paramsStr, key);
}

QString OAuthClient::generateNonce()
{
	// generate a random string of 32 characters
	srand(time(NULL));
	return QCryptographicHash::hash(QString::number(rand()).toAscii(), QCryptographicHash::Md5).toHex();
}

void OAuthClient::setTokens( QString cKey, QString cSecret, QString tKey, QString tSecret )
{
	_cKey = cKey;
	_cSecret = cSecret;
	_tKey = tKey;
	_tSecret = tSecret;
}