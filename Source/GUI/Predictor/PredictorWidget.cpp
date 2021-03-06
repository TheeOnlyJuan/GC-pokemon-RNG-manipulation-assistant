#include "PredictorWidget.h"

#include <QHeaderView>
#include <QTimer>
#include <QVBoxLayout>

#include "../SPokemonRNG.h"
#include "../Settings/SConfig.h"

PredictorWidget::PredictorWidget(QWidget* parent) : QWidget(parent)
{
  initialiseWidgets();
  makeLayouts();

  connect(m_tblStartersPrediction, &QTableWidget::itemSelectionChanged, this,
          &PredictorWidget::onSelectedPredictionChanged);
}

void PredictorWidget::initialiseWidgets()
{
  QLabel* lblUninitialised = new QLabel(tr("Select a game"));
  lblUninitialised->setAlignment(Qt::AlignmentFlag::AlignCenter);
  m_lblStartersNames.append(lblUninitialised);

  m_tblStartersPrediction = new QTableWidget();
  m_tblStartersPrediction->verticalHeader()->hide();
  m_tblStartersPrediction->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_tblStartersPrediction->setSelectionMode(QAbstractItemView::SingleSelection);
  m_tblStartersPrediction->setSelectionBehavior(QAbstractItemView::SelectRows);
}

void PredictorWidget::filterUnwanted(const bool filterUnwanted)
{
  for (int i = 0; i < m_tblStartersPrediction->rowCount(); i++)
  {
    bool showRow =
        !filterUnwanted || m_tblStartersPrediction->item(i, 2)->background().color().name() ==
                               greenBrush.color().name();

    m_tblStartersPrediction->setRowHidden(i, !showRow);
  }

  if (!filterUnwanted)
  {
    // For some reasons, Qt does some kind of refresh caused by changing which rows is shown which
    // breaks the scrolling, but we can workaround it by simply asking to do it in the near future
    QTimer::singleShot(1, this, &PredictorWidget::scrollToSelectedItem);
  }
  else
  {
    m_tblStartersPrediction->clearSelection();
  }
}

void PredictorWidget::scrollToSelectedItem()
{
  if (m_tblStartersPrediction->selectedItems().size() != 0)
  {
    m_tblStartersPrediction->scrollToItem(m_tblStartersPrediction->selectedItems().first(),
                                          QAbstractItemView::PositionAtCenter);
  }
}

void PredictorWidget::makeLayouts()
{
  m_startersNamesLayout = new QHBoxLayout;
  for (auto label : m_lblStartersNames)
    m_startersNamesLayout->addWidget(label);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addLayout(m_startersNamesLayout);
  mainLayout->addWidget(m_tblStartersPrediction);

  setLayout(mainLayout);
}

void PredictorWidget::resetPredictor(const GUICommon::gameSelection currentGame)
{
  switchGame(currentGame);
}

void PredictorWidget::clearLabels()
{
  m_lblStartersNames.clear();
  QLayoutItem* item;
  while ((item = m_startersNamesLayout->takeAt(0)) != 0)
  {
    delete item->widget();
    delete item;
  }
}

