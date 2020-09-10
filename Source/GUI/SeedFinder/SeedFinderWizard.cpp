#include "SeedFinderWizard.h"

#include <thread>

#include <QAbstractButton>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QtConcurrent>
#include <QFont>

#include "../../PokemonRNGSystem/XD/GaleDarknessRNGSystem.h"
#include "../SPokemonRNG.h"
#include "../Settings/SConfig.h"

int SeedFinderWizard::numberPass = 1;

SeedFinderWizard::SeedFinderWizard(QWidget* parent, const GUICommon::gameSelection game)
    : QWizard(parent), m_game(game)
{
  numberPass = 1;
  m_cancelSeedFinderPass = false;
  m_seedFinderFuture = QFuture<void>();
  //setPage(pageID::Start, new StartPage(this, game));
  setPage(pageID::Instructions, new InstructionsPage(this, game));
  setPage(pageID::SeedFinderPass, getSeedFinderPassPageForGame());
  setStartId(pageID::Instructions);

  setOptions(QWizard::HaveCustomButton1);
  setButtonText(QWizard::CustomButton1, "次 >");
  connect(this, &SeedFinderWizard::customButtonClicked, this, [=](int which) {
    if (which == QWizard::CustomButton1)
      nextSeedFinderPass();
  });
  connect(this, &SeedFinderWizard::onUpdateSeedFinderProgress, this, [=](int nbrSeedsSimulated) {
    if (!m_cancelSeedFinderPass)
      static_cast<SeedFinderPassPage*>(currentPage())->setSeedFinderProgress(nbrSeedsSimulated);
  });
  connect(this, &SeedFinderWizard::onSeedFinderPassDone, this,
          &SeedFinderWizard::seedFinderPassDone);

  connect(button(QWizard::NextButton), &QAbstractButton::clicked, this,
          &SeedFinderWizard::pageChanged);

  setWindowTitle(tr("Seed値特定"));
  setWizardStyle(QWizard::ModernStyle);

  QList<QWizard::WizardButton> btnlayout;
  btnlayout << QWizard::Stretch << QWizard::CancelButton << QWizard::NextButton;
  setButtonLayout(btnlayout);
  setFixedWidth(650);
}

SeedFinderWizard::~SeedFinderWizard()
{
  for (auto page : m_passPages)
    delete page;
}

void SeedFinderWizard::keyPressEvent(QKeyEvent* event)
{
  if ((event->key() == Qt::Key_Enter) || (event->key() == Qt::Key_Return))
  {
    if (currentId() >= pageID::SeedFinderPass)
      static_cast<QPushButton*>(button(QWizard::CustomButton1))->click();
  }
  QWizard::keyPressEvent(event);
}

std::vector<u32> SeedFinderWizard::getSeeds() const
{
  return m_seeds;
}

SeedFinderPassPage* SeedFinderWizard::getSeedFinderPassPageForGame()
{
  SeedFinderPassPage* page;
  switch (m_game)
  {
  case GUICommon::gameSelection::Colosseum:
    page = new SeedFinderPassColosseum(this, static_cast<int>(m_seeds.size()));
    break;
  case GUICommon::gameSelection::XD:
    page = new SeedFinderPassXD(this, static_cast<int>(m_seeds.size()));
    break;
  default:
    return nullptr;
  }
  m_passPages.append(page);
  QString strResultStatus("　？？？　");
  if (m_seeds.size() > 1)
    strResultStatus = QString::number(m_seeds.size()) + QString(" 検索結果");
  page->setTitle("Seed値特定　試行" + QString::number(numberPass) + "回目 (" + strResultStatus + ")");
  QFont f;
  f.setPointSize(12);
  page->setFont(f);
  adjustSize();
  return page;
}

void SeedFinderWizard::nextSeedFinderPass()
{
  SeedFinderPassPage* page = static_cast<SeedFinderPassPage*>(currentPage());

  button(QWizard::CustomButton1)->setEnabled(false);

  page->showSeedFinderProgress(true);
  unsigned int threadCount = SConfig::getInstance().getThreadLimit();
  if (threadCount == 0)
    threadCount = std::thread::hardware_concurrency();

  m_seedFinderFuture = QtConcurrent::run([=] {
    SPokemonRNG::getCurrentSystem()->seedFinderPass(
        threadCount, page->obtainCriteria(), m_seeds,
        [=](long int nbrSeedsSimulated) { emit onUpdateSeedFinderProgress(nbrSeedsSimulated); },
        [=] { return m_cancelSeedFinderPass; });
    if (!m_cancelSeedFinderPass)
      emit onSeedFinderPassDone();
  });
}

