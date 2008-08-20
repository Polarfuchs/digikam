/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : A widget stack to embed editor view.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef EDITORSTACKVIEW_H
#define EDITORSTACKVIEW_H

// KDE includes.

#include <qwidgetstack.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class Canvas;
class EditorStackViewPriv;

class DIGIKAM_EXPORT EditorStackView : public QWidgetStack
{
Q_OBJECT

public:

    enum StackViewMode
    {
        CanvasMode=0,
        ToolViewMode
    };

public:

    EditorStackView(QWidget *parent=0);
    ~EditorStackView();

    void setCanvas(Canvas* canvas);
    Canvas *canvas() const;

    void setToolView(QWidget* view);
    QWidget *toolView() const;

    int  viewMode();
    void setViewMode(int mode);

private:

    EditorStackViewPriv* d;
};

}  // namespace Digikam

#endif /* EDITORSTACKVIEW_H */