void PredictorWidget::switchGame(const GUICommon::gameSelection game)
{
  clearLabels();
  QLabel* lblSeedFinder = new QLabel(tr("Find your seed for predictions or set it manually"));
  lblSeedFinder->setAlignment(Qt::AlignmentFlag::AlignCenter);

  m_lblStartersNames.append(lblSeedFinder);
  m_startersNamesLayout->addWidget(m_lblStartersNames[0]);
  m_tblStartersPrediction->clear();
  m_tblStartersPrediction->setRowCount(0);
  m_tblHeaderLabels.clear();
  m_tblHeaderLabels.append(tr("Seed"));
  m_tblHeaderLabels.append(tr("Trainer ID"));
  m_tblHeaderLabels.append(tr("Frame (seconds)"));
  for (int i = 0; i < SPokemonRNG::getCurrentSystem()->getNbrStartersPrediction(); i++)
  {
    m_tblHeaderLabels.append(tr("HP IV (stat)"));
    m_tblHeaderLabels.append(tr("Atk IV"));
    m_tblHeaderLabels.append(tr("Def IV"));
    m_tblHeaderLabels.append(tr("Sp. Atk IV"));
    m_tblHeaderLabels.append(tr("Sp. Def IV"));
    m_tblHeaderLabels.append(tr("Speed IV"));
    m_tblHeaderLabels.append(tr("Hidden power"));
    m_tblHeaderLabels.append(tr("Nature"));

    if (game == GUICommon::gameSelection::XD)
    {
      m_tblHeaderLabels.append(tr("Gender"));
      m_tblHeaderLabels.append(tr("Shiny?"));
    }
  }
  m_tblStartersPrediction->setColumnCount(m_tblHeaderLabels.count());
  m_tblStartersPrediction->setHorizontalHeaderLabels(m_tblHeaderLabels);
  m_tblStartersPrediction->resizeColumnsToContents();
}

std::vector<BaseRNGSystem::StartersPrediction> PredictorWidget::getStartersPrediction()
{
  return m_startersPrediction;
}

void PredictorWidget::onSelectedPredictionChanged()
{
  if (m_tblStartersPrediction->currentRow() == -1)
    return;

  BaseRNGSystem::StartersPrediction prediction =
      m_startersPrediction[m_tblStartersPrediction->currentRow()];
  emit selectedPredictionChanged(prediction);
}

bool PredictorWidget::desiredPredictionFound(const GUICommon::gameSelection game)
{
  QVector<GUICommon::starter> startersSettings;
  if (game == GUICommon::gameSelection::Colosseum)
  {
    startersSettings.append(GUICommon::starter::Espeon);
    startersSettings.append(GUICommon::starter::Umbreon);
  }
  else if (game == GUICommon::gameSelection::XD)
  {
    startersSettings.append(GUICommon::starter::Eevee);
  }

  for (int i = 0; i < m_startersPrediction.size(); i++)
  {
    bool passAllFilters = true;
    for (int j = 0; j < m_startersPrediction[i].starters.size(); j++)
    {
      BaseRNGSystem::PokemonProperties starter = m_startersPrediction[i].starters[j];

      if (!(starter.hpIV >= SConfig::getInstance().getMinHpIv(startersSettings[j])))
      {
        passAllFilters = false;
        break;
      }

      if (!(starter.atkIV >= SConfig::getInstance().getMinAtkIv(startersSettings[j])))
      {
        passAllFilters = false;
        break;
      }

      if (!(starter.defIV >= SConfig::getInstance().getMinDefIv(startersSettings[j])))
      {
        passAllFilters = false;
        break;
      }

      if (!(starter.spAtkIV >= SConfig::getInstance().getMinSpAtkIv(startersSettings[j])))
      {
        passAllFilters = false;
        break;
      }

      if (!(starter.spDefIV >= SConfig::getInstance().getMinSpDefIv(startersSettings[j])))
      {
        passAllFilters = false;
        break;
      }

      if (!(starter.speedIV >= SConfig::getInstance().getMinSpeedIv(startersSettings[j])))
      {
        passAllFilters = false;
        break;
      }

      bool enableHiddenPowerTypeFilters =
          SConfig::getInstance().getEnableHiddenPowerTypesFilter(startersSettings[j]);
      int minPowerHiddenPower = SConfig::getInstance().getMinPowerHiddenPower(startersSettings[j]);
      QVector<bool> hiddenPowerTypeFilters =
          SConfig::getInstance().getHiddenPowerTypesFilters(startersSettings[j]);
      if (!((hiddenPowerTypeFilters[starter.hiddenPowerTypeIndex] &&
             starter.hiddenPowerPower >= minPowerHiddenPower) ||
            !enableHiddenPowerTypeFilters))
      {
        passAllFilters = false;
        break;
      }

      bool enableNatureFilters = SConfig::getInstance().getEnableNatureFilter(startersSettings[j]);
      QVector<bool> natureFilters = SConfig::getInstance().getNatureFilters(startersSettings[j]);
      if (!(natureFilters[starter.natureIndex] || !enableNatureFilters))
      {
        passAllFilters = false;
        break;
      }

      if (game == GUICommon::gameSelection::XD)
      {
        int genderIndex = static_cast<int>(SConfig::getInstance().getEeveeGender());
        if (!(starter.genderIndex == genderIndex ||
              genderIndex == static_cast<int>(GUICommon::gender::AnyGender)))
        {
          passAllFilters = false;
          break;
        }

        int shinynessIndex = static_cast<int>(SConfig::getInstance().getEeveeShininess());
        int isShinyInt = starter.isShiny ? static_cast<int>(GUICommon::shininess::Shiny)
                                         : static_cast<int>(GUICommon::shininess::NotShiny);
        if (!(isShinyInt == shinynessIndex ||
              shinynessIndex == static_cast<int>(GUICommon::shininess::AnyShininess)))
        {
          passAllFilters = false;
          break;
        }
      }
    }

    if (passAllFilters)
      return true;
  }
  return false;
}