void SeedFinderWizard::seedFinderPassDone()
{
  if (m_seeds.size() <= 1)
  {
    SeedFinderPassPage* page = static_cast<SeedFinderPassPage*>(currentPage());
    page->setSeedFinderDone(true);
    QList<QWizard::WizardButton> layout;
    layout << QWizard::Stretch << QWizard::FinishButton;
    setButtonLayout(layout);
    m_seedFinderDone = true;
    if (m_seeds.size() == 1)
      setPage(pageID::End, new EndPage(this, true, m_game, m_seeds[0]));
    else
      setPage(pageID::End, new EndPage(this, false, m_game));
    QWizard::next();
  }
  else
  {
    numberPass++;
    setPage(numberPass + pageID::SeedFinderPass, getSeedFinderPassPageForGame());
    button(QWizard::CustomButton1)->setEnabled(true);
    QWizard::next();
  }
}

void SeedFinderWizard::pageChanged()
{
  if (currentId() == pageID::SeedFinderPass)
  {
    QList<QWizard::WizardButton> layout;
    layout << QWizard::Stretch << QWizard::CancelButton << QWizard::CustomButton1;
    setButtonLayout(layout);
  }
}

void SeedFinderWizard::accept()
{
  if (m_seeds.size() == 1)
    QDialog::accept();
  else
    QDialog::reject();
}

void SeedFinderWizard::reject()
{
  if (m_seedFinderDone)
  {
    accept();
  }
  else
  {
    /*
    QMessageBox* cancelPrompt =
        new QMessageBox(QMessageBox::Question, "Seed Finder Cancellation",
                        "Are you sure you want to cancel the seed finding procedure?",
                        QMessageBox::No | QMessageBox::Yes, this);
    cancelPrompt->exec();
    if (cancelPrompt->result() == QMessageBox::Yes)
    {
      m_cancelSeedFinderPass = true;
      m_seedFinderFuture.waitForFinished();
      QWizard::reject();
    } 
	*/
    m_cancelSeedFinderPass = true;
    m_seedFinderFuture.waitForFinished();
    QWizard::reject();
    //delete cancelPrompt;
  }
}

//This page is currently pretty useless so it is emitted
StartPage::StartPage(QWidget* parent, const GUICommon::gameSelection game) : QWizardPage(parent)
{
  setTitle(tr("Introduction"));

  QLabel* label = new QLabel("This wizard will guide you through finding your seed in " +
                             GUICommon::gamesStr[game] + ".\n\nPress \"Next\" to continue.");
  label->setWordWrap(true);

  m_chkSkipInstructionPage = new QCheckBox(tr("Skip the instruction page"));
  m_chkSkipInstructionPage->setChecked(SConfig::getInstance().getSkipInstructionPage());

  QVBoxLayout* mainlayout = new QVBoxLayout;
  mainlayout->addWidget(label);
  mainlayout->addWidget(m_chkSkipInstructionPage);
  setLayout(mainlayout);
}

int StartPage::nextId() const
{
  SConfig::getInstance().setSkipInstructionPage(m_chkSkipInstructionPage->isChecked());
  if (m_chkSkipInstructionPage->isChecked())
    return SeedFinderWizard::pageID::SeedFinderPass;
  else
    return SeedFinderWizard::pageID::Instructions;
}

