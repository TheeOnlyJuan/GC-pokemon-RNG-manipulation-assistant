#include "MainWindow.h"

#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QRegularExpression>
#include <QVBoxLayout>
#include <QtConcurrent>

#include <sstream>
#include <thread>

#include "../PokemonRNGSystem/Colosseum/ColosseumRNGSystem.h"
#include "../PokemonRNGSystem/XD/GaleDarknessRNGSystem.h"
#include "GUICommon.h"
#include "SPokemonRNG.h"
#include "SeedFinder/SeedFinderWizard.h"
#include "Settings/DlgSettings.h"
#include "Settings/SConfig.h"

MainWindow::MainWindow()
{
  initialiseWidgets();
  makeLayouts();
  makeMenus();

  m_precalcFuture = QFuture<void>();
  m_dlgProgressPrecalc = nullptr;

  connect(this, &MainWindow::onPrecalcDone, this, &MainWindow::precalcDone);
  connect(this, &MainWindow::onUpdatePrecalcProgress, this, [=](long value) {
    if (value == m_dlgProgressPrecalc->maximum())
      m_dlgProgressPrecalc->setLabelText(tr("ディスクへの書き込み..."));
    else
      m_dlgProgressPrecalc->setValue(value);
  });

  if (SConfig::getInstance().getRestorePreviousWindowGeometry())
    restoreGeometry(SConfig::getInstance().getPreviousWindowGeometry());
}

MainWindow::~MainWindow()
{
  SPokemonRNG::deleteSystem();
  delete m_dlgProgressPrecalc;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  SConfig::getInstance().setPreviousWindowGeometry(saveGeometry());
}

void MainWindow::initialiseWidgets()
{
  m_cmbGame = new QComboBox;
  m_cmbGame->addItems(GUICommon::gamesStr);
  m_cmbGame->addItem(tr("--ゲームを選ぶ--"));
  m_cmbGame->setCurrentIndex(static_cast<int>(GUICommon::gameSelection::Unselected));
  connect(m_cmbGame, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &MainWindow::gameChanged);

  m_btnSettings = new QPushButton(tr("&設定"));
  connect(m_btnSettings, &QPushButton::clicked, this, &MainWindow::openSettings);

  m_btnStartSeedFinder = new QPushButton(tr("&Seedを見つける"));
  connect(m_btnStartSeedFinder, &QPushButton::clicked, this, &MainWindow::startSeedFinder);
  m_btnStartSeedFinder->setEnabled(false);

  m_btnReset = new QPushButton(tr("&リセット"));
  connect(m_btnReset, &QPushButton::clicked, this, &MainWindow::resetPredictor);
  m_btnReset->setEnabled(false);

  m_chkFilterUnwantedPredictions = new QCheckBox(tr("希望する予測のみ表示"));
  m_chkFilterUnwantedPredictions->setChecked(false);
  connect(m_chkFilterUnwantedPredictions, &QCheckBox::stateChanged, this,
          [=](int state) { m_predictorWidget->filterUnwanted(state == Qt::Checked); });

  m_edtManualSeed = new QLineEdit();
  m_edtManualSeed->setEnabled(false);
  m_btnSetSeedManually = new QPushButton("入力");
  connect(m_btnSetSeedManually, &QPushButton::clicked, this, &MainWindow::setSeedManually);
  m_btnSetSeedManually->setEnabled(false);

  m_lblCurrentSeed = new QLabel("  ????  ");
  m_lblStoredSeed = new QLabel("  なし  ");

  m_btnStoreSeed = new QPushButton("現在のSeed値を保存する");
  connect(m_btnStoreSeed, &QPushButton::clicked, this, &MainWindow::storeSeed);
  m_btnStoreSeed->setEnabled(false);

  m_btnRestoreSeed = new QPushButton("保存したSeed値をロードする");
  connect(m_btnRestoreSeed, &QPushButton::clicked, this, &MainWindow::restoreSeed);
  m_btnRestoreSeed->setEnabled(false);

  m_btnRerollPrediciton = new QPushButton(tr("リロール\n（リロール1回につきチーム生成を1回行ってください）"));
  connect(m_btnRerollPrediciton, &QPushButton::clicked, this, &MainWindow::singleRerollPredictor);
  m_btnRerollPrediciton->setEnabled(false);

  m_btnAutoRerollPrediciton =
      new QPushButton(tr("自動リロール\n（望ましいSeedが検索されるまでリロールします）"));
  connect(m_btnAutoRerollPrediciton, &QPushButton::clicked, this, &MainWindow::autoRerollPredictor);
  m_btnAutoRerollPrediciton->setEnabled(false);

  m_lblRerollCount = new QLabel(QString::number(m_rerollCount), this);

  m_predictorWidget = new PredictorWidget(this);
  m_statsReporterWidget = new StatsReporterWidget(this);
  m_statsReporterWidget->setDisabled(true);
  connect(m_predictorWidget, &PredictorWidget::selectedPredictionChanged, this,
          [=](BaseRNGSystem::StartersPrediction prediction) {
            m_statsReporterWidget->startersPredictionChanged(prediction);
            m_statsReporterWidget->setEnabled(true);
          });
}

