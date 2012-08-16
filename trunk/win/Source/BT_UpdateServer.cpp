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
#include "BT_UpdateServer.h"
#include "BT_FileTransferManager.h"
#include "BT_QtUtil.h"

const char* DownloadFailedException::what() const throw()
{
	return "Downloading the BumpTop update failed";
}

UpdateServerImpl::UpdateServerImpl(QString downloadDirPath)
{
	_messageFileUrl = "http://download.bumptop.com/message.txt";
	_versionFileUrl = "http://download.bumptop.com/v4_version.txt";
	_descFileUrl = "http://download.bumptop.com/v4_desc.txt";
	_md5FileUrl = "http://download.bumptop.com/v4_md5.txt";
	_pointerToBumptopInstallerUrl = "http://download.bumptop.com/v4_url.txt";
	_bumptopInstallerFilename = "BumpTopInstaller.exe";
	_downloadDirPath = downloadDirPath;
}

QString UpdateServerImpl::getVersionString()
{
	// Download the version file from our servers

	
	FileTransfer versionFileTransfer(FileTransfer::Download, 0);
	versionFileTransfer
		.setUrl(_versionFileUrl)
		.setTemporaryString();
	bool success = ftManager->addTransfer(versionFileTransfer);
	if (!success) return "";
	return versionFileTransfer.getTemporaryString();
}

QString UpdateServerImpl::getUpdateMessageString()
{
	QString existingMessagePath = _downloadDirPath + "\\message.txt";
	QString existingMessage;
	if (exists(existingMessagePath))
		existingMessage = read_file_utf8(existingMessagePath);

	FileTransfer messageFileTransfer(FileTransfer::Download, 0);
	messageFileTransfer
		.setUrl(_messageFileUrl)
		.setTemporaryString();
	if (ftManager->addTransfer(messageFileTransfer))
	{
		QString newMessage = messageFileTransfer.getTemporaryString();		
		if (existingMessage != newMessage)
		{
			int i = newMessage.indexOf("<error>", 0, Qt::CaseInsensitive);
			int j = newMessage.lastIndexOf("</error>", -1, Qt::CaseInsensitive);
			bool isS3Error = (i > -1) && (j > -1);
			if (!isS3Error)
			{
				QDir().mkpath(native(parent(existingMessagePath)));
				write_file_utf8(newMessage, existingMessagePath);
				return newMessage;
			}
		}		
	}

	return QString();
}

QString UpdateServerImpl::getDownloadedInstallerFile()
{
	if (exists(make_file(_downloadDirPath, "url.txt")))
	{
		return filename(read_file_utf8(native(make_file(_downloadDirPath, "url.txt"))));
	}
	return _bumptopInstallerFilename;
}

bool UpdateServerImpl::onUpdaterCheckHandler()
{
	return true;
}

void UpdateServerImpl::downloadNewVersion()
{
	if (!exists(_downloadDirPath))
		QDir().mkpath(_downloadDirPath);

	if (exists(make_file(_downloadDirPath, "version.txt")))
		QFile(native(make_file(_downloadDirPath, "version.txt"))).remove();

	if (exists(make_file(_downloadDirPath, "desc.txt")))
		QFile(native(make_file(_downloadDirPath, "desc.txt"))).remove();

	if (exists(make_file(_downloadDirPath, "md5.txt")))
		QFile(native(make_file(_downloadDirPath, "md5.txt"))).remove();
	
	if (exists(make_file(_downloadDirPath, "url.txt")))
		QFile(native(make_file(_downloadDirPath, "url.txt"))).remove();
		
	bool success = ftManager->addTransfer(FileTransfer(FileTransfer::Download, 0)
		.setUrl(_pointerToBumptopInstallerUrl)
		.setLocalPath(native(make_file(_downloadDirPath, "url.txt"))));
	if (!success) 
		throw DownloadFailedException();

	success = ftManager->addTransfer(FileTransfer(FileTransfer::Download, 0)
		.setUrl(_descFileUrl)
		.setLocalPath(native(make_file(_downloadDirPath, "desc.txt"))));
	if (!success) 
		throw DownloadFailedException();

	success = ftManager->addTransfer(FileTransfer(FileTransfer::Download, 0)
		.setUrl(_md5FileUrl)
		.setLocalPath(native(make_file(_downloadDirPath, "md5.txt"))));
	if (!success) 
		throw DownloadFailedException();
	QString downloadPath = read_file_utf8(native(make_file(_downloadDirPath, "url.txt")));
	_bumptopInstallerFilename = getDownloadedInstallerFile();

	if (exists(make_file(_downloadDirPath, _bumptopInstallerFilename)))
		QFile(native(make_file(_downloadDirPath, _bumptopInstallerFilename))).remove();

	success = ftManager->addTransfer(FileTransfer(FileTransfer::Download, 0)
		.setUrl(downloadPath)
		.setLocalPath(native(make_file(_downloadDirPath, _bumptopInstallerFilename))));
	if (!success) 
		throw DownloadFailedException();

	// read the md5 file into a string
	QString downloadedHexDigest = read_file_utf8((_downloadDirPath + "\\md5.txt"));

	// check the MD5 hash
	static const int bufSize = 1024*1024;
	QFile installer(native(make_file(_downloadDirPath, _bumptopInstallerFilename)));
	if (!installer.open(QFile::ReadOnly))
		throw DownloadFailedException();

	qint64 bytesRead;
	QCryptographicHash md5( QCryptographicHash::Md5 );
	QByteArray data;
	data.resize(bufSize);
	while ( ( bytesRead = installer.read ( data.data(), bufSize ) ) > 0 )
		md5.addData ( data.constData(), bytesRead);
	QString hexDigest = QString(md5.result().toHex());

	if (hexDigest != downloadedHexDigest)
		throw DownloadFailedException();

	// we download version last because that way we can ensure, if version.txt exists and is
	// the correct version number, the bumptop installer downloaded completely
	success = ftManager->addTransfer(FileTransfer(FileTransfer::Download, 0)
		.setUrl(_versionFileUrl)
		.setLocalPath(_downloadDirPath + "\\version.txt"));

	if (!success) 
		throw DownloadFailedException();

	return;
}


StagingUpdateServer::StagingUpdateServer(QString downloadDirPath)
: UpdateServerImpl(downloadDirPath)
{
	_messageFileUrl = "http://bumptop.com/download/staging/message.txt";
	_versionFileUrl = "http://bumptop.com/download/staging/version.txt";
	_descFileUrl = "http://bumptop.com/download/staging/desc.txt";
	_md5FileUrl = "http://bumptop.com/download/staging/md5.txt";
	_pointerToBumptopInstallerUrl = "http://bumptop.com/download/staging/url.txt";

}

VIP_UpdateServer::VIP_UpdateServer(QString downloadDirPath)
: UpdateServerImpl(downloadDirPath)
{
	_messageFileUrl = "http://download.bumptop.com/vip_message.txt";
	_versionFileUrl = "http://download.bumptop.com/vip_v4_version.txt";
	_descFileUrl = "http://download.bumptop.com/vip_v4_desc.txt";
	_md5FileUrl = "http://download.bumptop.com/vip_v4_md5.txt";
	_pointerToBumptopInstallerUrl = "http://download.bumptop.com/vip_v4_url.txt";
}