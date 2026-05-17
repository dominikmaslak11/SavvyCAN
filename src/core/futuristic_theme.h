#ifndef FUTURISTIC_THEME_H
#define FUTURISTIC_THEME_H

#include <QString>
#include <QPalette>

namespace FuturisticTheme {

/// Full application stylesheet – dark neon cyber theme for SavvyCAN
QString darkStyleSheet();

/// Light stylesheet variant
QString lightStyleSheet();

/// Dark QPalette matching the stylesheet
QPalette darkPalette();

/// Light QPalette
QPalette lightPalette();

/// High contrast stylesheet
QString highContrastStyleSheet();

/// High contrast QPalette
QPalette highContrastPalette();

} // namespace FuturisticTheme

#endif // FUTURISTIC_THEME_H
