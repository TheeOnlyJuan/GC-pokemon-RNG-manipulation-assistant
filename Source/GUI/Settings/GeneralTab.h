#pragma once

#include <QWidget>

#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>

#include "../GUICommon.h"

class GeneralTab : public QWidget
{
public:
  GeneralTab(QWidget* parent = nullptr);

  int getThreadLimit() const;
  int getPredictionTime() const;
  int getFrameOffset() const;
  int getMaxAutoReroll() const;
  bool getRestorePreviousWindowGeometry() const;
  int getTeddyStartingFrame();
  int getTeddySearchFrames();
  int getTeddyNewGameStartingFrame();
  int getTeddyNewGameSearchFrames();

  void setTeddyStartingFrame(const int frame);
  void setTeddySearchFrames(const int frames);
  void setTeddyNewGameStartingFrame(const int frame);
  void setTeddyNewGameSearchFrames(const int frame);

  void setThreadLimit(const int threadLimit);
  void setPredictionTime(const int predictionTime);
  void setFrameOffset(const int frameDelay);
  void setMaxAutoReroll(const int maxAutoReroll);
  void setRestorePreviousWindowGeometry(const bool restoreGeometry);

private:
  QComboBox* m_cmbThreadLimit;
  QSpinBox* m_spbPredictionsTime;
  QSpinBox* m_spbFrameOffset;
  QSpinBox* m_spbMaxAutoReroll;
  QSpinBox* m_spbTeddyStartingFrame;
  QSpinBox* m_spbTeddySearchFrames;
  QSpinBox* m_spbTeddyNewGameStartingFrame;
  QSpinBox* m_spbTeddyNewGameSearchFrames;
  QCheckBox* m_chkRestorePreviousWindowGeometry;
};
