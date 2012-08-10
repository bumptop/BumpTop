/*
 *  Copyright 2012 Google Inc. All Rights Reserved.
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <string>
#include <utility>

#include "BumpTop/Authorization.h"
#include "BumpTop/BumpTopApp.h"
#include "BumpTop/FileManager.h"
#include "BumpTop/ProtocolBufferHelpers.h"
#include "BumpTop/QStringHelpers.h"
#include "BumpTop/RC4.h"

#include "ThirdParty/GetPrimaryMacAddress.h"

SINGLETON_IMPLEMENTATION(ProAuthorization)

ProAuthorization::ProAuthorization()
: Authorization() {
}

bool ProAuthorization::parseFromString(std::string data) {
  return ParseFromString(data);
}

QString ProAuthorization::config_file_path() {
  return FileManager::getResourcePath() + "/BumpTopPro.config";
}

QString ProAuthorization::license_file_path() {
  return FileManager::getApplicationDataPath() + "bumppro.license";
}

std::pair<QString, QString> ProAuthorization::auth_script_web_host_and_path() {
  return std::pair<QString, QString>("bumptop.com", "/authorization/authorize_macos_pro.php");
}

QString ProAuthorization::account_type() {
  return "PRO";
}

Authorization::Authorization()
: authorized_(false) {
}

void Authorization::init() {
  license_ = new License();
}

bool Authorization::loadLicenseFile() {
  return loadBufferFromFile(license_, license_file_path());
}

bool Authorization::deleteLicenseFile() {
  return QFile(license_file_path()).remove();
}

BoolAndQString Authorization::registerOverInternet(QString invite_code) {
  QRegExp eight_digit_hexadecimal("[a-f0-9]{8}");
  if (!eight_digit_hexadecimal.exactMatch(invite_code))
    return BoolAndQString(false, "Invite code is invalid. Please double-check your entry.");
  http_ = new QHttp();
  std::pair<QString, QString> web_host_and_path = auth_script_web_host_and_path();
  http_->setHost(web_host_and_path.first);
  assert(QObject::connect(http_, SIGNAL(requestFinished(int, bool)), this, SLOT(requestFinished(int, bool))));
  download_finished_ = false;
  download_return_value_ = BoolAndQString(false, "");
  BoolAndQString bool_and_mac_address = GetPrimaryMacAddressString();

  if (bool_and_mac_address.first) {  // if successful
    QString mac_address = bool_and_mac_address.second;
    http_request_id_ = http_->get(web_host_and_path.second + "?mac_addresses=" + mac_address +
                                  "&invite_code=" + invite_code + "&account_type=" + account_type());

    while (!download_finished_)
      qApp->processEvents();

    return download_return_value_;
  } else {
    return BoolAndQString(false,
                          "Internal authorization error (error code 1). Please contact mac-feedback@bumptop.com.");
  }
}

void Authorization::requestFinished(int request_id, bool error) {
  if (request_id == http_request_id_) {
    download_finished_ = true;
    if (error) {
      download_return_value_ = BoolAndQString(false,
                                              "Could not connect to BumpTop authorization server. Please check your internet.");  // NOLINT
      // TODO: Check if, say, google.com can be loaded
      return;
    }
    QStringList server_response = QString(http_->readAll()).trimmed().split("|");

    if (server_response.size() == 4 && server_response[0] == "success") {
      QString salt = server_response[2];
      QByteArray ciphered_master_key = QByteArray::fromHex(server_response[3].toUtf8());
      license_->set_license_key_1(salt.toStdString());
      // must explicitly construct this as a std::string or it might be truncated
      license_->set_license_key_2(std::string(ciphered_master_key.data(), ciphered_master_key.size()));
      // TODO: delete http object
      download_return_value_ = BoolAndQString(true, "");
    } else {
      // Make sure we got an error message that doesn't include HTML; if it doesn't, it's an internal error
      if (server_response.size() > 0 && server_response[0].indexOf("<") == -1 && server_response[0].indexOf(">") && -1)
        download_return_value_ = BoolAndQString(false, server_response[0]);
      else
        download_return_value_ = BoolAndQString(false,
                                                "Internal authorization error (error code 2). Please contact mac-feedback@bumptop.com.");  // NOLINT
    }
  }
}

bool Authorization::saveLicenseFile() {
  return saveBufferToFile(license_, license_file_path());
}

bool Authorization::authorized() {
  return authorized_;
}

void Authorization::set_authorized(bool value) {
  authorized_ = value;
}

BoolAndQString Authorization::decipherAndLoadConfigFile() {
  std::pair<bool, QString> bool_and_mac_address = GetPrimaryMacAddressString();
  if (bool_and_mac_address.first) {  // success
    QString mac_address = bool_and_mac_address.second;
    std::string salt = license_->license_key_1();


    QCryptographicHash sha1_hasher(QCryptographicHash::Sha1);

    sha1_hasher.addData(salt.data());
    sha1_hasher.addData(mac_address.toUtf8());

    //QByteArray salted_and_hashed_mac_address = sha1_hasher.result();

    //QByteArray ciphered_master_key = QByteArray(license_->license_key_2().data(), license_->license_key_2().size());
    //QByteArray master_key = RC4Decipher(salted_and_hashed_mac_address.toHex(), ciphered_master_key);

    QFile file(config_file_path());
    if (!file.open(QIODevice::ReadOnly))
      return std::pair<bool, QString>(false,
                                      "Internal authorization error (error code 3). Please contact mac-feedback@bumptop.com.");  // NOLINT
    QByteArray ciphered_config_file = file.readAll();

    //QByteArray config_file = RC4Decipher(master_key, ciphered_config_file);
    bool successful_parse = parseFromString(std::string(ciphered_config_file.data(), ciphered_config_file.size()));
    file.close();
    if (successful_parse) {
      authorized_ = true;
      return BoolAndQString(true, "");
    } else {
      return BoolAndQString(false,
                            "Internal authorization error (error code 4). Please contact mac-feedback@bumptop.com.");
    }
  } else {
    return BoolAndQString(false,
                          "Internal authorization error (error code 5). Please contact mac-feedback@bumptop.com.");
  }
}

QString Authorization::key() {
  return QStringFromUtf8(license_->key());
}

#include "moc/moc_Authorization.cpp"
