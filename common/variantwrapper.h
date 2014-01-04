/*
  variantwrapper.h

  This file is part of GammaRay, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2014 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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

#ifndef GAMMARAY_VARIANTWRAPPER_H
#define GAMMARAY_VARIANTWRAPPER_H

#include <QDataStream>
#include <QVariant>

namespace GammaRay {

/** Transport wrapper for variants that shall not be unpacked by GammaRay::MethodArgument. */
class VariantWrapper
{
public:
  inline VariantWrapper() {}
  explicit inline VariantWrapper(const QVariant &variant) : m_variant(variant) {}
  inline ~VariantWrapper() {}

  inline QVariant variant() const { return m_variant; }
  inline void setVariant(const QVariant &v) { m_variant = v; }
  inline operator QVariant() const { return QVariant::fromValue(*this); }

private:
  QVariant m_variant;
};

inline QDataStream &operator<<(QDataStream &out, const VariantWrapper& wrapper)
{
  out << wrapper.variant();
  return out;
}

inline QDataStream &operator>>(QDataStream &in, VariantWrapper &value)
{
  QVariant v;
  in >> v;
  value.setVariant(v);
  return in;
}

}

Q_DECLARE_METATYPE(GammaRay::VariantWrapper)

#endif