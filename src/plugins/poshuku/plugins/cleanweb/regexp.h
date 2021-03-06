/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include <memory>
#include <QString>

#if USE_PCRE
// nothing yet
#else
#include <QRegExp>
#endif

namespace LeechCraft
{
namespace Poshuku
{
namespace CleanWeb
{
#if USE_PCRE
	class PCREWrapper;
#endif

	class RegExp
	{
#if USE_PCRE
		QString Pattern_;
		Qt::CaseSensitivity CS_;

		std::shared_ptr<PCREWrapper> PRx_;
#else
		QRegExp Rx_;
#endif
	public:
		static bool IsFast ();

		RegExp ();
		RegExp (const RegExp&);
		RegExp (const QString&, Qt::CaseSensitivity);
		~RegExp ();
		RegExp& operator= (const RegExp&);

		bool Matches (const QString&) const;

		QString GetPattern () const;
		Qt::CaseSensitivity GetCaseSensitivity () const;
	};
}
}
}
