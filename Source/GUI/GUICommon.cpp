#include "GUICommon.h"

#include <QObject>

namespace GUICommon
{
QStringList gamesStr =
    QStringList({QObject::tr("ポケモンコロシアム"), QObject::tr("ポケモンＸＤ 闇の旋風ダーク・ルギア")});
QStringList platformsStr = QStringList({"GC", "Wii"});

QStringList naturesStr = QStringList(
    {QObject::tr("がんばりや"),   QObject::tr("さみしがり"), QObject::tr("ゆうかん"),   QObject::tr("いじっぱり"),
     QObject::tr("やんちゃ"), QObject::tr("ずぶとい"),   QObject::tr("すなお"),  QObject::tr("のんき"),
     QObject::tr("わんぱく"),  QObject::tr("のうてんき"),    QObject::tr("おくびょう"),   QObject::tr("せっかち"),
     QObject::tr("まじめ"), QObject::tr("ようき"),  QObject::tr("むじゃき"),   QObject::tr("ひかえめ"),
     QObject::tr("おっとり"),    QObject::tr("れいせい"),  QObject::tr("てれや"), QObject::tr("うっかりや"),
     QObject::tr("おだやか"),    QObject::tr("おとなしい"), QObject::tr("なまいき"),   QObject::tr("しんちょう"),
     QObject::tr("きまぐれ")});

QStringList typesStr = QStringList(
    {QObject::tr("かくとう"), QObject::tr("ひこう"), QObject::tr("どく"), QObject::tr("じめん"),
     QObject::tr("いわ"), QObject::tr("むし"), QObject::tr("ゴースト"), QObject::tr("はがね"),
     QObject::tr("ほのお"), QObject::tr("みず"), QObject::tr("くさ"), QObject::tr("でんき"),
     QObject::tr("エスパー"), QObject::tr("こおり"), QObject::tr("ドラゴン"), QObject::tr("あく")});

QStringList shininessStr =
    QStringList({QObject::tr("希望する"), QObject::tr("希望しない"), QObject::tr("どちらでもよい")});
QStringList genderStr =
    QStringList({QObject::tr("オス"), QObject::tr("メス"), QObject::tr("どちらでもよい")});
}; // namespace GUICommon
