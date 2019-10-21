#include "GeneralTab.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>

#include <thread>

GeneralTab::GeneralTab(QWidget* parent) : QWidget(parent)
{
  QLabel* lblDetectedThreadDescription = new QLabel(tr("Amount of threads detected on this CPU: "));
  QLabel* lblDetectedThread = new QLabel(QString::number(std::thread::hardware_concurrency()));

  QLabel* lblThreadLimit = new QLabel(tr("Amount of threads to use in this program: "));
  m_cmbThreadLimit = new QComboBox(this);
  m_cmbThreadLimit->addItem(tr("All threads"));
  for (unsigned int i = 1; i < std::thread::hardware_concurrency(); i++)
    m_cmbThreadLimit->addItem(QString::number(i));
  m_cmbThreadLimit->setCurrentIndex(0);

  QLabel* lblThreadLimitDescription =
      new QLabel(tr("This setting allows you to limit the number of threads the program can use "
                    "for the algorithms of the seed finder and the precalculation file generation. "
                    "This will make them slower, but will allow critical CPU intensive "
                    "applications such as a live streaming software to function properly."));
  lblThreadLimitDescription->setWordWrap(true);

  QFormLayout* threadLimitLayout = new QFormLayout;
  threadLimitLayout->setLabelAlignment(Qt::AlignRight);
  threadLimitLayout->addRow(lblDetectedThreadDescription, lblDetectedThread);
  threadLimitLayout->addRow(lblThreadLimit, m_cmbThreadLimit);

  QVBoxLayout* cpuSettingsLayout = new QVBoxLayout;
  cpuSettingsLayout->addLayout(threadLimitLayout);
  cpuSettingsLayout->addSpacing(10);
  cpuSettingsLayout->addWidget(lblThreadLimitDescription);

  QGroupBox* gbCPUSettings = new QGroupBox(tr("CPU settings"));
  gbCPUSettings->setLayout(cpuSettingsLayout);

  QLabel* lblPredictionsTime =
      new QLabel(tr("Amount of time to generate predictions (in seconds): "));
  m_spbPredictionsTime = new QSpinBox();
  m_spbPredictionsTime->setMinimum(0);
  m_spbPredictionsTime->setValue(10);
  // Let's put a crazy time like 10 hours cause Qt puts 99 at default for no reaosns
  m_spbPredictionsTime->setMaximum(36000);
  m_spbPredictionsTime->setMaximumWidth(150);

  QLabel* lblFrameOffset =
      new QLabel(tr("Offset to apply to the amount of frames of the predictions: "));
  m_spbFrameOffset = new QSpinBox();
  m_spbFrameOffset->setMinimum(-100);
  m_spbFrameOffset->setValue(0);
  m_spbFrameOffset->setMaximum(100);
  m_spbFrameOffset->setMaximumWidth(150);

  QLabel* lblMaxAutoReroll =
      new QLabel(tr("Number of maximum reroll to perform when auto rerolling: "));
  m_spbMaxAutoReroll = new QSpinBox();
  m_spbMaxAutoReroll->setMinimum(1);
  m_spbMaxAutoReroll->setMaximum(10000);
  m_spbMaxAutoReroll->setValue(100);
  m_spbMaxAutoReroll->setMaximumWidth(150);

    QLabel* lblStartingTeddyFrame =
      new QLabel(tr("From Eevee seed Teddy search. Frame to start at: "));
  m_spbTeddyStartingFrame = new QSpinBox();
  m_spbTeddyStartingFrame->setMinimum(1);
  m_spbTeddyStartingFrame->setMaximum(1000000);
  m_spbTeddyStartingFrame->setValue(680000);
  m_spbTeddyStartingFrame->setMaximumWidth(150);

      QLabel* lblTeddySearchWindow =
      new QLabel(tr("From Eevee seed Teddy search. Number of frames to check: "));
  m_spbTeddySearchFrames = new QSpinBox();
  m_spbTeddySearchFrames->setMinimum(1);
  m_spbTeddySearchFrames->setMaximum(500000);
  m_spbTeddySearchFrames->setValue(63000);
  m_spbTeddySearchFrames->setMaximumWidth(150);

        QLabel* lblTeddyNewGameStartFrame =
      new QLabel(tr("New Game Teddy search. Frame to start at: "));
  m_spbTeddyNewGameStartingFrame = new QSpinBox();
  m_spbTeddyNewGameStartingFrame->setMinimum(1);
  m_spbTeddyNewGameStartingFrame->setMaximum(1000000);
  m_spbTeddyNewGameStartingFrame->setValue(473000);
  m_spbTeddyNewGameStartingFrame->setMaximumWidth(150);

        QLabel* lblTeddyNewGameSearchWindow =
      new QLabel(tr("New Game Teddy search. Number of frames to check: "));
  m_spbTeddyNewGameSearchFrames = new QSpinBox();
  m_spbTeddyNewGameSearchFrames->setMinimum(1);
  m_spbTeddyNewGameSearchFrames->setMaximum(500000);
  m_spbTeddyNewGameSearchFrames->setValue(7000);
  m_spbTeddyNewGameSearchFrames->setMaximumWidth(150);

  QFormLayout* predictionTimeLayout = new QFormLayout;
  predictionTimeLayout->setLabelAlignment(Qt::AlignRight);
  predictionTimeLayout->addRow(lblPredictionsTime, m_spbPredictionsTime);
  predictionTimeLayout->addRow(lblFrameOffset, m_spbFrameOffset);
  predictionTimeLayout->addRow(lblMaxAutoReroll, m_spbMaxAutoReroll);
  predictionTimeLayout->addRow(lblStartingTeddyFrame, m_spbTeddyStartingFrame);
  predictionTimeLayout->addRow(lblTeddySearchWindow, m_spbTeddySearchFrames);
  predictionTimeLayout->addRow(lblTeddyNewGameStartFrame, m_spbTeddyNewGameStartingFrame);
  predictionTimeLayout->addRow(lblTeddyNewGameSearchWindow, m_spbTeddyNewGameSearchFrames);

  QGroupBox* gbPredictor = new QGroupBox(tr("Starters predictor"));
  gbPredictor->setLayout(predictionTimeLayout);

  m_chkRestorePreviousWindowGeometry = new QCheckBox(
      "Restore the previous main window's position and size at the start of the program");
  m_chkRestorePreviousWindowGeometry->setChecked(false);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(gbCPUSettings);
  mainLayout->addWidget(gbPredictor);
  mainLayout->addWidget(m_chkRestorePreviousWindowGeometry);
  mainLayout->addStretch();

  setLayout(mainLayout);
}