void PredictorWidget::updateGUI(const GUICommon::gameSelection game)
{
  clearLabels();
  m_tblStartersPrediction->clearSelection();
  std::vector<std::string> names = SPokemonRNG::getCurrentSystem()->getStartersName();
  for (auto name : names)
  {
    QLabel* lblName = new QLabel(QString::fromStdString(name));
    lblName->setAlignment(Qt::AlignmentFlag::AlignCenter);

    m_lblStartersNames.append(lblName);
  }
  m_startersNamesLayout->addStretch();
  for (auto label : m_lblStartersNames)
  {
    m_startersNamesLayout->addSpacing(350);
    m_startersNamesLayout->addWidget(label);
    m_startersNamesLayout->addStretch();
  }
  if (m_lblStartersNames.size() > 1)
    m_startersNamesLayout->addStretch();

  m_tblStartersPrediction->setRowCount(static_cast<int>(m_startersPrediction.size()));
  QVector<GUICommon::starter> startersSettings;
  if (game == GUICommon::gameSelection::Colosseum)
  {
    startersSettings.append(GUICommon::starter::Espeon);
    startersSettings.append(GUICommon::starter::Umbreon);
  }
  else if (game == GUICommon::gameSelection::XD)
  {
    startersSettings.append(GUICommon::starter::Eevee);
  }

  bool desiredStarterFound = false;
  for (int i = 0; i < m_startersPrediction.size(); i++)
  {
    bool passAllFilters = true;
    m_tblStartersPrediction->setItem(
        i, 0,
        new QTableWidgetItem(
            QString("%1").arg(m_startersPrediction[i].startingSeed, 8, 16, QChar('0')).toUpper()));
    m_tblStartersPrediction->setItem(i, 1,
                                     new QTableWidgetItem(QString("%1").arg(
                                         m_startersPrediction[i].trainerId, 5, 10, QChar('0'))));

    if (m_startersPrediction[i].frameNumber == -1)
    {
      m_tblStartersPrediction->setItem(i, 2, new QTableWidgetItem("ALL FRAMES (N/A)"));
    }
    else
    {
      int frameNumberWithDelay =
          m_startersPrediction[i].frameNumber + SConfig::getInstance().getFrameOffset();
      if (i == 0)
      {
        m_tblStartersPrediction->setItem(
            i, 2, new QTableWidgetItem(QString::number(frameNumberWithDelay) + " (frame perfect)"));
      }
      else
      {
        m_tblStartersPrediction->setItem(
            i, 2,
            new QTableWidgetItem(QString::number(frameNumberWithDelay) + " (" +
                                 QString::number(frameNumberWithDelay / 60.0) + ")"));
      }
    }

    int nbrColPerStarter = 8;
    if (game == GUICommon::gameSelection::XD)
      nbrColPerStarter = 10;

    for (int j = 0; j < m_startersPrediction[i].starters.size(); j++)
    {
      BaseRNGSystem::PokemonProperties starter = m_startersPrediction[i].starters[j];

      m_tblStartersPrediction->setItem(
          i, 3 + j * nbrColPerStarter,
          new QTableWidgetItem(QString::number(starter.hpIV) + " (" +
                               QString::number(starter.hpStartingStat) + ")"));
      if (starter.hpIV >= SConfig::getInstance().getMinHpIv(startersSettings[j]))
      {
        m_tblStartersPrediction->item(i, 3 + j * nbrColPerStarter)->setBackground(greenBrush);
      }
      else
      {
        m_tblStartersPrediction->item(i, 3 + j * nbrColPerStarter)->setBackground(redBrush);
        passAllFilters = false;
      }

      m_tblStartersPrediction->setItem(i, 4 + j * nbrColPerStarter,
                                       new QTableWidgetItem(QString::number(starter.atkIV)));
      if (starter.atkIV >= SConfig::getInstance().getMinAtkIv(startersSettings[j]))
      {
        m_tblStartersPrediction->item(i, 4 + j * nbrColPerStarter)->setBackground(greenBrush);
      }
      else
      {
        m_tblStartersPrediction->item(i, 4 + j * nbrColPerStarter)->setBackground(redBrush);
        passAllFilters = false;
      }
      m_tblStartersPrediction->setItem(i, 5 + j * nbrColPerStarter,
                                       new QTableWidgetItem(QString::number(starter.defIV)));
      if (starter.defIV >= SConfig::getInstance().getMinDefIv(startersSettings[j]))
      {
        m_tblStartersPrediction->item(i, 5 + j * nbrColPerStarter)->setBackground(greenBrush);
      }
      else
      {
        m_tblStartersPrediction->item(i, 5 + j * nbrColPerStarter)->setBackground(redBrush);
        passAllFilters = false;
      }
      m_tblStartersPrediction->setItem(i, 6 + j * nbrColPerStarter,
                                       new QTableWidgetItem(QString::number(starter.spAtkIV)));
      if (starter.spAtkIV >= SConfig::getInstance().getMinSpAtkIv(startersSettings[j]))
      {
        m_tblStartersPrediction->item(i, 6 + j * nbrColPerStarter)->setBackground(greenBrush);
      }
      else
      {
        m_tblStartersPrediction->item(i, 6 + j * nbrColPerStarter)->setBackground(redBrush);
        passAllFilters = false;
      }
      m_tblStartersPrediction->setItem(i, 7 + j * nbrColPerStarter,
                                       new QTableWidgetItem(QString::number(starter.spDefIV)));
      if (starter.spDefIV >= SConfig::getInstance().getMinSpDefIv(startersSettings[j]))
      {
        m_tblStartersPrediction->item(i, 7 + j * nbrColPerStarter)->setBackground(greenBrush);
      }
      else
      {
        m_tblStartersPrediction->item(i, 7 + j * nbrColPerStarter)->setBackground(redBrush);
        passAllFilters = false;
      }
      m_tblStartersPrediction->setItem(i, 8 + j * nbrColPerStarter,
                                       new QTableWidgetItem(QString::number(starter.speedIV)));
      if (starter.speedIV >= SConfig::getInstance().getMinSpeedIv(startersSettings[j]))
      {
        m_tblStartersPrediction->item(i, 8 + j * nbrColPerStarter)->setBackground(greenBrush);
      }
      else
      {
        m_tblStartersPrediction->item(i, 8 + j * nbrColPerStarter)->setBackground(redBrush);
        passAllFilters = false;
      }

      m_tblStartersPrediction->setItem(
          i, 9 + j * nbrColPerStarter,
          new QTableWidgetItem(GUICommon::typesStr[starter.hiddenPowerTypeIndex] + " " +
                               QString::number(starter.hiddenPowerPower)));
      bool enableHiddenPowerTypeFilters =
          SConfig::getInstance().getEnableHiddenPowerTypesFilter(startersSettings[j]);
      int minPowerHiddenPower = SConfig::getInstance().getMinPowerHiddenPower(startersSettings[j]);
      QVector<bool> hiddenPowerTypeFilters =
          SConfig::getInstance().getHiddenPowerTypesFilters(startersSettings[j]);
      if ((hiddenPowerTypeFilters[starter.hiddenPowerTypeIndex] &&
           starter.hiddenPowerPower >= minPowerHiddenPower) ||
          !enableHiddenPowerTypeFilters)
      {
        m_tblStartersPrediction->item(i, 9 + j * nbrColPerStarter)->setBackground(greenBrush);
      }
      else
      {
        m_tblStartersPrediction->item(i, 9 + j * nbrColPerStarter)->setBackground(redBrush);
        passAllFilters = false;
      }

      m_tblStartersPrediction->setItem(
          i, 10 + j * nbrColPerStarter,
          new QTableWidgetItem(GUICommon::naturesStr[starter.natureIndex]));
      bool enableNatureFilters = SConfig::getInstance().getEnableNatureFilter(startersSettings[j]);
      QVector<bool> natureFilters = SConfig::getInstance().getNatureFilters(startersSettings[j]);
      if (natureFilters[starter.natureIndex] || !enableNatureFilters)
      {
        m_tblStartersPrediction->item(i, 10 + j * nbrColPerStarter)->setBackground(greenBrush);
      }
      else
      {
        m_tblStartersPrediction->item(i, 10 + j * nbrColPerStarter)->setBackground(redBrush);
        passAllFilters = false;
      }

      if (game == GUICommon::gameSelection::XD)
      {
        m_tblStartersPrediction->setItem(
            i, 11 + j * nbrColPerStarter,
            new QTableWidgetItem(GUICommon::genderStr[starter.genderIndex]));
        int genderIndex = static_cast<int>(SConfig::getInstance().getEeveeGender());
        if (starter.genderIndex == genderIndex ||
            genderIndex == static_cast<int>(GUICommon::gender::AnyGender))
        {
          m_tblStartersPrediction->item(i, 11 + j * nbrColPerStarter)->setBackground(greenBrush);
        }
        else
        {
          m_tblStartersPrediction->item(i, 11 + j * nbrColPerStarter)->setBackground(redBrush);
          passAllFilters = false;
        }
        m_tblStartersPrediction->setItem(i, 12 + j * nbrColPerStarter,
                                         new QTableWidgetItem(tr(starter.isShiny ? "Yes" : "No")));
        int shinynessIndex = static_cast<int>(SConfig::getInstance().getEeveeShininess());
        int isShinyInt = starter.isShiny ? static_cast<int>(GUICommon::shininess::Shiny)
                                         : static_cast<int>(GUICommon::shininess::NotShiny);
        if (isShinyInt == shinynessIndex ||
            shinynessIndex == static_cast<int>(GUICommon::shininess::AnyShininess))
        {
          m_tblStartersPrediction->item(i, 12 + j * nbrColPerStarter)->setBackground(greenBrush);
        }
        else
        {
          m_tblStartersPrediction->item(i, 12 + j * nbrColPerStarter)->setBackground(redBrush);
          passAllFilters = false;
        }
      }
    }
    if (passAllFilters)
    {
      m_tblStartersPrediction->item(i, 2)->setBackground(greenBrush);
      desiredStarterFound = true;
    }
    else
    {
      m_tblStartersPrediction->item(i, 2)->setBackground(redBrush);
    }
  }
  m_tblStartersPrediction->resizeColumnsToContents();
}

void PredictorWidget::setStartersPrediction(
    const std::vector<BaseRNGSystem::StartersPrediction> startersPrediction)
{
  m_startersPrediction = startersPrediction;
}
