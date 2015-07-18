/*
  aggregatedpropertymodel.cpp

  This file is part of GammaRay, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2014-2015 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Volker Krause <volker.krause@kdab.com>

  Licensees holding valid commercial KDAB GammaRay licenses may use this file in
  accordance with GammaRay Commercial License Agreement provided with the Software.

  Contact info@kdab.com if any conditions of this licensing are not clear to you.

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

#include "aggregatedpropertymodel.h"
#include "metaobjectrepository.h"
#include "objectinstance.h"
#include "probe.h"
#include "propertyaggregator.h"
#include "propertydata.h"
#include "propertyadaptorfactory.h"
#include "toolfactory.h"
#include "toolmodel.h"
#include "varianthandler.h"

#include <common/propertymodel.h>

#include <QDebug>

using namespace GammaRay;

AggregatedPropertyModel::AggregatedPropertyModel(QObject* parent) :
    QAbstractItemModel(parent),
    m_rootAdaptor(0)
{
}

AggregatedPropertyModel::~AggregatedPropertyModel()
{
}

void AggregatedPropertyModel::setObject(const ObjectInstance& oi)
{
    clear();

    auto adaptor = PropertyAdaptorFactory::create(oi, this);
    auto count = adaptor->count();
    if (count)
        beginInsertRows(QModelIndex(), 0, count -1);

    m_rootAdaptor = adaptor;
    addPropertyAdaptor(m_rootAdaptor);

    if (count)
        endInsertRows();
}

void AggregatedPropertyModel::clear()
{
    if (!m_rootAdaptor)
        return;

    const auto count = m_parentChildrenMap.value(m_rootAdaptor).size();
    if (count)
        beginRemoveRows(QModelIndex(), 0, count - 1);

    m_parentChildrenMap.clear();
    delete m_rootAdaptor;
    m_rootAdaptor = 0;

    if (count)
        endRemoveRows();
}

QVariant AggregatedPropertyModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_rootAdaptor)
        return QVariant();

    const auto adaptor = adaptorForIndex(index);
    const auto d = adaptor->propertyData(index.row());
    return data(d, index.column(), role);
}

QMap<int, QVariant> AggregatedPropertyModel::itemData(const QModelIndex& index) const
{
    QMap<int, QVariant> res;
    if (!index.isValid() || !m_rootAdaptor)
        return res;

    const auto adaptor = adaptorForIndex(index);
    const auto d = adaptor->propertyData(index.row());

    res.insert(Qt::DisplayRole, data(d, index.column(), Qt::DisplayRole));
    res.insert(Qt::ToolTipRole, data(d, index.column(), Qt::ToolTipRole));
    res.insert(PropertyModel::ActionRole, data(d, index.column(), PropertyModel::ActionRole));
    res.insert(PropertyModel::AppropriateToolRole, data(d, index.column(), PropertyModel::AppropriateToolRole));
    if (index.column() == 1) {
        res.insert(Qt::EditRole, data(d, index.column(), Qt::EditRole));
        res.insert(Qt::DecorationRole, data(d, index.column(), Qt::DecorationRole));
    }
    return res;
}

QVariant AggregatedPropertyModel::data(const PropertyData& d, int column, int role) const
{
    switch (role) {
        case Qt::DisplayRole:
            switch (column) {
                case 0:
                    return d.name();
                case 1:
                    return VariantHandler::displayString(d.value());
                case 2:
                    return d.typeName();
                case 3:
                    return d.className();
            }
            break;
        case Qt::EditRole:
            if (column == 1)
                return VariantHandler::serializableVariant(d.value());
            break;
        case Qt::ToolTipRole:
            return d.details();
        case Qt::DecorationRole:
            if (column == 1)
                return VariantHandler::decoration(d.value());
            break;
        case PropertyModel::ActionRole:
        {
            int actions = PropertyModel::NoAction;
            if (d.flags() & PropertyData::Resettable)
                actions |= PropertyModel::Reset;
            if (d.flags() & PropertyData::Deletable)
                actions |= PropertyModel::Delete;
            if ((MetaObjectRepository::instance()->metaObject(d.typeName()) && *reinterpret_cast<void* const*>(d.value().data())) || d.value().value<QObject*>())
                actions |= PropertyModel::NavigateTo;
            return actions;
        }
        case PropertyModel::ValueRole:
            return d.value();
        case PropertyModel::AppropriateToolRole:
        {
            ToolModel *toolModel = Probe::instance()->toolModel();
            ToolFactory *factory;
            if (d.value().canConvert<QObject*>())
                factory = toolModel->data(toolModel->toolForObject(d.value().value<QObject*>()), ToolModelRole::ToolFactory).value<ToolFactory*>();
            else
                factory = toolModel->data(toolModel->toolForObject(*reinterpret_cast<void* const*>(d.value().data()), d.value().typeName()), ToolModelRole::ToolFactory).value<ToolFactory*>();
            if (factory) {
                return factory->name();
            }
            return QVariant();
        }
    }

    return QVariant();
}

bool AggregatedPropertyModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || !m_rootAdaptor || role != Qt::EditRole || index.column() != 1)
        return false;

    const auto adaptor = adaptorForIndex(index);
    adaptor->writeProperty(index.row(), value);
    return true;
}

int AggregatedPropertyModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 4;
}

int AggregatedPropertyModel::rowCount(const QModelIndex& parent) const
{
    if (!m_rootAdaptor|| parent.column() > 0)
        return 0;
    if (!parent.isValid())
        return m_parentChildrenMap.value(m_rootAdaptor).size();

    auto adaptor = adaptorForIndex(parent);
    auto& siblings = m_parentChildrenMap[adaptor];
    if (!siblings.at(parent.row())) {
      // TODO: remember we tried if this returns 0
      auto a = PropertyAdaptorFactory::create(adaptor->propertyData(parent.row()).value(), adaptor);
      siblings[parent.row()] = a;
      addPropertyAdaptor(a);
    }
    auto childAdaptor = siblings.at(parent.row());
    if (!childAdaptor)
        return 0;
    return m_parentChildrenMap.value(childAdaptor).size(); // childAdaptor->count() might already be updated in insert/removeRows
}

Qt::ItemFlags AggregatedPropertyModel::flags(const QModelIndex& index) const
{
    const auto baseFlags = QAbstractItemModel::flags(index);
    if (!index.isValid() || index.column() != 1)
        return baseFlags;

    auto adaptor = adaptorForIndex(index);
    auto data = adaptor->propertyData(index.row());
    return (data.flags() & PropertyData::Writable) ? (baseFlags | Qt::ItemIsEditable) : baseFlags;
}

QVariant AggregatedPropertyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return tr("Property");
            case 1:
                return tr("Value");
            case 2:
                return tr("Type");
            case 3:
                return tr("Class");
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

QModelIndex AggregatedPropertyModel::parent(const QModelIndex& child) const
{
    auto childAdaptor = adaptorForIndex(child);
    if (childAdaptor == m_rootAdaptor)
        return QModelIndex();

    auto parentAdaptor = qobject_cast<PropertyAdaptor*>(childAdaptor->parent());
    return createIndex(m_parentChildrenMap.value(parentAdaptor).indexOf(childAdaptor), 0, parentAdaptor);
}

QModelIndex AggregatedPropertyModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent) || !m_rootAdaptor)
        return QModelIndex();

    if (!parent.isValid())
        return createIndex(row, column, m_rootAdaptor);
    auto adaptor = adaptorForIndex(parent);
    return createIndex(row, column, m_parentChildrenMap.value(adaptor).at(parent.row()));
}

PropertyAdaptor* AggregatedPropertyModel::adaptorForIndex(const QModelIndex& index) const
{
    if (!index.isValid())
        return m_rootAdaptor;
    return static_cast<PropertyAdaptor*>(index.internalPointer());
}

void AggregatedPropertyModel::addPropertyAdaptor(PropertyAdaptor* adaptor) const
{
    if (!adaptor)
        return;
    QVector<PropertyAdaptor*> children;
    children.resize(adaptor->count());
    m_parentChildrenMap.insert(adaptor, children);
    connect(adaptor, SIGNAL(propertyChanged(int,int)), this, SLOT(propertyChanged(int,int)));
    connect(adaptor, SIGNAL(propertyAdded(int,int)), this, SLOT(propertyAdded(int,int)));
    connect(adaptor, SIGNAL(propertyRemoved(int,int)), this, SLOT(propertyRemoved(int,int)));
}

void AggregatedPropertyModel::propertyChanged(int first, int last)
{
    auto adaptor = qobject_cast<PropertyAdaptor*>(sender());
    Q_ASSERT(adaptor);
    Q_ASSERT(m_parentChildrenMap.contains(adaptor));
    Q_ASSERT(first <= last);
    Q_ASSERT(first >= 0);
    Q_ASSERT(last < adaptor->count());

    emit dataChanged(createIndex(first, 0, adaptor), createIndex(last, columnCount() - 1, adaptor));
}

void AggregatedPropertyModel::propertyAdded(int first, int last)
{
    auto adaptor = qobject_cast<PropertyAdaptor*>(sender());
    Q_ASSERT(adaptor);
    Q_ASSERT(m_parentChildrenMap.contains(adaptor));
    Q_ASSERT(first <= last);
    Q_ASSERT(first >= 0);
    Q_ASSERT(last < adaptor->count());

    auto idx = createIndex(first, 0, adaptor);
    beginInsertRows(idx.parent(), first, last);
    auto &children = m_parentChildrenMap[adaptor];
    if (first >= children.size())
        children.resize(last + 1);
    else
      children.insert(first, last - first + 1, 0);
    endInsertRows();
}

void AggregatedPropertyModel::propertyRemoved(int first, int last)
{
    auto adaptor = qobject_cast<PropertyAdaptor*>(sender());
    Q_ASSERT(adaptor);
    Q_ASSERT(m_parentChildrenMap.contains(adaptor));
    Q_ASSERT(first <= last);
    Q_ASSERT(first >= 0);
    Q_ASSERT(last < adaptor->count());

    auto idx = createIndex(first, 0, adaptor);
    beginRemoveRows(idx.parent(), first, last);
    auto &children = m_parentChildrenMap[adaptor];
    children.remove(first, last - first + 1);
    endRemoveRows();
}
