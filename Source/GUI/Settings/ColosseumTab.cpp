﻿#include "ColosseumTab.h"

#include <QVBoxLayout>

ColosseumTab::ColosseumTab(QWidget* parent) : QWidget(parent)
{
  m_starterTabs = new QTabWidget();

  m_predictorFiltersEspeon = new CommonPredictorFiltersWidget(this);
  m_starterTabs->addTab(m_predictorFiltersEspeon, tr("エーフィ"));

  m_predictorFiltersUmbreon = new CommonPredictorFiltersWidget(this);
  m_starterTabs->addTab(m_predictorFiltersUmbreon, tr("ブラッキー"));

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(m_starterTabs);

  setLayout(mainLayout);
}

ColosseumTab::~ColosseumTab()
{
  delete m_predictorFiltersUmbreon;
  delete m_predictorFiltersEspeon;
}

CommonPredictorFiltersWidget* ColosseumTab::getUmbreonFiltersWidget() const
{
  return m_predictorFiltersUmbreon;
}

CommonPredictorFiltersWidget* ColosseumTab::getEspeonFiltersWidget() const
{
  return m_predictorFiltersEspeon;
}
