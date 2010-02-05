#include "PlaybackWidget.h"

#include "PlaybackWidget.h"

#include <QDate>
#include <QDirIterator>
#include <QPainter>
#include <QMenu>
#include <QDebug>

PlaybackWidget::PlaybackWidget(QWidget *parent)
	: QWidget(parent)
	, m_desiredSize(320,240)
	, m_dailyRecordingPath("")
	, m_currentFrame(0)
	, m_playbackFps(30)
	, m_currentPlaybackDate("")
	, m_status(Stopped)
{
	connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(updateImage()));
	
	setPlaybackFps(m_playbackFps); // starts timer as well
	
 	QDate date = QDate::currentDate();
 	m_currentPlaybackDate = date.toString("yyyy-MM-dd");
};

PlaybackWidget::~PlaybackWidget()
{
	
}
	
void PlaybackWidget::setDesiredSize(QSize size)
{
	m_desiredSize = size;
}

QSize PlaybackWidget::sizeHint() const
{
	return m_desiredSize;
}

void PlaybackWidget::setDailyRecordingPath(const QString& path)
{
	m_dailyRecordingPath = path;
}

void PlaybackWidget::setPlaybackFps(double d)
{
	m_playbackFps = d;
	m_updateTimer.stop();
	m_updateTimer.setInterval((int)(1000/m_playbackFps));
	m_updateTimer.start();
}

void PlaybackWidget::setCurrentFrame(int d)
{
	m_currentFrame = d;
	
	QFileInfo info = m_files[m_currentFrame];
	m_currentImage.load(info.canonicalFilePath());
			
	if(!m_currentImage.isNull() && m_desiredSize != m_currentImage.size())
	{
		m_desiredSize = m_currentImage.size();
		resize(m_desiredSize);
	}	
	
	update();
}

bool PlaybackWidget_sortQFileInfoByModified(const QFileInfo &a, const QFileInfo &b)
{
	return a.lastModified().toTime_t() < b.lastModified().toTime_t();
}

// date should be in YYYY-MM-DD
void PlaybackWidget::loadPlaybackDate(const QString & date)
{
	m_currentPlaybackDate = date;
	
	QStringList parts = date.split("-");
	QString path = m_dailyRecordingPath;
	
	path = "/ha/cameras/motion/cam2/jpeg/%Y/%m/%d";
	
	path.replace("%Y",parts[0]);
	path.replace("%m",parts[1]);
	path.replace("%d",parts[2]);
	//path = "/ha/cameras/motion/cam9/jpeg/2010/"; //02/05/";
	//path = "S:\\Security Camera Recordings\\cam1\\2010\\01\\31";
	qDebug() << "loadPlaybackDate("<<date<<"): Reading from path"<<path<<" (kinda hardocded!)";
	//QDir dir(path);
	
	//m_files = dir.entryInfoList(QStringList() << "*.jpg" << "*.JPG", QDir::NoFilter, QDir::Time);

	m_files.clear();

	QDirIterator it(path, QDirIterator::Subdirectories);
	while (it.hasNext())
	{
		qDebug() << it.next();
		if(it.fileInfo().isFile())
		{
			m_files << it.fileInfo();
		}
	}
	
	if(!m_files.isEmpty())
		qSort(m_files.begin(),m_files.end(), PlaybackWidget_sortQFileInfoByModified);
	
	qDebug() << "Found "<<m_files.size()<<" files.";
	m_currentFrame = 0;
	
	setStatus(Playing);
}	
	
	
void PlaybackWidget::paintEvent(QPaintEvent */*event*/)
{
	QPainter painter(this);
	painter.fillRect(rect(),Qt::black);
	
	if(!m_currentImage.isNull())
	{
		painter.drawImage(rect(),m_currentImage);
		
	}
	painter.setPen(QPen(Qt::white,2));
	painter.setBrush(Qt::black);
	painter.drawText(5,15,QString("%3 - %1/%2").arg(m_currentFrame).arg(m_files.size()).arg(status() == Playing ? "PLAYING" : "PAUSED/STOPPED"));
	
}
void PlaybackWidget::updateImage()
{
	if(status() == Playing)
	{
		if(m_currentFrame < m_files.size())
		{
			setCurrentFrame(m_currentFrame);
			m_currentFrame++;
		}
		else
		{
			setStatus(Stopped);
		}
	}
}

void PlaybackWidget::setStatus(Status s)
{
	m_status = s;
	update();
}
