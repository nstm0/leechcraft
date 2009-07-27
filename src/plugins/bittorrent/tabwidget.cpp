#include "tabwidget.h"
#include <QSortFilterProxyModel>
#include <plugininterface/proxy.h>
#include "core.h"
#include "filesviewdelegate.h"
#include "torrentfilesmodel.h"
#include "xmlsettingsmanager.h"
#include "peerstablinker.h"
#include "piecesmodel.h"
#include "peersmodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			using namespace Util;

			TabWidget::TabWidget (QWidget *parent)
			: QTabWidget (parent)
			, TorrentSelectionChanged_ (false)
			{
				Ui_.setupUi (this);
				QFontMetrics fm = QApplication::fontMetrics ();
				QHeaderView *header = Ui_.PerTrackerStats_->header ();
				header->resizeSection (0,
						fm.width ("www.domain.name.org"));
				header->resizeSection (1,
						fm.width ("1234.5678 bytes/s"));
				header->resizeSection (2,
						fm.width ("1234.5678 bytes/s"));

				connect (Ui_.OverallDownloadRateController_,
						SIGNAL (valueChanged (int)), this,
						SLOT (on_OverallDownloadRateController__valueChanged (int)));
				connect (Ui_.OverallUploadRateController_,
						SIGNAL (valueChanged (int)),
						this,
						SLOT (on_OverallUploadRateController__valueChanged (int)));
				connect (Ui_.DesiredRating_,
						SIGNAL (valueChanged (double)),
						this,
						SLOT (on_DesiredRating__valueChanged (double)));
				connect (Ui_.TorrentDownloadRateController_,
						SIGNAL (valueChanged (int)),
						this,
						SLOT (on_TorrentDownloadRateController__valueChanged (int)));
				connect (Ui_.TorrentUploadRateController_,
						SIGNAL (valueChanged (int)),
						this,
						SLOT (on_TorrentUploadRateController__valueChanged (int)));
				connect (Ui_.TorrentDesiredRating_,
						SIGNAL (valueChanged (double)),
						this,
						SLOT (on_TorrentDesiredRating__valueChanged (double)));
				connect (Ui_.TorrentManaged_,
						SIGNAL (stateChanged (int)),
						this,
						SLOT (on_TorrentManaged__stateChanged (int)));
				connect (Ui_.TorrentSequentialDownload_,
						SIGNAL (stateChanged (int)),
						this,
						SLOT (on_TorrentSequentialDownload__stateChanged (int)));
				connect (Ui_.TorrentSuperSeeding_,
						SIGNAL (stateChanged (int)),
						this,
						SLOT (on_TorrentSuperSeeding__stateChanged (int)));
				connect (Ui_.DownloadingTorrents_,
						SIGNAL (valueChanged (int)),
						this,
						SLOT (on_DownloadingTorrents__valueChanged (int)));
				connect (Ui_.UploadingTorrents_,
						SIGNAL (valueChanged (int)),
						this,
						SLOT (on_UploadingTorrents__valueChanged (int)));
				connect (Ui_.TorrentTags_,
						SIGNAL (editingFinished ()),
						this,
						SLOT (on_TorrentTags__editingFinished ()));

				connect (Core::Instance (),
						SIGNAL (dataChanged (const QModelIndex&,
								const QModelIndex&)),
						this,
						SLOT (updateTorrentStats ()));
				connect (this,
						SIGNAL (currentChanged (int)),
						this,
						SLOT (updateTorrentStats ()));

				TagsChangeCompleter_.reset (new TagsCompleter (Ui_.TorrentTags_));
				Ui_.TorrentTags_->AddSelector ();
			
				Ui_.PiecesView_->setModel (Core::Instance ()->GetPiecesModel ());
			
				Ui_.FilesView_->setModel (Core::Instance ()->GetTorrentFilesModel ());
				Ui_.FilesView_->setItemDelegate (new FilesViewDelegate (this));
			
				QSortFilterProxyModel *peersSorter = new QSortFilterProxyModel (this);
				peersSorter->setDynamicSortFilter (true);
				peersSorter->setSourceModel (Core::Instance ()->GetPeersModel ());
				peersSorter->setSortRole (PeersModel::SortRole);
				Ui_.PeersView_->setModel (peersSorter);

				new PeersTabLinker (&Ui_, peersSorter, this);
			
				UpdateDashboard ();
				
				QList<QByteArray> tabWidgetSettings;
				tabWidgetSettings << "ActiveSessionStats"
					<< "ActiveAdvancedSessionStats"
					<< "ActiveTrackerStats"
					<< "ActiveCacheStats"
					<< "ActiveTorrentStatus"
					<< "ActiveTorrentAdvancedStatus"
					<< "ActiveTorrentInfo"
					<< "ActiveTorrentPeers";
				XmlSettingsManager::Instance ()->RegisterObject (tabWidgetSettings,
						this, "setTabWidgetSettings");
			
				setTabWidgetSettings ();
			}

			void TabWidget::InvalidateSelection ()
			{
				TorrentSelectionChanged_ = true;
				Ui_.TorrentTags_->setText (Core::Instance ()->GetProxy ()->GetTagsManager ()->
						Join (Core::Instance ()->GetTagsForIndex ()));
				updateTorrentStats ();
			}

			void TabWidget::updateTorrentStats ()
			{
				if (!Core::Instance ()->GetCurrentTorrent () == -1)
					return;
			
				switch (currentIndex ())
				{
					case 0:
						UpdateDashboard ();
						UpdateOverallStats ();
						break;
					case 1:
						UpdateTorrentControl ();
						break;
					case 2:
						UpdateFilesPage ();
						break;
					case 3:
						UpdatePeersPage ();
						break;
					case 4:
						UpdatePiecesPage ();
						break;
				}
				TorrentSelectionChanged_ = false;
			}
			
			namespace
			{
				struct Percenter
				{
					template<typename T>
					QString operator() (const T& t1, const T& t2) const
					{
						if (t2)
						{
							return QString (" (") +
								QString::number (static_cast<float> (t1) * 100 /
										static_cast<float> (t2), 'f', 1) +
								"%)";
						}
						else
							return QString ("");
					}
				};
			
				template<int i>
				struct Constructor
				{
					template<typename T>
					QString operator() (const T& t1, const T& t2) const
					{
						Percenter p;
						return Proxy::Instance ()->MakePrettySize (t1) +
							(i ? QObject::tr ("/s") : "") + p (t1, t2);
					}
				};
			};
			
			void TabWidget::UpdateOverallStats ()
			{
				libtorrent::session_status stats = Core::Instance ()->GetOverallStats ();
			
				Ui_.LabelTotalDownloadRate_->
					setText (Proxy::Instance ()->
							MakePrettySize (stats.download_rate) + tr ("/s"));
				Ui_.LabelTotalUploadRate_->
					setText (Proxy::Instance ()->
							MakePrettySize (stats.upload_rate) + tr ("/s"));
			
				Constructor<1> speed;
			
				Ui_.LabelOverheadDownloadRate_->
					setText (speed (stats.ip_overhead_download_rate, stats.download_rate));
				Ui_.LabelOverheadUploadRate_->
					setText (speed (stats.ip_overhead_upload_rate, stats.upload_rate));
				Ui_.LabelDHTDownloadRate_->
					setText (speed (stats.dht_download_rate, stats.download_rate));
				Ui_.LabelDHTUploadRate_->
					setText (speed (stats.dht_upload_rate, stats.upload_rate));
				Ui_.LabelTrackerDownloadRate_->
					setText (speed (stats.tracker_download_rate, stats.download_rate));
				Ui_.LabelTrackerUploadRate_->
					setText (speed (stats.tracker_upload_rate, stats.upload_rate));
			
				Ui_.LabelTotalDownloaded_->
					setText (Proxy::Instance ()->
							MakePrettySize (stats.total_download));
				Ui_.LabelTotalUploaded_->
					setText (Proxy::Instance ()->
							MakePrettySize (stats.total_upload));
			
				Constructor<0> simple;
				Ui_.LabelOverheadDownloaded_->
					setText (simple (stats.total_ip_overhead_download, stats.total_download));
				Ui_.LabelOverheadUploaded_->
					setText (simple (stats.total_ip_overhead_upload, stats.total_upload));
				Ui_.LabelDHTDownloaded_->
					setText (simple (stats.total_dht_download, stats.total_download));
				Ui_.LabelDHTUploaded_->
					setText (simple (stats.total_dht_upload, stats.total_upload));
				Ui_.LabelTrackerDownloaded_->
					setText (simple (stats.total_tracker_download, stats.total_download));
				Ui_.LabelTrackerUploaded_->
					setText (simple (stats.total_tracker_upload, stats.total_upload));
			
				Ui_.LabelTotalPeers_->setText (QString::number (stats.num_peers));
				Ui_.LabelTotalDHTNodes_->setText (QString ("(") +
						QString::number (stats.dht_global_nodes) +
						QString (") ") +
						QString::number (stats.dht_nodes));
				Ui_.LabelDHTTorrents_->
					setText (QString::number (stats.dht_torrents));
				Ui_.LabelListenPort_->
					setText (QString::number (Core::Instance ()->GetListenPort ()));
				if (stats.total_payload_download)
					Ui_.LabelSessionRating_->
						setText (QString::number (stats.total_payload_upload /
									static_cast<double> (stats.total_payload_download), 'g', 4));
				else
					Ui_.LabelSessionRating_->setText (QString::fromUtf8 ("\u221E"));
				Ui_.LabelTotalFailedData_->
					setText (Proxy::Instance ()->MakePrettySize (stats.total_failed_bytes));
				Ui_.LabelTotalRedundantData_->
					setText (Proxy::Instance ()->MakePrettySize (stats.total_redundant_bytes));
				Ui_.LabelExternalAddress_->
					setText (Core::Instance ()->GetExternalAddress ());
			
				libtorrent::cache_status cs = Core::Instance ()->GetCacheStats ();
				Ui_.BlocksWritten_->setText (QString::number (cs.blocks_written));
				Ui_.Writes_->setText (QString::number (cs.writes));
				Ui_.WriteHitRatio_->
					setText (QString::number (static_cast<double> (cs.blocks_written - cs.writes ) /
							static_cast<double> (cs.blocks_written)));
				Ui_.CacheSize_->setText (QString::number (cs.cache_size));
				Ui_.TotalBlocksRead_->setText (QString::number (cs.blocks_read));
				Ui_.CachedBlockReads_->setText (QString::number (cs.blocks_read_hit));
				Ui_.ReadHitRatio_->
					setText (QString::number (static_cast<double> (cs.blocks_read_hit) /
							static_cast<double> (cs.blocks_read)));
				Ui_.ReadCacheSize_->setText (QString::number (cs.read_cache_size));
			
				Core::pertrackerstats_t ptstats;
				Core::Instance ()->GetPerTracker (ptstats);
				Ui_.PerTrackerStats_->clear ();
				for (Core::pertrackerstats_t::const_iterator i = ptstats.begin (),
						end = ptstats.end (); i != end; ++i)
				{
					QStringList strings;
					strings	<< i->first
						<< Proxy::Instance ()->MakePrettySize (i->second.DownloadRate_) + tr ("/s")
						<< Proxy::Instance ()->MakePrettySize (i->second.UploadRate_) + tr ("/s");
			
					new QTreeWidgetItem (Ui_.PerTrackerStats_, strings);
				}
			}
			
			void TabWidget::UpdateDashboard ()
			{
				Ui_.OverallDownloadRateController_->setValue (Core::Instance ()->GetOverallDownloadRate ());
				Ui_.OverallUploadRateController_->setValue (Core::Instance ()->GetOverallUploadRate ());
				Ui_.DownloadingTorrents_->setValue (Core::Instance ()->GetMaxDownloadingTorrents ());
				Ui_.UploadingTorrents_->setValue (Core::Instance ()->GetMaxUploadingTorrents ());
				Ui_.DesiredRating_->setValue (Core::Instance ()->GetDesiredRating ());
			}
			
			void TabWidget::UpdateTorrentControl ()
			{
				Ui_.TorrentDownloadRateController_->
					setValue (Core::Instance ()->GetTorrentDownloadRate ());
				Ui_.TorrentUploadRateController_->setValue (Core::Instance ()->
						GetTorrentUploadRate ());
				Ui_.TorrentDesiredRating_->setValue (Core::Instance ()->
						GetTorrentDesiredRating ());
				Ui_.TorrentManaged_->setCheckState (Core::Instance ()->
						IsTorrentManaged () ? Qt::Checked : Qt::Unchecked);
				Ui_.TorrentSequentialDownload_->setCheckState (Core::Instance ()->
						IsTorrentSequentialDownload () ? Qt::Checked : Qt::Unchecked);
				Ui_.TorrentSuperSeeding_->setCheckState (Core::Instance ()->
						IsTorrentSuperSeeding () ? Qt::Checked : Qt::Unchecked);
			
				std::auto_ptr<TorrentInfo> i;
				try
				{
					i = Core::Instance ()->GetTorrentStats ();
				}
				catch (...)
				{
					Ui_.TorrentControlTab_->setEnabled (false);
					return;
				}
			
				Ui_.TorrentControlTab_->setEnabled (true);
				Ui_.LabelState_->setText (i->State_);
				Ui_.LabelDownloadRate_->
					setText (Proxy::Instance ()->
							MakePrettySize (i->Status_.download_rate) + tr ("/s"));
				Ui_.LabelUploadRate_->
					setText (Proxy::Instance ()->
							MakePrettySize (i->Status_.upload_rate) + tr ("/s"));
				Ui_.LabelNextAnnounce_->
					setText (QTime (i->Status_.next_announce.hours (),
								i->Status_.next_announce.minutes (),
								i->Status_.next_announce.seconds ()).toString ());
				Ui_.LabelProgress_->
					setText (QString::number (i->Status_.progress * 100, 'f', 2) + "%");
				Ui_.LabelDownloaded_->
					setText (Proxy::Instance ()->MakePrettySize (i->Status_.total_download));
				Ui_.LabelUploaded_->
					setText (Proxy::Instance ()->MakePrettySize (i->Status_.total_upload));
				Ui_.LabelWantedDownloaded_->
					setText (Proxy::Instance ()->MakePrettySize (i->Status_.total_wanted_done));
				Ui_.LabelDownloadedTotal_->
					setText (Proxy::Instance ()->MakePrettySize (i->Status_.all_time_download));
				Ui_.LabelUploadedTotal_->
					setText (Proxy::Instance ()->MakePrettySize (i->Status_.all_time_upload));
				if (i->Status_.all_time_download)
					Ui_.LabelTorrentOverallRating_->
						setText (QString::number (i->Status_.all_time_upload /
									static_cast<double> (i->Status_.all_time_download), 'g', 4));
				else
					Ui_.LabelTorrentOverallRating_->
						setText (QString::fromUtf8 ("\u221E"));
				Ui_.LabelActiveTime_->
					setText (Proxy::Instance ()->MakeTimeFromLong (i->Status_.active_time));
				Ui_.LabelSeedingTime_->
					setText (Proxy::Instance ()->MakeTimeFromLong (i->Status_.seeding_time));
				Ui_.LabelSeedRank_->
					setText (QString::number (i->Status_.seed_rank));
				if (i->Status_.last_scrape >= 0)
					Ui_.LabelLastScrape_->
						setText (Proxy::Instance ()->MakeTimeFromLong (i->Status_.last_scrape));
				else
					Ui_.LabelLastScrape_->
						setText (tr ("Wasn't yet"));
				Ui_.LabelTotalSize_->
					setText (Proxy::Instance ()->MakePrettySize (i->Info_->total_size ()));
				Ui_.LabelWantedSize_->
					setText (Proxy::Instance ()->MakePrettySize (i->Status_.total_wanted));
				if (i->Status_.total_payload_download)
					Ui_.LabelTorrentRating_->
						setText (QString::number (i->Status_.total_payload_upload /
									static_cast<double> (i->Status_.total_payload_download), 'g', 4));
				else
					Ui_.LabelTorrentRating_->
						setText (QString::fromUtf8 ("\u221E"));
				Ui_.PiecesWidget_->setPieceMap (i->Status_.pieces);
				Ui_.LabelTracker_->
					setText (QString::fromStdString (i->Status_.current_tracker));
				Ui_.LabelDestination_->
					setText (i->Destination_);
				Ui_.LabelName_->
					setText (QString::fromUtf8 (i->Info_->name ().c_str ()));
				Ui_.LabelCreator_->
					setText (QString::fromUtf8 (i->Info_->creator ().c_str ()));
				Ui_.LabelComment_->
					setText (QString::fromUtf8 (i->Info_->comment ().c_str ()));
				Ui_.LabelPrivate_->
					setText (i->Info_->priv () ? tr ("Yes") : tr ("No"));
				Ui_.LabelDHTNodesCount_->
					setText (QString::number (i->Info_->nodes ().size ()));
				Ui_.LabelFailed_->
					setText (Proxy::Instance ()->MakePrettySize (i->Status_.total_failed_bytes));
				Ui_.LabelConnectedPeers_->
					setText (QString::number (i->Status_.num_peers));
				Ui_.LabelConnectedSeeds_->
					setText (QString::number (i->Status_.num_seeds));
				Ui_.LabelAnnounceInterval_->
					setText (QTime (i->Status_.announce_interval.hours (),
								i->Status_.announce_interval.minutes (),
								i->Status_.announce_interval.seconds ()).toString ());
				Ui_.LabelTotalPieces_->
					setText (QString::number (i->Info_->num_pieces ()));
				Ui_.LabelDownloadedPieces_->
					setText (QString::number (i->Status_.num_pieces));
				Ui_.LabelPieceSize_->
					setText (Proxy::Instance ()->MakePrettySize (i->Info_->piece_length ()));
				Ui_.LabelBlockSize_->
					setText (Proxy::Instance ()->MakePrettySize (i->Status_.block_size));
				Ui_.LabelDistributedCopies_->
					setText (i->Status_.distributed_copies == -1 ?
						tr ("Not tracking") : QString::number (i->Status_.distributed_copies));
				Ui_.LabelRedundantData_->
					setText (Proxy::Instance ()->MakePrettySize (i->Status_.total_redundant_bytes));
				Ui_.LabelPeersInList_->
					setText (QString::number (i->Status_.list_peers));
				Ui_.LabelSeedsInList_->
					setText (QString::number (i->Status_.list_seeds));
				Ui_.LabelPeersInSwarm_->
					setText ((i->Status_.num_incomplete == -1 ?
							tr ("Unknown") : QString::number (i->Status_.num_incomplete)));
				Ui_.LabelSeedsInSwarm_->
					setText ((i->Status_.num_complete == -1 ?
						  tr ("Unknown") : QString::number (i->Status_.num_complete)));
				Ui_.LabelConnectCandidates_->
					setText (QString::number (i->Status_.connect_candidates));
				Ui_.LabelUpBandwidthQueue_->
					setText (QString::number (i->Status_.up_bandwidth_queue));
				Ui_.LabelDownBandwidthQueue_->
					setText (QString::number (i->Status_.down_bandwidth_queue));
			}
			
			void TabWidget::UpdateFilesPage ()
			{
				if (TorrentSelectionChanged_)
				{
					Core::Instance ()->ResetFiles ();
					Ui_.FilesView_->expandAll ();
				}
				else
				{
					Core::Instance ()->UpdateFiles ();
					Ui_.FilesView_->expandAll ();
				}
			}
			
			void TabWidget::UpdatePeersPage ()
			{
				Core::Instance ()->UpdatePeers ();
			}
			
			void TabWidget::UpdatePiecesPage ()
			{
				Core::Instance ()->UpdatePieces ();
			}

			void TabWidget::on_OverallDownloadRateController__valueChanged (int val)
			{
				Core::Instance ()->SetOverallDownloadRate (val);
			}
			
			void TabWidget::on_OverallUploadRateController__valueChanged (int val)
			{
				Core::Instance ()->SetOverallUploadRate (val);
			}
			
			void TabWidget::on_DesiredRating__valueChanged (double val)
			{
				Core::Instance ()->SetDesiredRating (val);
			}
			
			void TabWidget::on_TorrentDownloadRateController__valueChanged (int val)
			{
				Core::Instance ()->SetTorrentDownloadRate (val);
			}
			
			void TabWidget::on_TorrentUploadRateController__valueChanged (int val)
			{
				Core::Instance ()->SetTorrentUploadRate (val);
			}
			
			void TabWidget::on_TorrentDesiredRating__valueChanged (double val)
			{
				Core::Instance ()->SetTorrentDesiredRating (val);
			}
			
			void TabWidget::on_TorrentManaged__stateChanged (int state)
			{
				Core::Instance ()->SetTorrentManaged (state == Qt::Checked);
			}
			
			void TabWidget::on_TorrentSequentialDownload__stateChanged (int state)
			{
				Core::Instance ()->SetTorrentSequentialDownload (state == Qt::Checked);
			}
			
			void TabWidget::on_TorrentSuperSeeding__stateChanged (int state)
			{
				Core::Instance ()->SetTorrentSuperSeeding (state == Qt::Checked);
			}
			
			void TabWidget::on_DownloadingTorrents__valueChanged (int newValue)
			{
				Core::Instance ()->SetMaxDownloadingTorrents (newValue);
			}
			
			void TabWidget::on_UploadingTorrents__valueChanged (int newValue)
			{
				Core::Instance ()->SetMaxUploadingTorrents (newValue);
			}
			
			void TabWidget::on_TorrentTags__editingFinished ()
			{
				Core::Instance ()->UpdateTags (Core::Instance ()->GetProxy ()->
						GetTagsManager ()->Split (Ui_.TorrentTags_->text ()));
			}

			void TabWidget::setTabWidgetSettings ()
			{
				Ui_.BoxSessionStats_->setVisible (XmlSettingsManager::Instance ()->
						property ("ActiveSessionStats").toBool ());
				Ui_.BoxAdvancedSessionStats_->setVisible (XmlSettingsManager::Instance ()->
						property ("ActiveAdvancedSessionStats").toBool ());
				Ui_.BoxPerTrackerStats_->setVisible (XmlSettingsManager::Instance ()->
						property ("ActiveTrackerStats").toBool ());
				Ui_.BoxCacheStats_->setVisible (XmlSettingsManager::Instance ()->
						property ("ActiveCacheStats").toBool ());
				Ui_.BoxTorrentStatus_->setVisible (XmlSettingsManager::Instance ()->
						property ("ActiveTorrentStatus").toBool ());
				Ui_.BoxTorrentAdvancedStatus_->setVisible (XmlSettingsManager::Instance ()->
						property ("ActiveTorrentAdvancedStatus").toBool ());
				Ui_.BoxTorrentInfo_->setVisible (XmlSettingsManager::Instance ()->
						property ("ActiveTorrentInfo").toBool ());
				Ui_.BoxTorrentPeers_->setVisible (XmlSettingsManager::Instance ()->
						property ("ActiveTorrentPeers").toBool ());
			}
		};
	};
};