InstructionsPage::InstructionsPage(QWidget* parent, const GUICommon::gameSelection game)
    : QWizardPage(parent)
{
  setTitle(tr("使い方"));
  //setSubTitle(tr("Follow these detailled instructions before starting the seed finding procedure"));

  QLabel* lblSummary = new QLabel(
      "ここでは、「とにかくバトル」のチーム編成のパターンを何回か入力することで現在のseed値を絞ります。",
      this);
  lblSummary->setWordWrap(true);

  switch (game)
  {
  case GUICommon::gameSelection::Colosseum:
    m_lblGameInstructions = new QLabel(tr(
        "\nゲームを開始してメインメニューに入り、「対戦モード」→「とにかくバトル」→「シングルバトル」→「さいきょう」の順に選択します。\n"
           "ランダムに生成されたチーム編成が表示されるので、\n表示された「プレイヤーの名前」と「先頭のポケモンの名前 をチェック欄に正しく記入します。\n"
		   "記入が終わったらゲームではとにかくバトルの画面に戻り、次のパスに進みます。\n"
		   "（ここで間違って戦闘を開始したり、メインメニューに戻ると正しく機能しなくなります。）\n"
           "以降、何度か同じ作業を繰り返すとseed値が特定されます。\n\n"
           "説明は以上です。「次へ」をクリックするとseed値特定が開始されます。"));
    break;
  case GUICommon::gameSelection::XD:
    m_lblGameInstructions = new QLabel(tr(
        "\nゲームを開始してメインメニューに入り、「対戦モード」→「シングルバトル」→「さいきょう」の順に選択します。\n"
           "ランダムに生成されたチーム編成が表示されるので、\n表示された「プレイヤーの名前」と「先頭"
           "のポケモンの名前」をチェック欄に正しく記入します。\n"
           "記入が終わったらゲームではとにかくバトルの画面に戻り、次のパスに進みます。\n"
           "（ここで間違って戦闘を開始したり、メインメニューに戻ると正しく機能しなくなります。）\n"
           "以降、何度か同じ作業を繰り返すとseed値が特定されます。\n\n"
           "説明は以上です。「次へ」をクリックするとseed値特定が開始されます。"));
    break;
  default:
    m_lblGameInstructions = new QLabel("");
    break;
  }
  QFont f;
  f.setPointSize(13);
  lblSummary->setFont(f);
  lblSummary->setWordWrap(true);
  m_lblGameInstructions->setFont(f);
  /*
  QLabel* lblNext = new QLabel(
      tr("\nPress \"Next\" once you acknowledged the above instructions to start the seed "
         "finding procedure."));
  lblNext->setWordWrap(true);
  */
  m_lblGameInstructions->setWordWrap(true);

  QVBoxLayout* instructionsLayout = new QVBoxLayout;
  instructionsLayout->addWidget(lblSummary);
  instructionsLayout->addWidget(m_lblGameInstructions);
  //instructionsLayout->addWidget(lblNext);
  instructionsLayout->addStretch();

  QWidget* instructionsWidget = new QWidget(this);
  instructionsWidget->setLayout(instructionsLayout);

  QScrollArea* mainWidget = new QScrollArea(this);
  mainWidget->setWidget(instructionsWidget);
  mainWidget->setWidgetResizable(true);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(mainWidget);

  setLayout(mainLayout);
  adjustSize();
  setFixedHeight(400);
}

int InstructionsPage::nextId() const
{
  return SeedFinderWizard::pageID::SeedFinderPass;
}

EndPage::EndPage(QWidget* parent, const bool sucess, const GUICommon::gameSelection game,
                 const u32 seed)
    : QWizardPage(parent)
{
  setTitle(tr("終了"));

  if (sucess)
  {
    QString additionalNotes("");
    if (game == GUICommon::gameSelection::Colosseum)
      additionalNotes = tr("プレイヤーの名前は、必ず「レオ」などのデフォルトネームを使用してください。");

    QString predictorInstructions = "";
 /*       "Predictions of the starters will appear depending on the amount of frames "
        "between pressing A on starting a new game and pressing A on the trainer name "
        "confirmation.";
    if (game == GUICommon::gameSelection::XD && GaleDarknessRNGSystem::getPalEnabled())
      predictorInstructions =
          "There will only be a single prediction in the prediction list, this will be your "
          "starter no matter how many frames you spend on the naming screen.";
*/

    m_lblResult = new QLabel(
        "Seed値特定が完了しました。\n\n" + QString("現在のseed値は ") +
            QString("%1").arg(seed, 8, 16, QChar('0')).toUpper() +
            QString(
                "です。\n\n" + predictorInstructions +
                "エーフィとブラッキーの予測値が表示されます。\n"
				"ここでのフレーム数とは、「新しく始める」を選択した瞬間（メモリーカードにデータがある場合は「はい」を選択した瞬間）"
				"から、プレイヤーの名前の確認画面で「はい」を選択するまでのフレーム数を表しています。\n"
				"望ましい予測値は緑、それ以外は赤でハイライトされます。（希望する予測値は設定から入力できます。）\n"
				"もし表示された予測値で不満な場合は、リロールもしくはオートリロールをクリックします。（リロールの回数だけチーム生成を行って下さい。）\n"
				"結果に満足な場合は、メインメニューに戻って下さい。\n"
				"画面右側の欄は、マグマラシ、アリゲイツ、ベイリーフのステータスを入力すると個体値、めざめるパワーを自動で特定できます。\n\n"
				+ additionalNotes +"「終了」をクリックすると予測値が表示されます。"),
        this);
  }
  else
  {
    m_lblResult = new QLabel(
        "検索は終了しましたが、現在のSeed値は発見されませんでした。\n"
		"もう一度はじめからやり直してください（ゲームのリセットを含む）。");
  }
  m_lblResult->setWordWrap(true);

  QVBoxLayout* mainlayout = new QVBoxLayout;
  mainlayout->addWidget(m_lblResult);
  mainlayout->addStretch();
  setLayout(mainlayout);
  m_lblResult->adjustSize();
  setFixedHeight(m_lblResult->height() + 200);

  QFont f;
  f.setPointSize(13);
  this->setFont(f);
}

int EndPage::nextId() const
{
  return -1;
}
