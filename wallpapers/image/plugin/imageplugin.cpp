/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "imageplugin.h"
#include <QQmlContext>

#include <KFileItem>

#include "daynightwallpaper.h"
#include "finder/mediametadatafinder.h"
#include "imagebackend.h"
#include "provider/packageimageprovider.h"
#include "provider/wallpaperpreviewimageprovider.h"
#include "sortingmode.h"
#include "utils/maximizedwindowmonitor.h"
#include "utils/mediaproxy.h"
#include "utils/wallpaperurl.h"

void ImagePlugin::initializeEngine(QQmlEngine *engine, const char *)
{
    engine->addImageProvider(QStringLiteral("package"), new PackageImageProvider);
    engine->addImageProvider(QStringLiteral("wallpaper-preview"), new WallpaperPreviewImageProvider);
}

void ImagePlugin::registerTypes(const char *uri)
{
    qRegisterMetaType<KFileItem>(); // For image preview
    qRegisterMetaType<MediaMetadata>(); // For image preview
}
