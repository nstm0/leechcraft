/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "separatetabwidget.h"
#include <algorithm>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QToolBar>
#include <QMenu>
#include <QMouseEvent>
#include <QLayoutItem>
#include <QLayout>
#include <QShortcut>
#include <QStyle>
#include <QtDebug>
#include <interfaces/ihavetabs.h>
#include "coreproxy.h"
#include "separatetabbar.h"
#include "xmlsettingsmanager.h"
#include "core.h"
#include "util/defaulthookproxy.h"
#include "coreinstanceobject.h"
#include "coreplugin2manager.h"
#include "tabmanager.h"

namespace LeechCraft
{
	SeparateTabWidget::SeparateTabWidget (QWidget *parent)
	: QWidget (parent)
	, LastContextMenuTab_ (-1)
	, MainStackedWidget_ (new QStackedWidget)
	, MainTabBar_ (new SeparateTabBar)
	, AddTabButton_ (new QToolButton)
	, LeftToolBar_ (new QToolBar)
	, RightToolBar_ (new QToolBar)
	, DefaultTabAction_ (new QAction (QString (), this))
	, CurrentWidget_ (0)
	, CurrentIndex_ (-1)
	, PreviousWidget_ (0)
	{
		XmlSettingsManager::Instance ()->RegisterObject ("SelectionBehavior",
			this, "handleSelectionBehavior");
		Core::Instance ().GetCoreInstanceObject ()->
				GetCorePluginManager ()->RegisterHookable (this);
		handleSelectionBehavior ();

		MainTabBar_->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Minimum);
		MainTabBar_->SetTabWidget (this);