void MainWindow::makeLayouts()
{
  QHBoxLayout* buttonsLayout = new QHBoxLayout;
  buttonsLayout->addWidget(m_btnSettings);
  buttonsLayout->addWidget(m_btnStartSeedFinder);
  buttonsLayout->addWidget(m_btnReset);

  QHBoxLayout* setSeedLayout = new QHBoxLayout;
  setSeedLayout->addWidget(new QLabel("手動でSeed値を入力する:"));
  setSeedLayout->addWidget(m_edtManualSeed);
  setSeedLayout->addWidget(m_btnSetSeedManually);

  QHBoxLayout* filterUnwantedLayout = new QHBoxLayout;
  filterUnwantedLayout->addStretch();
  filterUnwantedLayout->addWidget(m_chkFilterUnwantedPredictions);
  filterUnwantedLayout->addStretch();

  QLabel* lblReroll = new QLabel(tr("リロール数: "), this);

  QHBoxLayout* rerollCountLayout = new QHBoxLayout;
  rerollCountLayout->addStretch();
  rerollCountLayout->addWidget(lblReroll);
  rerollCountLayout->addWidget(m_lblRerollCount);
  rerollCountLayout->addStretch();

  QHBoxLayout* seedInfoLayout = new QHBoxLayout;
  seedInfoLayout->addStretch();
  seedInfoLayout->addWidget(new QLabel("保存したSeed値:"));
  seedInfoLayout->addWidget(m_lblStoredSeed);
  seedInfoLayout->addSpacing(20);
  seedInfoLayout->addWidget(new QLabel("現在のSeed値:"));
  seedInfoLayout->addWidget(m_lblCurrentSeed);
  seedInfoLayout->addStretch();

  QHBoxLayout* seedStoreRestoreLayout = new QHBoxLayout;
  seedStoreRestoreLayout->addWidget(m_btnStoreSeed);
  seedStoreRestoreLayout->addWidget(m_btnRestoreSeed);

  QHBoxLayout* rerollButtonsLayout = new QHBoxLayout;
  rerollButtonsLayout->addWidget(m_btnRerollPrediciton);
  rerollButtonsLayout->addWidget(m_btnAutoRerollPrediciton);

  QVBoxLayout* predictorLayout = new QVBoxLayout;
  predictorLayout->addWidget(m_cmbGame);
  predictorLayout->addLayout(buttonsLayout);
  predictorLayout->addLayout(setSeedLayout);
  predictorLayout->addLayout(filterUnwantedLayout);
  predictorLayout->addWidget(m_predictorWidget);
  predictorLayout->addLayout(rerollButtonsLayout);
  predictorLayout->addLayout(rerollCountLayout);
  predictorLayout->addLayout(seedStoreRestoreLayout);
  predictorLayout->addLayout(seedInfoLayout);

  QFrame* separatorLine = new QFrame();
  separatorLine->setFrameShape(QFrame::VLine);

  QHBoxLayout* mainLayout = new QHBoxLayout;
  mainLayout->addLayout(predictorLayout);
  mainLayout->addWidget(separatorLine);
  mainLayout->addWidget(m_statsReporterWidget);

  QWidget* mainWidget = new QWidget;
  mainWidget->setLayout(mainLayout);
  setCentralWidget(mainWidget);

  setWindowTitle("ポケモン乱数調整ツール");
}

