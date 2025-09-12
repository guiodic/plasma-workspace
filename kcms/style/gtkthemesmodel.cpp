/*
    SPDX-FileCopyrightText: 2020 Mikhail Zolotukhin <zomial@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QUrl>

#include <KIO/DeleteJob>

#include "gtkthemesmodel.h"

GtkThemesModel::GtkThemesModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_selectedTheme(QStringLiteral("Breeze"))
    , m_themes()
{
}

void GtkThemesModel::load()
{
    QMap<QString, QString> gtk3ThemesNames;

    static const QStringList gtk3SubdirPattern(QStringLiteral("gtk-3.*"));
    for (const QString &possibleThemePath : possiblePathsToThemes()) {
        // If the directory contains any of gtk-3.X folders, it is the GTK3 theme for sure
        QDir possibleThemeDirectory(possibleThemePath);
        if (!possibleThemeDirectory.entryList(gtk3SubdirPattern, QDir::Dirs).isEmpty()) {
            // Do not show dark Breeze GTK variant, since the colors of it
            // are coming from the color scheme and selecting them here
            // is redundant and does not work
            if (possibleThemeDirectory.dirName() == u"Breeze-Dark") {
                continue;
            }
            if (possibleThemeDirectory.dirName() == u"Default") {
                // Adwaita is a special case, since it is implemented inside GTK itself
                // also setting gtk-theme-name to "Default" breaks dark theme
                gtk3ThemesNames.insert(QStringLiteral("Adwaita"), possibleThemeDirectory.path());
                continue;
            }
            gtk3ThemesNames.insert(possibleThemeDirectory.dirName(), possibleThemeDirectory.path());
        }
    }

    setThemesList(gtk3ThemesNames);
}

QString GtkThemesModel::themePath(const QString &themeName)
{
    if (themeName.isEmpty()) {
        return {};
    } else {
        return m_themes.constFind(themeName).value();
    }
}

QVariant GtkThemesModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return {};
    }

    auto it = m_themes.constBegin();
    std::advance(it, index.row());

    switch (role) {
    case Qt::DisplayRole:
    case Roles::ThemeNameRole:
        return it.key();
    case Roles::ThemePathRole:
        return it.value();
    default:
        return {};
    }
}

QHash<int, QByteArray> GtkThemesModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[Roles::ThemeNameRole] = QByteArrayLiteral("theme-name");
    roles[Roles::ThemePathRole] = QByteArrayLiteral("theme-path");

    return roles;
}

int GtkThemesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_themes.count();
}

void GtkThemesModel::setThemesList(const QMap<QString, QString> &themes)
{
    beginResetModel();
    m_themes = themes;
    endResetModel();
}

QMap<QString, QString> GtkThemesModel::themesList()
{
    return m_themes;
}

void GtkThemesModel::setSelectedTheme(const QString &themeName)
{
    m_selectedTheme = themeName;
    Q_EMIT selectedThemeChanged(themeName);
}

QString GtkThemesModel::selectedTheme()
{
    return m_selectedTheme;
}

QStringList GtkThemesModel::possiblePathsToThemes()
{
    QStringList possibleThemesPaths;

    QStringList themesLocationsPaths =
        QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("themes"), QStandardPaths::LocateDirectory);
    // TODO: Remove. See https://gitlab.gnome.org/GNOME/gtk/-/issues/6219 for when GTK plans to, and when we should,
    // remove it as well.
    themesLocationsPaths << QDir::homePath() + QStringLiteral("/.themes");

    for (const QString &themesLocationPath : std::as_const(themesLocationsPaths)) {
        const QStringList possibleThemesDirectoriesNames = QDir(themesLocationPath).entryList(QDir::NoDotAndDotDot | QDir::AllDirs);
        for (const QString &possibleThemeDirectoryName : possibleThemesDirectoriesNames) {
            possibleThemesPaths += themesLocationPath + u'/' + possibleThemeDirectoryName;
        }
    }

    return possibleThemesPaths;
}

bool GtkThemesModel::selectedThemeRemovable()
{
    return themePath(m_selectedTheme).contains(QDir::homePath());
}

void GtkThemesModel::removeSelectedTheme()
{
    QString path = themePath(m_selectedTheme);
    KIO::DeleteJob *deleteJob = KIO::del(QUrl::fromLocalFile(path), KIO::HideProgressInfo);
    connect(deleteJob, &KJob::finished, this, [this]() {
        Q_EMIT themeRemoved();
    });
}

int GtkThemesModel::findThemeIndex(const QString &themeName)
{
    return static_cast<int>(std::distance(m_themes.constBegin(), m_themes.constFind(themeName)));
}

void GtkThemesModel::setSelectedThemeDirty()
{
    Q_EMIT selectedThemeChanged(m_selectedTheme);
}

#include "moc_gtkthemesmodel.cpp"
