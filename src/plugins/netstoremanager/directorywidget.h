/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_directorywidget.h"

namespace LeechCraft
{
namespace NetStoreManager
{
	class DirectoryWidget : public QWidget
	{
		Q_OBJECT

		Ui::SelectDirectoryWidget Ui_;
		QString Path_;

	public:
		DirectoryWidget (QWidget *parent = 0);

		void SetPath (const QString& path, bool byHand = false);
		QString GetPath () const;

	private slots:
		void on_OpenDir__released ();
		void handleEditingFinished ();

	signals:
		void finished (QWidget *widget);
	};
}
}
