#ifndef LIN_FILEIO_H
#define LIN_FILEIO_H

#include <QObject>
#include <QString>
#include <QVector>
#include "lin_structs.h"

// ═══════════════════════════════════════════════════════════════════════════
// LINFileIO — static methods for loading/saving LIN frame logs and LDF files
// ═══════════════════════════════════════════════════════════════════════════

class LINFileIO : public QObject
{
    Q_OBJECT

public:
    LINFileIO();

    // ── LDF (LIN Description File) ──────────────────────────────────────
    static bool loadLDF(QString filename, LDFDatabase &db);
    static bool saveLDF(QString filename, const LDFDatabase &db);

    // ── LIN log formats ──────────────────────────────────────────────────
    static bool loadLINLogGeneric(QString filename, QVector<LINFrame> &frames);
    static bool saveLINLogCSV(QString filename, const QVector<LINFrame> &frames);
    static bool saveLINLogNative(QString filename, const QVector<LINFrame> &frames);

    // ── Detection ────────────────────────────────────────────────────────
    static bool isLDFFile(QString filename);
    static bool isLINLogFile(QString filename);

private:
    static void skipWhitespace(const QString &line, int &pos);
    static QString readToken(const QString &line, int &pos);
    static QString readQuotedString(const QString &line, int &pos);
    static int readInteger(const QString &line, int &pos);
    static double readDouble(const QString &line, int &pos);
};

#endif // LIN_FILEIO_H