void MainWindow::makeMenus()
{
  m_menuFile = menuBar()->addMenu(tr("&ファイル"));
  m_actGenerationPrecalcFile =
      m_menuFile->addAction(tr("選択したゲームの事前計算を生成する"), this,
                            [=]() { generatePrecalc(); });
  m_actGenerationPrecalcFile->setEnabled(false);
  m_menuFile->addAction(tr("&終了"), this, [=]() { close(); });

  m_menuEdit = menuBar()->addMenu(tr("&編集"));
  m_menuEdit->addAction(tr("&設定"), this, [=]() { openSettings(); });

  m_menuHelp = menuBar()->addMenu(tr("&ヘルプ"));
  m_menuHelp->addAction(tr("&インフォ"), this, [=]() {
    QString title = tr("About GameCube Pokémon RNG assistant");
    QString text =
        "Version 1.0.2J\n\n" +
        tr("A program to allow the manipulation of the starters RNG in Pokémon Colosseum and "
           "Pokémon XD: Gale of darkness.\n\nThis program is licensed under the MIT license. "
           "You should have received a copy of the MIT license along with this program");
    QMessageBox::about(this, title, text);
  });
}

void MainWindow::gameChanged()
{
  GUICommon::gameSelection selection =
      static_cast<GUICommon::gameSelection>(m_cmbGame->currentIndex());
  if (selection == GUICommon::gameSelection::Colosseum)
  {
    SPokemonRNG::setCurrentSystem(new ColosseumRNGSystem());
  }
  else if (selection = GUICommon::gameSelection::XD)
  {
    SPokemonRNG::setCurrentSystem(new GaleDarknessRNGSystem());
    GaleDarknessRNGSystem::setPalEnabled(SConfig::getInstance().getXDPalVersionEnabled());
  }

  if (m_cmbGame->count() == static_cast<int>(GUICommon::gameSelection::Unselected) + 1)
  {
    m_cmbGame->removeItem(static_cast<int>(GUICommon::gameSelection::Unselected));
    m_btnStartSeedFinder->setEnabled(true);
    m_actGenerationPrecalcFile->setEnabled(true);
  }
  m_predictorWidget->switchGame(selection);
  m_btnReset->setEnabled(false);
  m_btnRerollPrediciton->setEnabled(false);
  m_btnAutoRerollPrediciton->setEnabled(false);
  m_rerollCount = 0;
  m_lblRerollCount->setText(QString::number(m_rerollCount));
  m_edtManualSeed->setEnabled(true);
  m_btnSetSeedManually->setEnabled(true);
  m_lblCurrentSeed->setText("  ????  ");
  m_seedSet = false;
  m_lblStoredSeed->setText("  なし  ");

  m_statsReporterWidget->gameChanged(selection);
  m_statsReporterWidget->setDisabled(true);
}

void MainWindow::setCurrentSeed(u32 seed, int rerollCount)
{
  m_currentSeed = seed;

  GUICommon::gameSelection selection =
      static_cast<GUICommon::gameSelection>(m_cmbGame->currentIndex());

  m_lblCurrentSeed->setText(QString::number(m_currentSeed, 16).toUpper());
  std::vector<BaseRNGSystem::StartersPrediction> predictions =
      SPokemonRNG::getCurrentSystem()->predictStartersForNbrSeconds(
          m_currentSeed, SConfig::getInstance().getPredictionTime());
  m_predictorWidget->setStartersPrediction(predictions);
  m_predictorWidget->updateGUI(selection);
  m_predictorWidget->filterUnwanted(m_chkFilterUnwantedPredictions->isChecked());
  m_btnReset->setEnabled(true);
  m_btnRerollPrediciton->setEnabled(true);
  m_btnAutoRerollPrediciton->setEnabled(true);
  m_rerollCount = rerollCount;
  m_lblRerollCount->setText(QString::number(m_rerollCount));
  m_btnStoreSeed->setEnabled(true);
}

void MainWindow::startSeedFinder()
{
  QFileInfo info(QString::fromStdString(SPokemonRNG::getCurrentSystem()->getPrecalcFilename()));
  if (!(info.exists() && info.isFile()))
  {
    QMessageBox* msg = new QMessageBox(
        QMessageBox::Critical, "事前計算ファイルが見つかりません",
        "Seed値検索に必要な事前計算ファイル「" + info.fileName() +
            "」（ポケモンコロシアム専用のファイル）が本プログラムのディレクトリ内に見つかりませんでした。"
			"お手数ですが、当ファイルをダウンロード（本プログラムがリリースされている場所に配置されています）するか、"
			"ファイルメニューから生成を行うかしてください。"
			"ただし、当ファイルの生成には数時間を要するので、ダウンロードすることをお勧めします。",
        QMessageBox::Ok);
    msg->exec();
    delete msg;
    return;
  }

  GUICommon::gameSelection selection =
      static_cast<GUICommon::gameSelection>(m_cmbGame->currentIndex());
  SeedFinderWizard* wizard = new SeedFinderWizard(this, selection);
  if (wizard->exec() == QDialog::Accepted)
  {
    setCurrentSeed(wizard->getSeeds()[0], 0);
    m_seedSet = true;
  }

  delete wizard;
}

