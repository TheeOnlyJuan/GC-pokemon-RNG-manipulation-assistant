﻿#include "DlgSettings.h"

#include <QFile>
#include <QMessageBox>
#include <QVBoxLayout>

#include "../../PokemonRNGSystem/Colosseum/ColosseumRNGSystem.h"
#include "../../PokemonRNGSystem/XD/GaleDarknessRNGSystem.h"
#include "../GUICommon.h"
#include "SConfig.h"

DlgSettings::DlgSettings(QWidget* parent) : QDialog(parent)
{
  m_tabWidget = new QTabWidget(this);
  m_generalTab = new GeneralTab(this);
  m_tabWidget->addTab(m_generalTab, tr("一般"));
  m_coloTab = new ColosseumTab(this);
  m_tabWidget->addTab(
      m_coloTab, "プレディクター (" + GUICommon::gamesStr[GUICommon::gameSelection::Colosseum] + ")");
  m_xdTab = new GaleDarknessTab(this);
  m_tabWidget->addTab(m_xdTab,
                      "プレディクター (" + GUICommon::gamesStr[GUICommon::gameSelection::XD] + ")");

  m_buttonsDlg = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  connect(m_buttonsDlg, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(m_buttonsDlg, &QDialogButtonBox::clicked, this, [=](QAbstractButton* button) {
    if (m_buttonsDlg->buttonRole(button) == QDialogButtonBox::AcceptRole)
    {
      saveSettings();
      QDialog::accept();
    }
  });

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(m_tabWidget);
  mainLayout->addWidget(m_buttonsDlg);
  setLayout(mainLayout);

  setWindowTitle(tr("設定"));
  QFont f;
  f.setPointSize(9);
  this->setFont(f);

  loadSettings();
}

DlgSettings::~DlgSettings()
{
  delete m_buttonsDlg;
  delete m_generalTab;
  delete m_coloTab;
  delete m_xdTab;
}

void DlgSettings::loadSettings()
{
  m_generalTab->setPredictionTime(SConfig::getInstance().getPredictionTime());
  m_generalTab->setFrameOffset(SConfig::getInstance().getFrameOffset());
  m_generalTab->setThreadLimit(SConfig::getInstance().getThreadLimit());
  m_generalTab->setMaxAutoReroll(SConfig::getInstance().getMaxAutoReroll());
  m_generalTab->setRestorePreviousWindowGeometry(
      SConfig::getInstance().getRestorePreviousWindowGeometry());

  loadUmbreonSettings();
  loadEspeonSettings();
  loadEeveeSettings();
  //m_xdTab->setPalVersionEnabled(SConfig::getInstance().getXDPalVersionEnabled());
}

void DlgSettings::saveSettings() const
{
  SConfig::getInstance().setPredictionTime(m_generalTab->getPredictionTime());
  SConfig::getInstance().setFrameOffset(m_generalTab->getFrameOffset());
  SConfig::getInstance().setThreadLimit(m_generalTab->getThreadLimit());
  SConfig::getInstance().setMaxAutoReroll(m_generalTab->getMaxAutoReroll());
  SConfig::getInstance().setRestorePreviousWindowGeometry(
      m_generalTab->getRestorePreviousWindowGeometry());

  saveUmbreonSettings();
  saveEspeonSettings();
  saveEeveeSettings();
  //SConfig::getInstance().setXDPalVersionEnabled(m_xdTab->getPalVersionEnabled());
  //GaleDarknessRNGSystem::setPalEnabled(m_xdTab->getPalVersionEnabled());
}

void DlgSettings::loadUmbreonSettings()
{
  m_coloTab->getUmbreonFiltersWidget()->setMinHpIv(
      SConfig::getInstance().getMinHpIv(GUICommon::starter::Umbreon));
  m_coloTab->getUmbreonFiltersWidget()->setMinAtkIv(
      SConfig::getInstance().getMinAtkIv(GUICommon::starter::Umbreon));
  m_coloTab->getUmbreonFiltersWidget()->setMinDefIv(
      SConfig::getInstance().getMinDefIv(GUICommon::starter::Umbreon));
  m_coloTab->getUmbreonFiltersWidget()->setMinSpAtkIv(
      SConfig::getInstance().getMinSpAtkIv(GUICommon::starter::Umbreon));
  m_coloTab->getUmbreonFiltersWidget()->setMinSpDefIv(
      SConfig::getInstance().getMinSpDefIv(GUICommon::starter::Umbreon));
  m_coloTab->getUmbreonFiltersWidget()->setMinSpeedIv(
      SConfig::getInstance().getMinSpeedIv(GUICommon::starter::Umbreon));
  m_coloTab->getUmbreonFiltersWidget()->setMinPowerHiddenPower(
      SConfig::getInstance().getMinPowerHiddenPower(GUICommon::starter::Umbreon));
  m_coloTab->getUmbreonFiltersWidget()->setEnableNatureFilter(
      SConfig::getInstance().getEnableNatureFilter(GUICommon::starter::Umbreon));
  m_coloTab->getUmbreonFiltersWidget()->setNatureFilters(
      SConfig::getInstance().getNatureFilters(GUICommon::starter::Umbreon));
  m_coloTab->getUmbreonFiltersWidget()->setEnableHiddenPowerTypesFilter(
      SConfig::getInstance().getEnableHiddenPowerTypesFilter(GUICommon::starter::Umbreon));
  m_coloTab->getUmbreonFiltersWidget()->setHiddenPowerTypesFilters(
      SConfig::getInstance().getHiddenPowerTypesFilters(GUICommon::starter::Umbreon));
}

void DlgSettings::loadEspeonSettings()
{
  m_coloTab->getEspeonFiltersWidget()->setMinHpIv(
      SConfig::getInstance().getMinHpIv(GUICommon::starter::Espeon));
  m_coloTab->getEspeonFiltersWidget()->setMinAtkIv(
      SConfig::getInstance().getMinAtkIv(GUICommon::starter::Espeon));
  m_coloTab->getEspeonFiltersWidget()->setMinDefIv(
      SConfig::getInstance().getMinDefIv(GUICommon::starter::Espeon));
  m_coloTab->getEspeonFiltersWidget()->setMinSpAtkIv(
      SConfig::getInstance().getMinSpAtkIv(GUICommon::starter::Espeon));
  m_coloTab->getEspeonFiltersWidget()->setMinSpDefIv(
      SConfig::getInstance().getMinSpDefIv(GUICommon::starter::Espeon));
  m_coloTab->getEspeonFiltersWidget()->setMinSpeedIv(
      SConfig::getInstance().getMinSpeedIv(GUICommon::starter::Espeon));
  m_coloTab->getEspeonFiltersWidget()->setMinPowerHiddenPower(
      SConfig::getInstance().getMinPowerHiddenPower(GUICommon::starter::Espeon));
  m_coloTab->getEspeonFiltersWidget()->setEnableNatureFilter(
      SConfig::getInstance().getEnableNatureFilter(GUICommon::starter::Espeon));
  m_coloTab->getEspeonFiltersWidget()->setNatureFilters(
      SConfig::getInstance().getNatureFilters(GUICommon::starter::Espeon));
  m_coloTab->getEspeonFiltersWidget()->setEnableHiddenPowerTypesFilter(
      SConfig::getInstance().getEnableHiddenPowerTypesFilter(GUICommon::starter::Espeon));
  m_coloTab->getEspeonFiltersWidget()->setHiddenPowerTypesFilters(
      SConfig::getInstance().getHiddenPowerTypesFilters(GUICommon::starter::Espeon));
}

void DlgSettings::loadEeveeSettings()
{
  m_xdTab->getEeveeFiltersWidget()->setMinHpIv(
      SConfig::getInstance().getMinHpIv(GUICommon::starter::Eevee));
  m_xdTab->getEeveeFiltersWidget()->setMinAtkIv(
      SConfig::getInstance().getMinAtkIv(GUICommon::starter::Eevee));
  m_xdTab->getEeveeFiltersWidget()->setMinDefIv(
      SConfig::getInstance().getMinDefIv(GUICommon::starter::Eevee));
  m_xdTab->getEeveeFiltersWidget()->setMinSpAtkIv(
      SConfig::getInstance().getMinSpAtkIv(GUICommon::starter::Eevee));
  m_xdTab->getEeveeFiltersWidget()->setMinSpDefIv(
      SConfig::getInstance().getMinSpDefIv(GUICommon::starter::Eevee));
  m_xdTab->getEeveeFiltersWidget()->setMinSpeedIv(
      SConfig::getInstance().getMinSpeedIv(GUICommon::starter::Eevee));
  m_xdTab->getEeveeFiltersWidget()->setMinPowerHiddenPower(
      SConfig::getInstance().getMinPowerHiddenPower(GUICommon::starter::Eevee));
  m_xdTab->getEeveeFiltersWidget()->setEnableNatureFilter(
      SConfig::getInstance().getEnableNatureFilter(GUICommon::starter::Eevee));
  m_xdTab->getEeveeFiltersWidget()->setNatureFilters(
      SConfig::getInstance().getNatureFilters(GUICommon::starter::Eevee));
  m_xdTab->getEeveeFiltersWidget()->setEnableHiddenPowerTypesFilter(
      SConfig::getInstance().getEnableHiddenPowerTypesFilter(GUICommon::starter::Eevee));
  m_xdTab->getEeveeFiltersWidget()->setHiddenPowerTypesFilters(
      SConfig::getInstance().getHiddenPowerTypesFilters(GUICommon::starter::Eevee));
  m_xdTab->setShininess(SConfig::getInstance().getEeveeShininess());
  m_xdTab->setGender(SConfig::getInstance().getEeveeGender());
}

void DlgSettings::saveUmbreonSettings() const
{
  SConfig::getInstance().setMinHpIv(GUICommon::starter::Umbreon,
                                    m_coloTab->getUmbreonFiltersWidget()->getMinHpIv());
  SConfig::getInstance().setMinAtkIv(GUICommon::starter::Umbreon,
                                     m_coloTab->getUmbreonFiltersWidget()->getMinAtkIv());
  SConfig::getInstance().setMinDefIv(GUICommon::starter::Umbreon,
                                     m_coloTab->getUmbreonFiltersWidget()->getMinDefIv());
  SConfig::getInstance().setMinSpAtkIv(GUICommon::starter::Umbreon,
                                       m_coloTab->getUmbreonFiltersWidget()->getMinSpAtkIv());
  SConfig::getInstance().setMinSpDefIv(GUICommon::starter::Umbreon,
                                       m_coloTab->getUmbreonFiltersWidget()->getMinSpDefIv());
  SConfig::getInstance().setMinSpeedIv(GUICommon::starter::Umbreon,
                                       m_coloTab->getUmbreonFiltersWidget()->getMinSpeedIv());
  SConfig::getInstance().setMinPowerHiddenPower(
      GUICommon::starter::Umbreon, m_coloTab->getUmbreonFiltersWidget()->getMinPowerHiddenPower());
  SConfig::getInstance().setEnableNatureFilter(
      GUICommon::starter::Umbreon, m_coloTab->getUmbreonFiltersWidget()->getEnableNatureFilter());
  SConfig::getInstance().setEnableHiddenPowerTypesFilter(
      GUICommon::starter::Umbreon,
      m_coloTab->getUmbreonFiltersWidget()->getEnableHiddenPowerTypesFilter());
  SConfig::getInstance().setNatureFilters(GUICommon::starter::Umbreon,
                                          m_coloTab->getUmbreonFiltersWidget()->getNatureFilters());
  SConfig::getInstance().setHiddenPowerTypesFilters(
      GUICommon::starter::Umbreon,
      m_coloTab->getUmbreonFiltersWidget()->getHiddenPowerTypesFilters());
}

void DlgSettings::saveEspeonSettings() const
{
  SConfig::getInstance().setMinHpIv(GUICommon::starter::Espeon,
                                    m_coloTab->getEspeonFiltersWidget()->getMinHpIv());
  SConfig::getInstance().setMinAtkIv(GUICommon::starter::Espeon,
                                     m_coloTab->getEspeonFiltersWidget()->getMinAtkIv());
  SConfig::getInstance().setMinDefIv(GUICommon::starter::Espeon,
                                     m_coloTab->getEspeonFiltersWidget()->getMinDefIv());
  SConfig::getInstance().setMinSpAtkIv(GUICommon::starter::Espeon,
                                       m_coloTab->getEspeonFiltersWidget()->getMinSpAtkIv());
  SConfig::getInstance().setMinSpDefIv(GUICommon::starter::Espeon,
                                       m_coloTab->getEspeonFiltersWidget()->getMinSpDefIv());
  SConfig::getInstance().setMinSpeedIv(GUICommon::starter::Espeon,
                                       m_coloTab->getEspeonFiltersWidget()->getMinSpeedIv());
  SConfig::getInstance().setMinPowerHiddenPower(
      GUICommon::starter::Espeon, m_coloTab->getEspeonFiltersWidget()->getMinPowerHiddenPower());
  SConfig::getInstance().setEnableNatureFilter(
      GUICommon::starter::Espeon, m_coloTab->getEspeonFiltersWidget()->getEnableNatureFilter());
  SConfig::getInstance().setEnableHiddenPowerTypesFilter(
      GUICommon::starter::Espeon,
      m_coloTab->getEspeonFiltersWidget()->getEnableHiddenPowerTypesFilter());
  SConfig::getInstance().setNatureFilters(GUICommon::starter::Espeon,
                                          m_coloTab->getEspeonFiltersWidget()->getNatureFilters());
  SConfig::getInstance().setHiddenPowerTypesFilters(
      GUICommon::starter::Espeon,
      m_coloTab->getEspeonFiltersWidget()->getHiddenPowerTypesFilters());
}

void DlgSettings::saveEeveeSettings() const
{
  SConfig::getInstance().setMinHpIv(GUICommon::starter::Eevee,
                                    m_xdTab->getEeveeFiltersWidget()->getMinHpIv());
  SConfig::getInstance().setMinAtkIv(GUICommon::starter::Eevee,
                                     m_xdTab->getEeveeFiltersWidget()->getMinAtkIv());
  SConfig::getInstance().setMinDefIv(GUICommon::starter::Eevee,
                                     m_xdTab->getEeveeFiltersWidget()->getMinDefIv());
  SConfig::getInstance().setMinSpAtkIv(GUICommon::starter::Eevee,
                                       m_xdTab->getEeveeFiltersWidget()->getMinSpAtkIv());
  SConfig::getInstance().setMinSpDefIv(GUICommon::starter::Eevee,
                                       m_xdTab->getEeveeFiltersWidget()->getMinSpDefIv());
  SConfig::getInstance().setMinSpeedIv(GUICommon::starter::Eevee,
                                       m_xdTab->getEeveeFiltersWidget()->getMinSpeedIv());
  SConfig::getInstance().setMinPowerHiddenPower(
      GUICommon::starter::Eevee, m_xdTab->getEeveeFiltersWidget()->getMinPowerHiddenPower());
  SConfig::getInstance().setEnableNatureFilter(
      GUICommon::starter::Eevee, m_xdTab->getEeveeFiltersWidget()->getEnableNatureFilter());
  SConfig::getInstance().setEnableHiddenPowerTypesFilter(
      GUICommon::starter::Eevee,
      m_xdTab->getEeveeFiltersWidget()->getEnableHiddenPowerTypesFilter());
  SConfig::getInstance().setNatureFilters(GUICommon::starter::Eevee,
                                          m_xdTab->getEeveeFiltersWidget()->getNatureFilters());
  SConfig::getInstance().setHiddenPowerTypesFilters(
      GUICommon::starter::Eevee, m_xdTab->getEeveeFiltersWidget()->getHiddenPowerTypesFilters());
  SConfig::getInstance().setEeveeShininess(m_xdTab->getShininess());
  SConfig::getInstance().setEeveeGender(m_xdTab->getGender());
}
