/*
  clientconnectionmanager.h

  This file is part of GammaRay, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2013-2015 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Volker Krause <volker.krause@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GAMMARAY_CLIENTCONNECTIONMANAGER_H
#define GAMMARAY_CLIENTCONNECTIONMANAGER_H

#include "gammaray_client_export.h"

#include <QObject>
#include <QTime>
#include <QUrl>

class QAbstractItemModel;

namespace GammaRay {

class Client;
class MainWindow;

/** @brief Pre-MainWindow connection setup logic.
 *
 * This is useful for embedding the GammaRay client into another application
 *
 * @since 2.3
 */
class GAMMARAY_CLIENT_EXPORT ClientConnectionManager : public QObject
{
  Q_OBJECT
  public:
    explicit ClientConnectionManager(QObject* parent = 0);
    ~ClientConnectionManager();

    /** Connect to a GammaRay probe at @p url. */
    void connectToHost(const QUrl &url);

    /** One-time initialization of stream operators and factory callbacks. */
    static void init();

  signals:
    void ready();

  public slots:
    void createMainWindow();

  private slots:
    void connectToHost();
    void connectionEstablished();
    void transientConnectionError();
    void persistentConnectionError( const QString &msg);

    void toolModelPopulated();

  private:
    QUrl m_serverUrl;
    Client *m_client;
    MainWindow *m_mainWindow;
    QAbstractItemModel *m_toolModel;
    QTime m_connectionTimeout;
};

}

#endif // GAMMARAY_CLIENTCONNECTIONMANAGER_H
