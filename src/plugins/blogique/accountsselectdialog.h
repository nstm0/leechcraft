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

#include <QDialog>
#include "ui_accountsselectdialog.h"

class QStandardItem;
class QStandardItemModel;

namespace LeechCraft
{
namespace Blogique
{
	class IAccount;

	class AccountsSelectDialog : public QDialog
	{
		Q_OBJECT

		Ui::AccountsSelectDialog Ui_;

		QStandardItemModel *Model_;
		QHash<QStandardItem*, IAccount*> Item2Accotun_;

	public:
		AccountsSelectDialog (QWidget *parent = 0);

		void FillAccounts (const QList<IAccount*>& accounts);
		QList<IAccount*> GetSelectedAccounts () const;
	};
}
}