void MainWindow::resetPredictor()
{
  GUICommon::gameSelection selection =
      static_cast<GUICommon::gameSelection>(m_cmbGame->currentIndex());
  m_predictorWidget->resetPredictor(selection);
  m_btnReset->setEnabled(false);
  m_btnRerollPrediciton->setEnabled(false);
  m_btnAutoRerollPrediciton->setEnabled(false);
  m_rerollCount = 0;
  m_lblRerollCount->setText(QString::number(m_rerollCount));
  m_btnStoreSeed->setEnabled(false);
  m_btnRestoreSeed->setEnabled(false);
  m_lblCurrentSeed->setText("  ????  ");
  m_seedSet = false;
  m_lblStoredSeed->setText("  なし  ");
  m_statsReporterWidget->reset();
  m_statsReporterWidget->setDisabled(true);
}

void MainWindow::storeSeed()
{
  m_storedSeed = m_currentSeed;
  m_storedRerollCount = m_rerollCount;
  m_lblStoredSeed->setText(QString::number(m_storedSeed, 16).toUpper());
  m_btnRestoreSeed->setEnabled(true);
}

void MainWindow::restoreSeed()
{
  setCurrentSeed(m_storedSeed, m_storedRerollCount);
}

void MainWindow::setSeedManually()
{
  QRegularExpression hexMatcher("^[0-9A-F]{1,8}$", QRegularExpression::CaseInsensitiveOption);
  QRegularExpressionMatch match = hexMatcher.match(m_edtManualSeed->text());
  if (match.hasMatch())
  {
    std::stringstream ss(m_edtManualSeed->text().toStdString());
    ss >> std::hex;
    u32 seed = 0;
    ss >> seed;
    setCurrentSeed(seed, 0);
    m_seedSet = true;
  }
  else
  {
    QMessageBox* msg = new QMessageBox(QMessageBox::Critical, "無効seed",
                                       "３２ビット１６進法 数のみが許可されています。",
                                       QMessageBox::Ok);
    msg->exec();
    delete msg;
  }
}

void MainWindow::singleRerollPredictor()
{
  rerollPredictor(true);
  m_lblCurrentSeed->setText(QString::number(m_currentSeed, 16).toUpper());
}

bool MainWindow::rerollPredictor(bool withGuiUpdates)
{
  std::vector<int> dummyCriteria;
  for (int i = 0; i < 6; i++)
    dummyCriteria.push_back(-1);

  GUICommon::gameSelection selection =
      static_cast<GUICommon::gameSelection>(m_cmbGame->currentIndex());
  SPokemonRNG::getCurrentSystem()->generateBattleTeam(m_currentSeed, dummyCriteria);
  std::vector<BaseRNGSystem::StartersPrediction> predictions =
      SPokemonRNG::getCurrentSystem()->predictStartersForNbrSeconds(
          m_currentSeed, SConfig::getInstance().getPredictionTime());
  m_rerollCount++;

  m_predictorWidget->setStartersPrediction(predictions);
  bool desiredStarterFound = m_predictorWidget->desiredPredictionFound(selection);

  if (withGuiUpdates)
  {
    m_predictorWidget->updateGUI(selection);
    m_predictorWidget->filterUnwanted(m_chkFilterUnwantedPredictions->isChecked());
    m_lblRerollCount->setText(QString::number(m_rerollCount));
    m_statsReporterWidget->setDisabled(true);
  }
  return desiredStarterFound;
}

