#include "BaseRNGSystem.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

std::string BaseRNGSystem::getPrecalcFilenameForSettings(bool useWii, int rtcErrorMarginSeconds)
{
  std::stringstream ss;
  ss << (useWii ? "Wii" : "GC");
  ss << '-';
  ss << rtcErrorMarginSeconds;
  return ss.str();
}

BaseRNGSystem::seedRange BaseRNGSystem::getRangeForSettings(bool useWii, int rtcErrorMarginSeconds)
{
  seedRange range;
  int ticksPerSecond = useWii ? Common::ticksPerSecondWii : Common::ticksPerSecondGC;
  if (rtcErrorMarginSeconds == 0)
  {
    range.min = 0;
    range.max = 0x100000000;
  }
  else
  {
    range.min = useWii ? minRTCTicksToBootWii : minRTCTicksToBootGC;
    range.max = range.min + ticksPerSecond * rtcErrorMarginSeconds;
  }
  return range;
}

size_t BaseRNGSystem::getPracalcFileSize(bool useWii, int rtcErrorMarginSeconds)
{
  seedRange range = getRangeForSettings(useWii, rtcErrorMarginSeconds);
  return (range.max - range.min) * sizeof(u16);
}

void BaseRNGSystem::precalculateNbrRollsBeforeTeamGeneration(bool useWii, int rtcErrorMarginSeconds)
{
  std::ofstream precalcFile(getPrecalcFilenameForSettings(useWii, rtcErrorMarginSeconds),
                            std::ios::binary | std::ios::out);
  seedRange range = getRangeForSettings(useWii, rtcErrorMarginSeconds);
  for (s64 i = range.min; i < range.max; i++)
  {
    u32 seed = 0;
    u16 counter = 0;
    seed = rollRNGToBattleMenu(static_cast<u32>(i), &counter);
    u16* ptrCounter = &counter;
    precalcFile.write(reinterpret_cast<const char*>(ptrCounter), sizeof(u16));
  }
  precalcFile.close();
}

void BaseRNGSystem::seedFinder(std::vector<int> criteria, std::vector<u32>& seeds, bool useWii,
                               int rtcErrorMarginSeconds, bool usePrecalc, bool firstPass)
{
  std::vector<u32> newSeeds;
  seedRange range;
  range.max = seeds.size();
  if (firstPass)
    range = getRangeForSettings(useWii, rtcErrorMarginSeconds);
  std::ifstream precalcFile(getPrecalcFilenameForSettings(useWii, rtcErrorMarginSeconds),
                            std::ios::binary | std::ios::in);
  usePrecalc = usePrecalc && precalcFile.good();
  u16* precalc = nullptr;
  if (usePrecalc)
  {
    precalc = new u16[range.max - range.min];
    precalcFile.read(reinterpret_cast<char*>(precalc), (range.max - range.min) * sizeof(u16));
  }
  std::cout << "Simulating " << range.max - range.min << " seeds "
            << (usePrecalc ? "with" : "without") << " precalculation using "
            << std::thread::hardware_concurrency() << " thread(s)...\n";
  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
#pragma omp parallel for
  for (s64 i = range.min; i < range.max; i++)
  {
    u32 seed = 0;
    if (firstPass)
    {
      if (usePrecalc)
      {
        u16 nbrRngCalls = 0;
        std::memcpy(&nbrRngCalls, precalc + (i - range.min), sizeof(u16));
        seed = LCGn(static_cast<u32>(i), nbrRngCalls);
      }
      else
      {
        seed = rollRNGToBattleMenu(static_cast<u32>(i));
      }
    }
    else
    {
      seed = seeds[i];
    }
    if (generateBattleTeam(seed, criteria))
#pragma omp critical(addSeed)
      newSeeds.push_back(seed);
  }
  std::swap(newSeeds, seeds);
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::cout << "done in " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count()
            << " seconds" << std::endl;
  // As the number of calls differs depending on the starting seed, it may happen that some seeds
  // may end up being the same as another one, this gets rid of the duplicates so the program can
  // only have one unique result at the end
  std::sort(seeds.begin(), seeds.end());
  auto last = std::unique(seeds.begin(), seeds.end());
  seeds.erase(last, seeds.end());

  std::cout << seeds.size() << " result(s)" << std::endl;
  std::cout << std::endl;
  delete[] precalc;
}

std::vector<BaseRNGSystem::startersPrediction>
BaseRNGSystem::predictStartersForNbrSeconds(u32 seed, int nbrSeconds)
{
  std::vector<startersPrediction> predictionsResult;
  seed = rollRNGNamingScreenInit(seed);

  for (int i = getMinFramesAmountNamingScreen();
       i < getMinFramesAmountNamingScreen() + nbrSeconds * pollingRateNamingScreenPerSec; i++)
  {
    seed = rollRNGNamingScreenNext(seed);
    BaseRNGSystem::startersPrediction prediction = generateStarterPokemons(seed);
    prediction.frameNumber = i;
    predictionsResult.push_back(prediction);
  }
  return predictionsResult;
}