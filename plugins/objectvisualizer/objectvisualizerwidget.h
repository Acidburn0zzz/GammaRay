/*
  This file is part of GammaRay, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2010-2013 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Kevin Funk <kevin.funk@kdab.com>

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

#ifndef GAMMARAY_OBJECTVISUALIZER_OBJECTVISUALIZERWIDGET_H
#define GAMMARAY_OBJECTVISUALIZER_OBJECTVISUALIZERWIDGET_H

#include <QWidget>

class QTreeView;
class QModelIndex;

namespace GammaRay {

class GraphWidget;

class GraphViewerWidget : public QWidget
{
  Q_OBJECT
  public:
    explicit GraphViewerWidget(QWidget *parent = 0);
    virtual ~GraphViewerWidget();

  private Q_SLOTS:
    void delayedInit();
    void handleRowChanged(const QModelIndex &index);

    void objectRowsInserted(const QModelIndex &parent, int start, int end);
    void objectRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);

  private:
    QTreeView *mObjectTreeView;
    GraphWidget *mWidget;
};

}

#endif // GAMMARAY_GRAPHVIEWER_H