void MainWindow::autoRerollPredictor()
{
  QDialog* autoRerollDlg = new QDialog;
  autoRerollDlg->setModal(true);
  autoRerollDlg->setWindowTitle("自動リロール");
  QLabel* lblAutoRerolling = new QLabel(autoRerollDlg);
  lblAutoRerolling->setText("自動リロールしています。少々お待ち下さい...");
  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(lblAutoRerolling);
  autoRerollDlg->setLayout(mainLayout);
  autoRerollDlg->show();

  bool desiredPredictionFound = false;
  int nbrRerolls = 0;
  for (nbrRerolls; nbrRerolls < SConfig::getInstance().getMaxAutoReroll(); nbrRerolls++)
  {
    if (rerollPredictor(false))
    {
      desiredPredictionFound = true;
      break;
    }
    // No idea why, but this is required for the dialog to keep rendering which is odd because one
    // call to this before the loop should have been enough, but apparently, it isn't
    QCoreApplication::processEvents();
  }
  autoRerollDlg->close();
  delete autoRerollDlg;

  if (desiredPredictionFound)
  {
    QString lastBattleConfirmationStr =
        QString::fromStdString(SPokemonRNG::getCurrentSystem()->getLastObtainedCriteriasString());
    QMessageBox* msg =
        new QMessageBox(QMessageBox::Information, "望ましいseedが見つかりました",
                        "" + QString::number(nbrRerolls + 1) +
                            "回のリロールで希望のseed値が見つかります。表が更新されます\n\n"
                            "最後のチーム編成は、以下のように表示されます:\n" +
                            lastBattleConfirmationStr,
                        QMessageBox::Ok);
    msg->exec();
    delete msg;
  }
  else
  {
    QMessageBox* msg = new QMessageBox(
        QMessageBox::Critical, "望ましいseedが見つかりません",
        QString::number(nbrRerolls) + "リロール制限に達しました。"
            "望ましいseedが見つかりません。"
            "再度自動リロールをお試しください。",
        QMessageBox::Ok);
    msg->exec();
    delete msg;
  }
  GUICommon::gameSelection selection =
      static_cast<GUICommon::gameSelection>(m_cmbGame->currentIndex());
  m_predictorWidget->updateGUI(selection);
  m_predictorWidget->filterUnwanted(m_chkFilterUnwantedPredictions->isChecked());
  m_lblRerollCount->setText(QString::number(m_rerollCount));
  m_lblCurrentSeed->setText(QString::number(m_currentSeed, 16).toUpper());
  m_statsReporterWidget->setDisabled(true);
}

void MainWindow::openSettings()
{
  DlgSettings* dlg = new DlgSettings(this);
  int dlgResult = dlg->exec();
  delete dlg;
  if (dlgResult == QDialog::Accepted && m_seedSet)
  {
    // Refresh the predictor
    setCurrentSeed(m_currentSeed, m_rerollCount);
  }
}

void MainWindow::generatePrecalc()
{
  QMessageBox* msg = new QMessageBox(
      QMessageBox::Warning, "事前計算ファイル",
      "選択したゲームの事前計算を生成しようとしています。"
      "この作業はお使いのPCのCPUやスレッド数にもよりますが、数時間かかる事が予想されます。"
      "代わりに、本プログラムがリリースされている場所に配置されているファイルをダウンロードすること"
      "も可能です。\n本当に事前計算ファイルの生成を開始しますか？ ",
      QMessageBox::NoButton, this);
  QAbstractButton* pButtonNo = msg->addButton(tr("いいえ"), QMessageBox::NoRole);
  QAbstractButton* pButtonYes = msg->addButton(tr("はい"), QMessageBox::YesRole);
  msg->exec();
  if (msg->clickedButton() == pButtonYes)
  {
    unsigned int threadCount = SConfig::getInstance().getThreadLimit();
    if (threadCount == 0)
      threadCount = std::thread::hardware_concurrency();

    delete m_dlgProgressPrecalc;
    m_dlgProgressPrecalc = new QProgressDialog(this);
    m_dlgProgressPrecalc->setWindowModality(Qt::WindowModal);
    m_dlgProgressPrecalc->setWindowTitle(tr("事前計算ファイルを作っています"));
    m_dlgProgressPrecalc->setCancelButtonText(tr("キャンセル"));
    m_dlgProgressPrecalc->setMinimum(0);
    m_dlgProgressPrecalc->setLabelText("事前計算 " +
                                       QString::number(Common::nbrPossibleSeeds) + " seeds");
    m_dlgProgressPrecalc->setMaximum(65536);
    connect(m_dlgProgressPrecalc, &QProgressDialog::canceled, this, [=]() {
      m_cancelPrecalc = true;
      m_precalcFuture.waitForFinished();
    });
    m_dlgProgressPrecalc->setValue(0);

    QtConcurrent::run([=] {
      SPokemonRNG::getCurrentSystem()->generatePrecalculationFile(
          threadCount, [=](long value) { emit onUpdatePrecalcProgress(value); },
          [=]() { return m_cancelPrecalc; });
      if (!m_cancelPrecalc)
        emit onPrecalcDone();
    });
    m_precalcFuture.waitForFinished();
    m_cancelPrecalc = false;
  }
  delete msg;
}

void MainWindow::precalcDone()
{
  m_dlgProgressPrecalc->setValue(m_dlgProgressPrecalc->maximum());
  QMessageBox* msg =
      new QMessageBox(QMessageBox::Information, "結果",
                      "事前計算成功", QMessageBox::Ok);
  msg->exec();
  delete msg;
}