int GeneralTab::getPredictionTime() const
{
  return m_spbPredictionsTime->value();
}

int GeneralTab::getFrameOffset() const
{
  return m_spbFrameOffset->value();
}

int GeneralTab::getThreadLimit() const
{
  return m_cmbThreadLimit->currentIndex();
}

int GeneralTab::getMaxAutoReroll() const
{
  return m_spbMaxAutoReroll->value();
}

bool GeneralTab::getRestorePreviousWindowGeometry() const
{
  return m_chkRestorePreviousWindowGeometry->isChecked();
}

void GeneralTab::setPredictionTime(const int predictionTime)
{
  m_spbPredictionsTime->setValue(predictionTime);
}

void GeneralTab::setThreadLimit(const int threadLimit)
{
  m_cmbThreadLimit->setCurrentIndex(threadLimit);
}

void GeneralTab::setFrameOffset(const int frameDelay)
{
  m_spbFrameOffset->setValue(frameDelay);
}

void GeneralTab::setMaxAutoReroll(const int maxAutoReroll)
{
  m_spbMaxAutoReroll->setValue(maxAutoReroll);
}

void GeneralTab::setRestorePreviousWindowGeometry(const bool restoreGeometry)
{
  m_chkRestorePreviousWindowGeometry->setChecked(restoreGeometry);
}

void GeneralTab::setTeddyStartingFrame(const int frame)
{
 m_spbTeddyStartingFrame->setValue(frame);
}
void GeneralTab::setTeddySearchFrames(const int frames)
{
  m_spbTeddySearchFrames->setValue(frames);
}
void GeneralTab::setTeddyNewGameStartingFrame(const int frame)
{
  m_spbTeddyNewGameStartingFrame->setValue(frame);
}
void GeneralTab::setTeddyNewGameSearchFrames(const int frames)
{
  m_spbTeddyNewGameSearchFrames->setValue(frames);
}

int GeneralTab::getTeddyStartingFrame()
{
  return m_spbTeddyStartingFrame->value();
}
int GeneralTab::getTeddySearchFrames()
{
  return m_spbTeddySearchFrames->value();
}
int GeneralTab::getTeddyNewGameStartingFrame()
{
  return m_spbTeddyNewGameStartingFrame->value();
}
int GeneralTab::getTeddyNewGameSearchFrames()
{
  return m_spbTeddyNewGameSearchFrames->value();
}