		LeftToolBar_->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);
		RightToolBar_->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);

		LeftToolBar_->setMaximumHeight (25);
		RightToolBar_->setMaximumHeight (25);

		auto leftToolBarLayout = new QVBoxLayout;
		auto rightToolBarLayout = new QVBoxLayout;
		leftToolBarLayout->addWidget (LeftToolBar_);
		leftToolBarLayout->setContentsMargins (0, 0, 0, 0);
		rightToolBarLayout->addWidget (RightToolBar_);
		rightToolBarLayout->setContentsMargins (0, 0, 0, 0);

		MainTabBarLayout_ = new QHBoxLayout;
		MainTabBarLayout_->setContentsMargins (0, 0, 0, 0);
		MainTabBarLayout_->setSpacing (0);
		MainTabBarLayout_->addLayout (leftToolBarLayout);
		MainTabBarLayout_->addWidget (MainTabBar_);
		MainTabBarLayout_->addLayout (rightToolBarLayout);

		MainToolBarLayout_ = new QHBoxLayout;
		MainToolBarLayout_->setSpacing (0);
		MainToolBarLayout_->setContentsMargins (0, 0, 0, 0);

		MainToolBarLayout_->addSpacerItem (new QSpacerItem (1, 1,
				QSizePolicy::Minimum, QSizePolicy::Minimum));

		MainLayout_ = new QVBoxLayout (this);
		MainLayout_->setContentsMargins (0, 0, 0, 0);
		MainLayout_->setSpacing (0);
		MainLayout_->addLayout (MainToolBarLayout_);
		MainLayout_->addWidget (MainStackedWidget_);

		XmlSettingsManager::Instance ()->RegisterObject ("TabBarPosition",
				this, "handleTabBarPosition");
		handleTabBarPosition ();

		Init ();
		AddTabButtonInit ();
	}

	QObject* SeparateTabWidget::GetObject ()
	{
		return this;
	}

	int SeparateTabWidget::WidgetCount () const
	{
		return MainStackedWidget_->count ();
	}

	QWidget* SeparateTabWidget::Widget (int index) const
	{
		return MainStackedWidget_->widget (index);
	}

	QList<QAction*> SeparateTabWidget::GetPermanentActions () const
	{
		QList<QAction*> result;
		std::transform (TabBarActions_.begin (), TabBarActions_.end (),
				std::back_inserter (result),
				[] (decltype (TabBarActions_.front ()) action)
				{
					return action.data ();
				});
		return result;
	}

	QVariant SeparateTabWidget::TabData (int index) const
	{
		return MainTabBar_->tabData (index);
	}

	void SeparateTabWidget::SetTabData (int index, QVariant data)
	{
		MainTabBar_->setTabData (index, data);
	}

	QString SeparateTabWidget::TabText (int index) const
	{
		return TabNames_.value (index);
	}

	void SeparateTabWidget::SetTabText (int index, const QString& text)
	{
		if (index < 0 || index >= WidgetCount ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid index"
					<< index;
			return;
		}

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookTabSetText (proxy, index);
		if (proxy->IsCancelled ())
			return;

		MainTabBar_->setTabText (index, text);
		if (!text.isEmpty ())
			TabNames_ [index] = text;
	}

	QIcon SeparateTabWidget::TabIcon (int index) const
	{
		return MainTabBar_->tabIcon (index);
	}

	void SeparateTabWidget::SetTabIcon (int index, const QIcon& icon)
	{
		if (index < 0 ||
				index >= WidgetCount ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid index"
					<< index;
			return;
		}

		MainTabBar_->setTabIcon (index, icon);
	}

	QString SeparateTabWidget::TabToolTip (int index) const
	{
		return MainTabBar_->tabToolTip (index);
	}

	void SeparateTabWidget::SetTabToolTip (int index, const QString& tip)
	{
		if (index < 0 ||
				index >= WidgetCount ())
		{
			qWarning () << Q_FUNC_INFO
				<< "invalid index"
				<< index;
			return;
		}

		MainTabBar_->setTabToolTip (index, tip);
	}

	QWidget* SeparateTabWidget::TabButton (int index, QTabBar::ButtonPosition positioin) const
	{
		return MainTabBar_->tabButton (index, positioin);
	}

	QTabBar::ButtonPosition SeparateTabWidget::GetCloseButtonPosition () const
	{
		return MainTabBar_->GetCloseButtonPosition ();
	}

	void SeparateTabWidget::SetTabClosable (int index, bool closable, QWidget *closeButton)
	{
		MainTabBar_->SetTabClosable (index, closable, closeButton);
	}

	void SeparateTabWidget::SetTabsClosable (bool closable)
	{
		MainTabBar_->setTabsClosable (closable);
		MainTabBar_->SetTabClosable (WidgetCount (), false);
	}

	void SeparateTabWidget::AddWidget2TabBarLayout (QTabBar::ButtonPosition pos,
			QWidget *w)
	{
		if (pos == QTabBar::LeftSide)
			LeftToolBar_->addWidget (w);
		else
			RightToolBar_->addWidget (w);
	}

	void SeparateTabWidget::AddAction2TabBarLayout (QTabBar::ButtonPosition pos,
			QAction *action)
	{
		if (pos == QTabBar::LeftSide)
			LeftToolBar_->addAction (action);
		else
			RightToolBar_->addAction (action);
	}

	void SeparateTabWidget::RemoveActionFromTabBarLayout (QTabBar::ButtonPosition pos,
			QAction *action)
	{
		if (pos == QTabBar::LeftSide)
			LeftToolBar_->removeAction (action);
		else
			RightToolBar_->removeAction (action);
	}

	void SeparateTabWidget::AddAction2TabBar (QAction *act)
	{
		TabBarActions_ << act;
		connect (act,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handleActionDestroyed ()));
	}

	void SeparateTabWidget::InsertAction2TabBar (int index, QAction *act)
	{
		TabBarActions_.insert (index, act);
		connect (act,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handleActionDestroyed ()));
	}

	void SeparateTabWidget::InsertAction2TabBar (QAction *before, QAction *action)
	{
		int idx = TabBarActions_.indexOf (before);
		if (idx < 0)
			idx = TabBarActions_.size ();
		InsertAction2TabBar (idx, action);
	}

	int SeparateTabWidget::CurrentIndex () const
	{
		return MainTabBar_->currentIndex ();
	}

	QWidget* SeparateTabWidget::CurrentWidget () const
	{
		return MainStackedWidget_->currentWidget ();
	}

	QMenu* SeparateTabWidget::GetTabMenu (int index)
	{
		QMenu *menu = new QMenu ();
		if (XmlSettingsManager::Instance ()->
				property ("ShowPluginMenuInTabs").toBool ())
		{
			bool asSub = XmlSettingsManager::Instance ()->
				property ("ShowPluginMenuInTabsAsSubmenu").toBool ();
			ITabWidget *imtw = qobject_cast<ITabWidget*> (Widget (index));
			if (imtw)
			{
				QList<QAction*> tabActions = imtw->GetTabBarContextMenuActions ();

				QMenu *subMenu = new QMenu (TabText (index), menu);
				Q_FOREACH (QAction *act, tabActions)
					(asSub ? subMenu : menu)->addAction (act);
				if (asSub)
					menu->addMenu (subMenu);
				if (tabActions.size ())
					menu->addSeparator ();
			}
		}

		Q_FOREACH (QAction *act, TabBarActions_)
		{
			if (!act)
			{
				qWarning () << Q_FUNC_INFO
						<< "detected null pointer";
				continue;
			}
			menu->addAction (act);
		}

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookTabContextMenuFill (proxy, menu, index);

		return menu;
	}

	int SeparateTabWidget::IndexOf (QWidget *page) const
	{
		return MainStackedWidget_->indexOf (page);
	}

	int SeparateTabWidget::GetLastContextMenuTab () const
	{
		return LastContextMenuTab_;
	}

	void SeparateTabWidget::SetAddTabButtonContextMenu (QMenu *menu)
	{
		AddTabButtonContextMenu_ = menu;
		AddTabButton_->setMenu (AddTabButtonContextMenu_);
	}

	SeparateTabBar* SeparateTabWidget::TabBar () const
	{
		return MainTabBar_;
	}

	int SeparateTabWidget::AddTab (QWidget *page, const QString& text)
	{
		return AddTab (page, QIcon (), text);
	}

	int SeparateTabWidget::AddTab (QWidget *page,
			const QIcon& icon, const QString& text)
	{
		if (!page)
		{
			qWarning () << Q_FUNC_INFO
					<< "There is no widget to add to SeaprateTabWidget"
					<< page;
			return -1;
		}

		if (MainStackedWidget_->indexOf (page) != -1)
			return -1;

		int newIndex = 0;
		MainStackedWidget_->addWidget (page);
		TabNames_ << text;
		if (!AddTabButtonAction_->isVisible ())
			newIndex = MainTabBar_->
					insertTab (MainTabBar_->count () - 1, icon, text);
		else
			newIndex = MainTabBar_->addTab (icon, text);

		MainTabBar_->setTabToolTip (newIndex, text);

		if (MainTabBar_->currentIndex () >= WidgetCount ())
			setCurrentTab (WidgetCount () - 1);

		return newIndex;
	}

	int SeparateTabWidget::InsertTab (int index, QWidget *page,
			const QString& text)
	{
		return InsertTab (index, page, QIcon (), text);
	}

	int SeparateTabWidget::InsertTab (int index, QWidget *page,
			const QIcon& icon, const QString& text)
	{
		int newIndex = index;
		if (index > WidgetCount () && !AddTabButtonAction_->isVisible ())
			newIndex = WidgetCount ();

		MainStackedWidget_->insertWidget (index, page);
		int idx = MainTabBar_->insertTab (newIndex, icon, text);
		MainTabBar_->setTabToolTip (idx, text);

		TabNames_.insert (index, text);

		if (MainTabBar_->currentIndex () >= WidgetCount ())
			setCurrentTab (WidgetCount () - 1);

		return idx;
	}

	void SeparateTabWidget::RemoveTab (int index)
	{
		if (index >= WidgetCount () &&
			!AddTabButtonAction_->isVisible ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid index"
					<< index;
			return;
		}

		if (MainStackedWidget_->widget (index) == PreviousWidget_)
			PreviousWidget_ = 0;

		MainStackedWidget_->removeWidget (Widget (index));
		MainTabBar_->removeTab (index);

		TabNames_.removeAt (index);
	}

	bool SeparateTabWidget::IsAddTabActionVisible () const
	{
		return AddTabButtonAction_->isVisible ();
	}

	void SeparateTabWidget::AddWidget2SeparateTabWidget (QWidget *widget)
	{
		widget->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Minimum);
		MainToolBarLayout_->addWidget (widget);
	}

	void SeparateTabWidget::RemoveWidgetFromSeparateTabWidget (QWidget *widget)
	{
		MainToolBarLayout_->removeWidget (widget);
		widget->hide ();
	}

	int SeparateTabWidget::TabAt (const QPoint& point)
	{
		return MainTabBar_->tabAt (point);
	}

	void SeparateTabWidget::MoveTab (int from, int to)
	{
		MainTabBar_->moveTab (from, to);
	}

	QWidget* SeparateTabWidget::GetPreviousWidget () const
	{
		return PreviousWidget_;
	}

	void SeparateTabWidget::resizeEvent (QResizeEvent *event)
	{
		QWidget::resizeEvent (event);
		int length = 0;
		for (int i = 0; i < MainTabBar_->count (); ++i)
			length += MainTabBar_->tabRect (i).width ();

		if (event->oldSize ().width () > event->size ().width ())
		{
			if (length + 30 > MainTabBar_->width () &&
					!AddTabButtonAction_->isVisible ())
			{
				handleShowAddTabButton (true);
				MainTabBar_->SetLastTab (false);
				MainTabBar_->removeTab (MainTabBar_->count () - 1);
			}
		}
		else if (event->oldSize ().width () < event->size ().width ())
		{
			if (length + 60 < MainTabBar_->width () &&
					AddTabButtonAction_->isVisible ())
			{
				handleShowAddTabButton (false);
				MainTabBar_->SetLastTab (true);
				int index = MainTabBar_->addTab (QString ());
				MainTabBar_->SetTabClosable (index, false);
			}
		}
	}

	void SeparateTabWidget::mousePressEvent (QMouseEvent *event)
	{
		const bool mBack = event->button () == Qt::XButton1;
		const bool mForward = event->button () == Qt::XButton2;
		if (mBack || mForward)
		{
			mBack ? Core::Instance ().GetTabManager ()->rotateLeft () :
					Core::Instance ().GetTabManager ()->rotateRight ();
			event->accept ();
			return;
		}

		QWidget::mousePressEvent (event);
	}

	void SeparateTabWidget::Init ()
	{
		connect (MainTabBar_,
				SIGNAL (addDefaultTab ()),
				this,
				SLOT (handleAddDefaultTab ()));

		connect (DefaultTabAction_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleAddDefaultTab ()));

		connect (MainTabBar_,
				SIGNAL (tabWasInserted (int)),
				this,
				SIGNAL (tabInserted (int)));
		connect (MainTabBar_,
				SIGNAL (tabWasRemoved (int)),
				this,
				SIGNAL (tabWasRemoved (int)));
		connect (MainTabBar_,
				SIGNAL (tabCloseRequested (int)),
				this,
				SIGNAL (tabCloseRequested (int)));
		connect (MainTabBar_,
				SIGNAL (tabMoved (int, int)),
				this,
				SLOT (handleTabMoved (int, int)));
		connect (MainTabBar_,
				SIGNAL (currentChanged (int)),
				this,
				SLOT (setCurrentIndex (int)));
		connect (MainTabBar_,
				SIGNAL (customContextMenuRequested (const QPoint&)),
				this,
				SLOT (handleContextMenuRequested (const QPoint&)));
		connect (MainTabBar_,
				SIGNAL (showAddTabButton (bool)),
				this,
				SLOT (handleShowAddTabButton (bool)));
		connect (MainTabBar_,
				SIGNAL (releasedMouseAfterMove (int)),
				this,
				SLOT (releaseMouseAfterMove (int)));
	}

	void SeparateTabWidget::AddTabButtonInit ()
	{
		DefaultTabAction_->setProperty ("ActionIcon", "list-add");
		AddTabButton_->setPopupMode (QToolButton::MenuButtonPopup);
		AddTabButton_->setToolButtonStyle (Qt::ToolButtonIconOnly);
		AddTabButton_->setArrowType (Qt::NoArrow);
		AddTabButton_->setDefaultAction (DefaultTabAction_);
		AddTabButtonAction_ = RightToolBar_->addWidget (AddTabButton_);
		AddTabButtonAction_->setVisible (false);
		RightToolBar_->addSeparator ();
	}

	void SeparateTabWidget::setCurrentIndex (int index)
	{
		if (index >= WidgetCount () &&
				!AddTabButtonAction_->isVisible ())
			index = WidgetCount () - 1;

		MainStackedWidget_->setCurrentIndex (index);
		MainTabBar_->setCurrentIndex (index);

		if (CurrentWidget_ != Widget (index))
		{
			CurrentIndex_ = IndexOf (CurrentWidget_);
			CurrentWidget_ = Widget (index);
			PreviousWidget_ = Widget (CurrentIndex_);
			CurrentIndex_ = index;
		}

		emit currentChanged (index);
	}

	void SeparateTabWidget::setCurrentTab (int tabIndex)
	{
		MainTabBar_->setCurrentIndex (tabIndex);
	}

	void SeparateTabWidget::setCurrentWidget (QWidget *widget)
	{
		if (!widget)
			return;

		int index = MainStackedWidget_->indexOf (widget);
		setCurrentTab (index);
	}

	void SeparateTabWidget::handleNewTabShortcutActivated ()
	{
		handleAddDefaultTab ();
	}

	void SeparateTabWidget::setPreviousTab ()
	{
		setCurrentWidget (PreviousWidget_);
	}

	void SeparateTabWidget::handleTabMoved (int from, int to)
	{
		if (from == MainTabBar_->count () - 1 &&
				!AddTabButtonAction_->isVisible ())
		{
			MainTabBar_->moveTab (to, from);
			return;
		}

		if ((to == MainTabBar_->count () - 1) &&
				!AddTabButtonAction_->isVisible ())
			return;

		std::swap (TabNames_ [from], TabNames_ [to]);

		MainStackedWidget_->insertWidget (to, MainStackedWidget_->widget (from));

		MainTabBar_->SetInMove (true);
		setCurrentIndex (to);
		emit tabWasMoved (from, to);
	}

	void SeparateTabWidget::handleContextMenuRequested (const QPoint& point)
	{
		if (point.isNull ())
			return;

		QMenu *menu = new QMenu ("", MainTabBar_);
		int index = MainTabBar_->tabAt (point);

		if (index == -1)
		{
			Q_FOREACH (QAction *act, TabBarActions_)
			{
				if (!act)
				{
					qWarning () << Q_FUNC_INFO
							<< "detected null pointer";
					continue;
				}
				menu->addAction (act);
			}
		}
		else if ((index == MainTabBar_->count () - 1) &&
				!AddTabButtonAction_->isVisible ())
		{
			menu->addActions (AddTabButtonContextMenu_->actions ());
		}
		else
		{
			LastContextMenuTab_ = index;
			delete menu;
			menu = GetTabMenu (index);
		}
		menu->exec (MainTabBar_->mapToGlobal (point));
		delete menu;
	}

	void SeparateTabWidget::handleActionDestroyed ()
	{
		Q_FOREACH (QPointer<QAction> act, TabBarActions_)
			if (!act || act == sender ())
				TabBarActions_.removeAll (act);
	}

	void SeparateTabWidget::releaseMouseAfterMove (int index)
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookTabFinishedMoving (proxy, index);
	}

	void SeparateTabWidget::handleTabBarPosition ()
	{
		MainLayout_->removeItem (MainTabBarLayout_);

		const auto& settingVal = XmlSettingsManager::Instance ()->property ("TabBarPosition").toString ();
		const bool isBottom = settingVal == "Bottom";
		if (isBottom)
			MainLayout_->addLayout (MainTabBarLayout_);
		else
			MainLayout_->insertLayout (0, MainTabBarLayout_);

		MainTabBar_->setShape (isBottom ? QTabBar::RoundedSouth : QTabBar::RoundedNorth);
	}

	void SeparateTabWidget::handleSelectionBehavior ()
	{
		const QString& selection = XmlSettingsManager::Instance ()->
				property ("SelectionBehavior").toString ();
		if (selection == "PreviousActive")
			MainTabBar_->setSelectionBehaviorOnRemove (QTabBar::SelectPreviousTab);
		else if (selection == "NextIndex")
			MainTabBar_->setSelectionBehaviorOnRemove (QTabBar::SelectRightTab);
		else if (selection == "PreviousIndex")
			MainTabBar_->setSelectionBehaviorOnRemove (QTabBar::SelectLeftTab);
	}

	void SeparateTabWidget::handleAddDefaultTab ()
	{
		const auto& combined = XmlSettingsManager::Instance ()->
				property ("DefaultNewTab").toString ().toLatin1 ();
		if (combined != "contextdependent")
		{
			const auto& parts = combined.split ('|');
			if (parts.size () != 2)
				qWarning () << Q_FUNC_INFO
						<< "incorrect split"
						<< parts
						<< combined;
			else
			{
				const QByteArray& newTabId = parts.at (0);
				const QByteArray& tabClass = parts.at (1);
				QObject *plugin = Core::Instance ()
						.GetPluginManager ()->GetPluginByID (newTabId);
				IHaveTabs *iht = qobject_cast<IHaveTabs*> (plugin);
				if (!iht)
					qWarning () << Q_FUNC_INFO
							<< "plugin with id"
							<< newTabId
							<< "is not a IMultiTabs";
				else
				{
					iht->TabOpenRequested (tabClass);
					return;
				}
			}
		}

		IHaveTabs *highestIHT = 0;
		QByteArray highestTabClass;
		int highestPriority = 0;
		Q_FOREACH (IHaveTabs *iht, Core::Instance ()
				.GetPluginManager ()->GetAllCastableTo<IHaveTabs*> ())
			Q_FOREACH (const TabClassInfo& info, iht->GetTabClasses ())
			{
				if (!(info.Features_ & TFOpenableByRequest))
					continue;

				if (info.Priority_ <= highestPriority)
					continue;

				highestIHT = iht;
				highestTabClass = info.TabClass_;
				highestPriority = info.Priority_;
			}

		ITabWidget *imtw = qobject_cast<ITabWidget*> (CurrentWidget ());
		const int delta = 15;
		if (imtw && imtw->GetTabClassInfo ().Priority_ + delta > highestPriority)
		{
			highestIHT = qobject_cast<IHaveTabs*> (imtw->ParentMultiTabs ());
			highestTabClass = imtw->GetTabClassInfo ().TabClass_;
		}

		if (!highestIHT)
		{
			qWarning () << Q_FUNC_INFO
					<< "no IHT detected";
			return;
		}

		highestIHT->TabOpenRequested (highestTabClass);
	}

	void SeparateTabWidget::handleShowAddTabButton (bool show)
	{
		AddTabButtonAction_->setVisible (show);
	}
}
