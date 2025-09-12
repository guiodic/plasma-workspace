/*
    SPDX-FileCopyrightText: 2003-2009 Craig Drummond <craig@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Style.h"
#include "Fc.h"
#include "WritingSystems.h"
#include "XmlStrings.h"
#include <QDomElement>
#include <QStringList>
#include <QTextStream>

using namespace Qt::StringLiterals;

namespace KFI
{
Style::Style(const QDomElement &elem, bool loadFiles)
{
    bool ok;
    int weight(KFI_NULL_SETTING), width(KFI_NULL_SETTING), slant(KFI_NULL_SETTING), tmp(KFI_NULL_SETTING);

    if (elem.hasAttribute(WEIGHT_ATTR)) {
        tmp = elem.attribute(WEIGHT_ATTR).toInt(&ok);
        if (ok) {
            weight = tmp;
        }
    }
    if (elem.hasAttribute(WIDTH_ATTR)) {
        tmp = elem.attribute(WIDTH_ATTR).toInt(&ok);
        if (ok) {
            width = tmp;
        }
    }

    if (elem.hasAttribute(SLANT_ATTR)) {
        tmp = elem.attribute(SLANT_ATTR).toInt(&ok);
        if (ok) {
            slant = tmp;
        }
    }

    m_scalable = !elem.hasAttribute(SCALABLE_ATTR) || elem.attribute(SCALABLE_ATTR) != u"false";
    m_value = FC::createStyleVal(weight, width, slant);
    m_writingSystems = 0;

    if (elem.hasAttribute(LANGS_ATTR)) {
        m_writingSystems = WritingSystems::instance()->get(elem.attribute(LANGS_ATTR).split(LANG_SEP, Qt::SkipEmptyParts));
    }

    if (loadFiles) {
        if (elem.hasAttribute(PATH_ATTR)) {
            File file(elem, false);

            if (!file.path().isEmpty()) {
                m_files.insert(file);
            }
        } else {
            for (QDomNode n = elem.firstChild(); !n.isNull(); n = n.nextSibling()) {
                QDomElement ent = n.toElement();

                if (FILE_TAG == ent.tagName()) {
                    File file(ent, false);

                    if (!file.path().isEmpty()) {
                        m_files.insert(file);
                    }
                }
            }
        }
    }
}

QString Style::toXml(bool disabled, const QString &family) const
{
    QStringList files;
    FileCont::ConstIterator it(m_files.begin()), end(m_files.end());

    for (; it != end; ++it) {
        QString f((*it).toXml(disabled));

        if (!f.isEmpty()) {
            files.append(f);
        }
    }

    if (files.count() > 0) {
        QString str(u"  <" + FONT_TAG + u' ');
        int weight, width, slant;

        KFI::FC::decomposeStyleVal(m_value, weight, width, slant);

        if (!family.isEmpty()) {
            str += FAMILY_ATTR + u"=\"" + family + u"\" ";
        }
        if (KFI_NULL_SETTING != weight) {
            str += WEIGHT_ATTR + u"=\"" + QString::number(weight) + u"\" ";
        }
        if (KFI_NULL_SETTING != width) {
            str += WIDTH_ATTR + u"=\"" + QString::number(width) + u"\" ";
        }
        if (KFI_NULL_SETTING != slant) {
            str += SLANT_ATTR + u"=\"" + QString::number(slant) + u"\" ";
        }
        if (!m_scalable) {
            str += SCALABLE_ATTR + u"=\"false\" ";
        }

        QStringList ws(WritingSystems::instance()->getLangs(m_writingSystems));

        if (!ws.isEmpty()) {
            str += LANGS_ATTR + u"=\"" + ws.join(LANG_SEP) + u"\" ";
        }

        if (1 == files.count()) {
            str += (*files.begin()) + u"/>";
        } else {
            QStringList::ConstIterator it(files.begin()), end(files.end());

            str += QStringView(u">\n");
            for (; it != end; ++it) {
                str += u"   <" + FILE_TAG + u' ' + (*it) + u"/>\n";
            }
            str += u"  </" + FONT_TAG + u">";
        }

        return str;
    }

    return {};
}

}

QDBusArgument &operator<<(QDBusArgument &argument, const KFI::Style &obj)
{
    argument.beginStructure();

    argument << obj.value() << obj.scalable() << obj.writingSystems();
    argument.beginArray(qMetaTypeId<KFI::File>());
    KFI::FileCont::ConstIterator it(obj.files().begin()), end(obj.files().end());
    for (; it != end; ++it) {
        argument << *it;
    }
    argument.endArray();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, KFI::Style &obj)
{
    quint32 value;
    bool scalable;
    qulonglong ws;
    argument.beginStructure();
    argument >> value >> scalable >> ws;
    obj = KFI::Style(value, scalable, ws);
    argument.beginArray();
    while (!argument.atEnd()) {
        KFI::File f;
        argument >> f;
        obj.add(f);
    }
    argument.endArray();
    argument.endStructure();
    return argument;
}
