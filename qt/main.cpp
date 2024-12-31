#include "Config.h"
#include "Database.h"
#include "DigiByteCore.h"
#include "PlotManager.h"
#include "RPC/Server.h"
#include "RPCLoader.h"
#include "tabs/SyncTab.h"
#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QMovie>
#include <QProcess>
#include <QProgressBar>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <chrono>
#include <regex>

QProcess *coreProcess = nullptr;
QProgressBar *progressBar = nullptr;
QLabel *statusLabel = nullptr;
QLabel *timeLabel = nullptr;
QLabel *loadingSpinner = nullptr;
QMovie *spinnerMovie = nullptr;
QTabWidget *tabWidget = nullptr;
RPCLoader loader;  // Create a single instance of Loader

int previousSync = 0;
double estimatedTime = 0;
std::chrono::steady_clock::time_point lastUpdate;

void startCoreProcess() {
    if (coreProcess && coreProcess->state() != QProcess::NotRunning) {
        qDebug() << "Core process is already running.";
        return;
    }

    coreProcess = new QProcess();
    QString coreExecutable = QCoreApplication::applicationDirPath() + "/digiasset_core";
    coreProcess->start(coreExecutable);

    QObject::connect(coreProcess, &QProcess::errorOccurred, [](QProcess::ProcessError error) {
        qDebug() << "Core process error:" << error;
    });

    qDebug() << "Core process started: " << coreExecutable;
}

QString estimateTimeRemaining(int sync, int count) {
    using namespace std::chrono;

    int remaining = count - sync;

    auto now = steady_clock::now();
    double elapsed = duration_cast<seconds>(now - lastUpdate).count();

    int blocksSynced = sync - previousSync;
    double syncSpeed = blocksSynced / elapsed;

    if (syncSpeed > 0) {
        double currentEstimate = remaining / syncSpeed;
        estimatedTime = (0.7 * estimatedTime) + (0.3 * currentEstimate);  // Smoothing factor

        int days = static_cast<int>(estimatedTime) / 86400;
        int hours = (static_cast<int>(estimatedTime) % 86400) / 3600;
        int minutes = (static_cast<int>(estimatedTime) % 3600) / 60;
        int seconds = static_cast<int>(estimatedTime) % 60;

        if (days > 0) {
            return QString("%1 days %2 hrs %3 min %4 sec remaining")
                    .arg(days).arg(hours).arg(minutes).arg(seconds);
        } else if (hours > 0) {
            return QString("%1 hrs %2 min %3 sec remaining")
                    .arg(hours).arg(minutes).arg(seconds);
        } else {
            return QString("%1 min %2 sec remaining")
                    .arg(minutes).arg(seconds);
        }
    } else {
        return "Calculating time...";
    }
}


void updateLoadingProgress(QTimer *timer, QWidget &splash) {
    DigiByteCore& dgb = loader.getCore();

    try {
        Json::Value args = Json::arrayValue;
        Json::Value result = dgb.sendcommand("syncstate", args);

        int count = result["count"].asInt();
        int sync = result["sync"].asInt();
        int progress = count + sync;
        int percentage = (progress * 100) / count;

        progressBar->setValue(percentage);

        QString syncMessage;
        switch (sync) {
            case 1:
                syncMessage = "Sync Stopped. Something is wrong!";
                progressBar->setVisible(false);
                loadingSpinner->setVisible(true);
                spinnerMovie->start();
                break;
            case 2:
                syncMessage = "Initializing...";
                progressBar->setVisible(false);
                loadingSpinner->setVisible(true);
                spinnerMovie->start();
                break;
            case 3:
                syncMessage = "Rewinding blocks...";
                progressBar->setVisible(false);
                loadingSpinner->setVisible(true);
                spinnerMovie->start();
                break;
            case 4:
                syncMessage = "Optimizing blocks...";
                progressBar->setVisible(false);
                loadingSpinner->setVisible(true);
                spinnerMovie->start();
                break;
            default:
                syncMessage = "Syncing Blocks: " + QString::number(progress) + " / " + QString::number(count);
                progressBar->setVisible(true);
                loadingSpinner->setVisible(false);
                spinnerMovie->stop();

                if (previousSync != sync) {
                    QString timeEstimate = estimateTimeRemaining(sync, count);
                    timeLabel->setText("Estimated time: " + timeEstimate);
                    previousSync = sync;
                    lastUpdate = std::chrono::steady_clock::now();
                }
                break;
        }
        statusLabel->setText(syncMessage);

        if (sync == 0) {
            timer->stop();
            splash.close();

            tabWidget = new QTabWidget();
            SyncTab *syncTab = new SyncTab();
            tabWidget->addTab(syncTab, "Sync Status");
            tabWidget->resize(800, 600);
            tabWidget->show();
        }

    } catch (const DigiByteException &e) {
        statusLabel->setText("Error: " + QString::fromStdString(e.getMessage()));
    }
}

void showLoadingScreen(QApplication &app) {
    QWidget splash;
    splash.setFixedSize(400, 250);
    splash.setWindowTitle("Loading...");

    QVBoxLayout *layout = new QVBoxLayout(&splash);
    statusLabel = new QLabel("Starting DigiAsset Core...");
    statusLabel->setAlignment(Qt::AlignCenter);

    progressBar = new QProgressBar();
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setVisible(false);

    timeLabel = new QLabel("Estimated time: Calculating...");
    timeLabel->setAlignment(Qt::AlignCenter);

    loadingSpinner = new QLabel();
    QString spinnerPath = QCoreApplication::applicationDirPath() + "/images/spinner.gif";
    spinnerMovie = new QMovie(spinnerPath);
    loadingSpinner->setMovie(spinnerMovie);
    loadingSpinner->setAlignment(Qt::AlignCenter);
    loadingSpinner->setVisible(true);
    spinnerMovie->start();

    layout->addWidget(statusLabel);
    layout->addWidget(progressBar);
    layout->addWidget(timeLabel);
    layout->addWidget(loadingSpinner);

    splash.show();

    QTimer *timer = new QTimer(&splash);
    QObject::connect(timer, &QTimer::timeout, [&]() {
        updateLoadingProgress(timer, splash);
    });

    lastUpdate = std::chrono::steady_clock::now();
    timer->start(15000);
    app.exec();
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    startCoreProcess();
    showLoadingScreen(app);

    return 0;
}
