/*
 * Copyright (c) 2011-2015 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/Sheet>
#include <bb/cascades/LocaleHandler>

#include "applicationui.hpp"
#include "settings.hpp"
#include "database.hpp"


using namespace bb::cascades;

ApplicationUI :: ApplicationUI() : QObject(), m_dataModel(0) {
    m_pTranslator = new QTranslator(this);
    m_pLocaleHandler = new LocaleHandler(this);
    settings = new Settings(this);
    database = new Database(this);
    if (isFirstStart()) { initializeApplication(); }
    readApplicationSettings();
    readCodeList();
    bool res = QObject::connect(m_pLocaleHandler, SIGNAL(systemLanguageChanged()), this, SLOT(onSystemLanguageChanged()));
    Q_ASSERT(res);
    Q_UNUSED(res);
    onSystemLanguageChanged();
    QmlDocument *qml = QmlDocument::create("asset:///pages/MainPage.qml").parent(this);
    AbstractPane *root = qml->createRootObject<AbstractPane>();
    Application::instance()->setScene(root);
    qml->setContextProperty("_settings", settings);
    qml->setContextProperty("_database", database);
    qml->setContextProperty("_app", this);
}

void ApplicationUI :: onSystemLanguageChanged() {
    QCoreApplication::instance()->removeTranslator(m_pTranslator);
    QString locale_string = QLocale().name();
    QString file_name = QString("GoogleAuthenticator_%1").arg(locale_string);
    if (m_pTranslator->load(file_name, "app/native/qm")) {
        QCoreApplication::instance()->installTranslator(m_pTranslator);
    }
}

bool ApplicationUI :: isFirstStart() {
    return settings->isFirstStart();
}

bool ApplicationUI :: initializeApplication() {
    bool success = false;
    if (database->initializeDatabase()){
        if (settings->initializeSettings()){
            success = true;
        }
    }
    database->createRecord();
    return success;
}

bool ApplicationUI :: readApplicationSettings() {
    bool success = false;
    return success;
}

void ApplicationUI :: initializeDataModel()
{
    m_dataModel = new GroupDataModel(this);
    m_dataModel->setSortingKeys(QStringList() << "title");
    m_dataModel->setGrouping(ItemGrouping::None);
}

bool ApplicationUI :: readCodeList() {
    bool success = false;
    initializeDataModel();
    m_dataModel->clear();
    QVariant result = database->readRecords();
    if (!result.isNull()) {
        QVariantList list = result.value<QVariantList>();
        int recordsListSize = list.size();
        for (int i = 0; i < recordsListSize; i++) {
            QVariantMap map = list.at(i).value<QVariantMap>();
            //CodeListItem *listItem = new CodeListItem();
            //m_dataModel->insert(listItem);
        }
        success = true;
    }
    return success;
}

GroupDataModel* ApplicationUI :: getDataModel() const
{
    return m_dataModel;
}

void ApplicationUI :: parseBarcodeData(const QString& data) {
    QUrl url(data);
    if (url.scheme().toAscii() == "otpauth") {
        QString authType = url.host();
        QString urlPath = url.path();
        QString secterKey = url.queryItemValue("secret");
        QString issuerTitle;
        if (url.hasQueryItem("issuer")) {
            issuerTitle = url.queryItemValue("issuer");
        }
        //QString algorithmType;
        //if (url.hasQueryItem("algorithm")) {
        //    algorithmType = url.queryItemValue("algorithm");
        //}
        QString keyLenght;
        if (url.hasQueryItem("digits")) {
            keyLenght = url.queryItemValue("digits");
        }
        QString counterValue;
        if (url.hasQueryItem("counter")) {
            counterValue = url.queryItemValue("counter");
        }
        QString periodTime;
        if (url.hasQueryItem("period")) {
            periodTime = url.queryItemValue("period");
        }
        Sheet* sheet = new Sheet;
        Page* page = QmlDocument::create("asset:///pages/AddCodePage.qml");
        page->issuerTitle = issuerTitle;
        page->accountName = accountName;
        page->secretKey = secretKey;
        page->keyLenght.setSelectedOption(keyLenght);
        page->authType.setSelectedOption(authType);
    }
}

void alert(const QString &message) {
    SystemDialog *dialog;
    dialog = new SystemDialog(tr("OK"), 0);
    dialog->setTitle(tr("Alert"));
    dialog->setBody(message);
    dialog->setDismissAutomatically(true);
    bool ok = connect(dialog, SIGNAL(finished(bb::system::SystemUiResult::Type)), dialog, SLOT(deleteLater()));
    Q_ASSERT(ok);
    dialog->show();
}
