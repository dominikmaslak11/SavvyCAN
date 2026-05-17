#include "futuristic_theme.h"
#include <QPalette>
#include <QColor>

namespace FuturisticTheme {

// ── Color tokens ──────────────────────────────────────────────────────────
// Deep space background palette
static const char *const kBgPrimary   = "#0a0a1a";  // main bg
static const char *const kBgSecondary = "#12122a";  // surface / alternate
static const char *const kBgTertiary  = "#1a1a35";  // elevated
static const char *const kBgHover     = "#222244";  // hover state

// Neon accent palette
static const char *const kAccentCyan   = "#00e5ff";  // primary accent
static const char *const kAccentPurple = "#7c4dff";  // secondary accent
static const char *const kAccentPink   = "#ff4081";  // danger / warning

// Text
static const char *const kTextPrimary   = "#e0e0f0";
static const char *const kTextSecondary = "#8899aa";
static const char *const kTextDisabled  = "#555566";

// Borders
static const char *const kBorderSubtle  = "#1e1e3a";
static const char *const kBorderAccent  = "#00e5ff";

// Scrollbar
static const char *const kScrollBg    = "#0f0f22";
static const char *const kScrollHandle = "#2a2a50";
static const char *const kScrollHover  = "#3a3a66";

// ── Stylesheet ────────────────────────────────────────────────────────────

QString darkStyleSheet()
{
    return QStringLiteral(R"(
/* ═══════════════════════════════════════════════════════════════════════════
   SAVVYCAN · FUTURISTIC DARK THEME · Qt6
   ═══════════════════════════════════════════════════════════════════════════ */

/* ── Global ─────────────────────────────────────────────────────────────── */
* {
    color: %1;
    selection-background-color: %2;
    selection-color: #000000;
}

QMainWindow {
    background-color: %3;
}

QMainWindow::separator {
    background-color: %4;
    width: 1px;
    height: 1px;
}

/* ── Menu Bar ───────────────────────────────────────────────────────────── */
QMenuBar {
    background-color: %5;
    border-bottom: 1px solid %4;
    padding: 2px 0px;
    font-size: 13px;
}

QMenuBar::item {
    padding: 6px 14px;
    background: transparent;
    border-radius: 4px;
    margin: 2px 2px;
}

QMenuBar::item:selected {
    background-color: rgba(0, 229, 255, 0.12);
    color: %2;
}

QMenuBar::item:pressed {
    background-color: rgba(0, 229, 255, 0.20);
}

/* ── Menus ──────────────────────────────────────────────────────────────── */
QMenu {
    background-color: %5;
    border: 1px solid %4;
    border-radius: 6px;
    padding: 6px 0px;
}

QMenu::item {
    padding: 8px 32px 8px 18px;
    margin: 1px 6px;
    border-radius: 4px;
}

QMenu::item:selected {
    background-color: rgba(0, 229, 255, 0.15);
    color: %2;
}

QMenu::separator {
    height: 1px;
    background: %4;
    margin: 4px 10px;
}

QMenu::indicator {
    width: 14px;
    height: 14px;
    margin-left: 6px;
}

/* ── Push Buttons ───────────────────────────────────────────────────────── */
QPushButton {
    background-color: %6;
    color: %1;
    border: 1px solid %4;
    border-radius: 6px;
    padding: 7px 18px;
    font-size: 13px;
    font-weight: 500;
    min-height: 24px;
}

QPushButton:hover {
    background-color: %7;
    border-color: %2;
}

QPushButton:pressed {
    background-color: rgba(0, 229, 255, 0.18);
}

QPushButton:checked {
    background-color: rgba(0, 229, 255, 0.15);
    border-color: %2;
    color: %2;
}

QPushButton:disabled {
    background-color: %5;
    color: %8;
    border-color: %4;
}

QPushButton#btnCaptureToggle {
    font-weight: bold;
    letter-spacing: 1px;
}

QPushButton#btnCaptureToggle:checked {
    background-color: rgba(255, 64, 129, 0.15);
    border-color: %9;
    color: %9;
}

/* ── Check Boxes ────────────────────────────────────────────────────────── */
QCheckBox {
    spacing: 8px;
    font-size: 13px;
    color: %1;
}

QCheckBox::indicator {
    width: 18px;
    height: 18px;
    border: 2px solid %4;
    border-radius: 4px;
    background-color: %3;
}

QCheckBox::indicator:hover {
    border-color: %2;
}

QCheckBox::indicator:checked {
    background-color: %2;
    border-color: %2;
}

QCheckBox::indicator:checked:hover {
    background-color: rgba(0, 229, 255, 0.85);
}

/* ── Radio Buttons ──────────────────────────────────────────────────────── */
QRadioButton {
    spacing: 8px;
    font-size: 13px;
    color: %1;
}

QRadioButton::indicator {
    width: 18px;
    height: 18px;
    border: 2px solid %4;
    border-radius: 9px;
    background-color: %3;
}

QRadioButton::indicator:hover {
    border-color: %2;
}

QRadioButton::indicator:checked {
    background-color: %2;
    border-color: %2;
}

/* ── Line Edits ─────────────────────────────────────────────────────────── */
QLineEdit {
    background-color: %3;
    color: %1;
    border: 1px solid %4;
    border-radius: 6px;
    padding: 6px 10px;
    font-size: 13px;
    selection-background-color: %2;
    selection-color: #000000;
}

QLineEdit:focus {
    border-color: %2;
}

QLineEdit:hover:!focus {
    border-color: %8;
}

QLineEdit:disabled {
    background-color: %5;
    color: %8;
}

/* ── Spin Boxes ─────────────────────────────────────────────────────────── */
QSpinBox, QDoubleSpinBox {
    background-color: %3;
    color: %1;
    border: 1px solid %4;
    border-radius: 6px;
    padding: 5px 8px;
    font-size: 13px;
}

QSpinBox:focus, QDoubleSpinBox:focus {
    border-color: %2;
}

QSpinBox::up-button, QDoubleSpinBox::up-button {
    subcontrol-origin: border;
    subcontrol-position: top right;
    width: 20px;
    border-left: 1px solid %4;
    border-bottom: 1px solid %4;
    border-top-right-radius: 5px;
    background-color: %6;
}

QSpinBox::down-button, QDoubleSpinBox::down-button {
    subcontrol-origin: border;
    subcontrol-position: bottom right;
    width: 20px;
    border-left: 1px solid %4;
    border-bottom-right-radius: 5px;
    background-color: %6;
}

QSpinBox::up-arrow, QDoubleSpinBox::up-arrow {
    image: none;
    border-left: 5px solid transparent;
    border-right: 5px solid transparent;
    border-bottom: 5px solid %1;
}

QSpinBox::down-arrow, QDoubleSpinBox::down-arrow {
    image: none;
    border-left: 5px solid transparent;
    border-right: 5px solid transparent;
    border-top: 5px solid %1;
}

/* ── Combo Boxes ────────────────────────────────────────────────────────── */
QComboBox {
    background-color: %3;
    color: %1;
    border: 1px solid %4;
    border-radius: 6px;
    padding: 6px 10px;
    font-size: 13px;
    min-width: 80px;
}

QComboBox:hover {
    border-color: %8;
}

QComboBox:focus {
    border-color: %2;
}

QComboBox::drop-down {
    subcontrol-origin: padding;
    subcontrol-position: top right;
    width: 26px;
    border-left: 1px solid %4;
    border-top-right-radius: 5px;
    border-bottom-right-radius: 5px;
    background-color: %6;
}

QComboBox QAbstractItemView {
    background-color: %5;
    border: 1px solid %4;
    border-radius: 4px;
    selection-background-color: rgba(0, 229, 255, 0.18);
    selection-color: %2;
    outline: none;
}

/* ── Table Views ────────────────────────────────────────────────────────── */
QTableView, QTableWidget {
    background-color: %3;
    alternate-background-color: %10;
    border: 1px solid %4;
    border-radius: 6px;
    gridline-color: %4;
    font-size: 13px;
    selection-background-color: rgba(0, 229, 255, 0.18);
    selection-color: %1;
}

QTableView::item, QTableWidget::item {
    padding: 4px 8px;
    border-bottom: 1px solid transparent;
}

QTableView::item:selected, QTableWidget::item:selected {
    background-color: rgba(0, 229, 255, 0.18);
    color: %2;
}

QHeaderView::section {
    background-color: %5;
    color: %2;
    padding: 6px 8px;
    border: none;
    border-bottom: 2px solid %2;
    border-right: 1px solid %4;
    font-weight: bold;
    font-size: 12px;
    text-transform: uppercase;
    letter-spacing: 1px;
}

QHeaderView::section:hover {
    background-color: %7;
}

/* ── List Widgets ───────────────────────────────────────────────────────── */
QListWidget {
    background-color: %3;
    border: 1px solid %4;
    border-radius: 6px;
    font-size: 13px;
    outline: none;
}

QListWidget::item {
    padding: 4px 8px;
    border-bottom: 1px solid transparent;
    border-radius: 3px;
}

QListWidget::item:selected {
    background-color: rgba(0, 229, 255, 0.18);
    color: %2;
}

QListWidget::item:hover {
    background-color: rgba(0, 229, 255, 0.08);
}

/* ── Scroll Bars ────────────────────────────────────────────────────────── */
QScrollBar:horizontal {
    border: none;
    background: %11;
    height: 8px;
    margin: 0px;
    border-radius: 4px;
}

QScrollBar::handle:horizontal {
    background: %12;
    min-width: 30px;
    border-radius: 4px;
}

QScrollBar::handle:horizontal:hover {
    background: %13;
}

QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
    width: 0px;
}

QScrollBar:vertical {
    border: none;
    background: %11;
    width: 8px;
    margin: 0px;
    border-radius: 4px;
}

QScrollBar::handle:vertical {
    background: %12;
    min-height: 30px;
    border-radius: 4px;
}

QScrollBar::handle:vertical:hover {
    background: %13;
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0px;
}

QScrollBar::add-page, QScrollBar::sub-page {
    background: none;
}

/* ── Status Bar ─────────────────────────────────────────────────────────── */
QStatusBar {
    background-color: %5;
    border-top: 1px solid %4;
    color: %8;
    font-size: 12px;
    padding: 2px 6px;
}

QStatusBar::item {
    border: none;
}

QStatusBar QLabel {
    color: %8;
    padding: 0px 6px;
}

/* ── Labels ─────────────────────────────────────────────────────────────── */
QLabel {
    color: %1;
    font-size: 13px;
}

QLabel#lbFPS, QLabel#lbNumFrames {
    color: %2;
    font-size: 16px;
    font-weight: bold;
    font-family: "Consolas", "Courier New", monospace;
}

QLabel#lblContMsg {
    color: %9;
    font-size: 13px;
    font-weight: bold;
    letter-spacing: 2px;
}

/* ── Tool Tips ──────────────────────────────────────────────────────────── */
QToolTip {
    background-color: %5;
    color: %1;
    border: 1px solid %2;
    border-radius: 4px;
    padding: 6px 10px;
    font-size: 12px;
}

/* ── Group Boxes ────────────────────────────────────────────────────────── */
QGroupBox {
    border: 1px solid %4;
    border-radius: 8px;
    margin-top: 12px;
    padding: 16px 12px 12px 12px;
    font-size: 13px;
    font-weight: bold;
    color: %2;
}

QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top left;
    padding: 0px 8px;
    color: %2;
}

/* ── Tab Widget ─────────────────────────────────────────────────────────── */
QTabWidget::pane {
    border: 1px solid %4;
    border-radius: 6px;
    background-color: %3;
    top: -1px;
}

QTabBar::tab {
    background-color: %5;
    color: %8;
    border: 1px solid %4;
    border-bottom: none;
    border-top-left-radius: 6px;
    border-top-right-radius: 6px;
    padding: 8px 18px;
    margin-right: 3px;
    font-size: 13px;
}

QTabBar::tab:selected {
    background-color: %3;
    color: %2;
    border-bottom: 2px solid %2;
}

QTabBar::tab:hover:!selected {
    background-color: %7;
    color: %1;
}

/* ── Progress Bar ───────────────────────────────────────────────────────── */
QProgressBar {
    background-color: %3;
    border: 1px solid %4;
    border-radius: 6px;
    text-align: center;
    color: %1;
    font-size: 12px;
    height: 18px;
}

QProgressBar::chunk {
    background-color: %2;
    border-radius: 5px;
}

/* ── Dialogs ────────────────────────────────────────────────────────────── */
QDialog {
    background-color: %3;
}

/* ── Splitter ───────────────────────────────────────────────────────────── */
QSplitter::handle {
    background-color: %4;
    margin: 1px;
}

QSplitter::handle:hover {
    background-color: %2;
}

/* ── Tool Bar ───────────────────────────────────────────────────────────── */
QToolBar {
    background-color: %5;
    border-bottom: 1px solid %4;
    padding: 3px;
    spacing: 4px;
}

QToolButton {
    background-color: transparent;
    border: 1px solid transparent;
    border-radius: 4px;
    padding: 5px 10px;
    color: %1;
}

QToolButton:hover {
    background-color: %7;
    border-color: %4;
}

QToolButton:pressed {
    background-color: rgba(0, 229, 255, 0.18);
}

/* ── Sliders ────────────────────────────────────────────────────────────── */
QSlider::groove:horizontal {
    border: none;
    height: 6px;
    background: %4;
    border-radius: 3px;
}

QSlider::handle:horizontal {
    background: %2;
    border: 2px solid %2;
    width: 16px;
    height: 16px;
    margin: -5px 0;
    border-radius: 9px;
}

QSlider::handle:horizontal:hover {
    background: %1;
    border-color: %1;
}

QSlider::sub-page:horizontal {
    background: %2;
    border-radius: 3px;
}

/* ── Text Edit / Plain Text Edit ────────────────────────────────────────── */
QTextEdit, QPlainTextEdit {
    background-color: %3;
    color: %1;
    border: 1px solid %4;
    border-radius: 6px;
    padding: 6px;
    font-size: 13px;
    selection-background-color: %2;
    selection-color: #000000;
}

QTextEdit:focus, QPlainTextEdit:focus {
    border-color: %2;
}

/* ── Dock Widget ────────────────────────────────────────────────────────── */
QDockWidget {
    color: %1;
    titlebar-close-icon: none;
    titlebar-normal-icon: none;
}

QDockWidget::title {
    background-color: %5;
    padding: 6px;
    border-bottom: 1px solid %4;
    text-align: left;
}

/* ── Line (horizontal/vertical separators) ──────────────────────────────── */
QFrame[frameShape="4"],  /* HLine */
QFrame[frameShape="5"]   /* VLine */
{
    color: %4;
}

/* ═══════════════════════════════════════════════════════════════════════════
   END OF STYLESHEET
   ═══════════════════════════════════════════════════════════════════════════ */
)")
        .arg(kTextPrimary)      // %1
        .arg(kAccentCyan)       // %2
        .arg(kBgPrimary)        // %3
        .arg(kBorderSubtle)     // %4
        .arg(kBgSecondary)      // %5
        .arg(kBgTertiary)       // %6
        .arg(kBgHover)          // %7
        .arg(kTextDisabled)     // %8
        .arg(kAccentPink)       // %9
        .arg(kBgSecondary)      // %10  alternate bg for table rows
        .arg(kScrollBg)         // %11
        .arg(kScrollHandle)     // %12
        .arg(kScrollHover);     // %13
}

// ── Dark QPalette ─────────────────────────────────────────────────────────

QPalette darkPalette()
{
    QPalette pal;

    pal.setColor(QPalette::Window,          QColor(kBgPrimary));
    pal.setColor(QPalette::WindowText,      QColor(kTextPrimary));
    pal.setColor(QPalette::Base,            QColor(kBgPrimary));
    pal.setColor(QPalette::AlternateBase,   QColor(kBgSecondary));
    pal.setColor(QPalette::ToolTipBase,     QColor(kBgSecondary));
    pal.setColor(QPalette::ToolTipText,     QColor(kTextPrimary));
    pal.setColor(QPalette::Text,            QColor(kTextPrimary));
    pal.setColor(QPalette::Button,          QColor(kBgTertiary));
    pal.setColor(QPalette::ButtonText,      QColor(kTextPrimary));
    pal.setColor(QPalette::BrightText,      QColor(kAccentCyan));
    pal.setColor(QPalette::Link,            QColor(kAccentCyan));
    pal.setColor(QPalette::LinkVisited,     QColor(kAccentPurple));
    pal.setColor(QPalette::Highlight,       QColor(kAccentCyan));
    pal.setColor(QPalette::HighlightedText, QColor("#000000"));
    pal.setColor(QPalette::PlaceholderText, QColor(kTextDisabled));

    // Disabled group
    pal.setColor(QPalette::Disabled, QPalette::WindowText, QColor(kTextDisabled));
    pal.setColor(QPalette::Disabled, QPalette::Text,       QColor(kTextDisabled));
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(kTextDisabled));
    pal.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(kTextDisabled));

    return pal;
}

// ── Light Stylesheet ──────────────────────────────────────────────────────

QString lightStyleSheet()
{
    return QStringLiteral(R"(
* { color: #222222; selection-background-color: #0066cc; selection-color: #ffffff; }
QMainWindow { background-color: #f5f5f5; }
QMenuBar { background-color: #e8e8e8; border-bottom: 1px solid #ccc; }
QMenuBar::item:selected { background-color: #0066cc; color: #fff; }
QMenu { background-color: #ffffff; border: 1px solid #ccc; border-radius: 6px; }
QMenu::item:selected { background-color: #0066cc; color: #fff; }
QPushButton { background-color: #e0e0e0; color: #222; border: 1px solid #bbb; border-radius: 4px; padding: 6px 14px; }
QPushButton:hover { background-color: #d0d0d0; border-color: #0066cc; }
QPushButton:pressed { background-color: #0066cc; color: #fff; }
QPushButton:disabled { background-color: #eee; color: #999; }
QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox { background: #fff; color: #222; border: 1px solid #bbb; border-radius: 4px; padding: 4px 8px; }
QLineEdit:focus, QSpinBox:focus, QComboBox:focus { border-color: #0066cc; }
QTableView, QTableWidget { background: #fff; alternate-background-color: #f0f0f0; border: 1px solid #ccc; gridline-color: #ddd; }
QHeaderView::section { background: #e8e8e8; color: #222; border-bottom: 2px solid #0066cc; padding: 4px 8px; font-weight: bold; }
QScrollBar:vertical { background: #f0f0f0; width: 10px; }
QScrollBar::handle:vertical { background: #c0c0c0; border-radius: 5px; }
QScrollBar::handle:vertical:hover { background: #0066cc; }
QStatusBar { background: #e8e8e8; border-top: 1px solid #ccc; }
QGroupBox { border: 1px solid #ccc; border-radius: 6px; margin-top: 10px; padding-top: 16px; color: #0066cc; font-weight: bold; }
QTabWidget::pane { border: 1px solid #ccc; }
QTabBar::tab { background: #e8e8e8; color: #666; border: 1px solid #ccc; padding: 6px 14px; }
QTabBar::tab:selected { background: #fff; color: #0066cc; border-bottom: 2px solid #0066cc; }
QCheckBox, QRadioButton { color: #222; spacing: 6px; }
QTextEdit, QPlainTextEdit { background: #fff; border: 1px solid #ccc; border-radius: 4px; }
QDialog { background: #f5f5f5; }
)");
}

QPalette lightPalette()
{
    QPalette pal;
    pal.setColor(QPalette::Window,          QColor("#f5f5f5"));
    pal.setColor(QPalette::WindowText,      QColor("#222222"));
    pal.setColor(QPalette::Base,            QColor("#ffffff"));
    pal.setColor(QPalette::AlternateBase,   QColor("#f0f0f0"));
    pal.setColor(QPalette::ToolTipBase,     QColor("#ffffff"));
    pal.setColor(QPalette::ToolTipText,     QColor("#222222"));
    pal.setColor(QPalette::Text,            QColor("#222222"));
    pal.setColor(QPalette::Button,          QColor("#e0e0e0"));
    pal.setColor(QPalette::ButtonText,      QColor("#222222"));
    pal.setColor(QPalette::BrightText,      QColor("#0066cc"));
    pal.setColor(QPalette::Link,            QColor("#0066cc"));
    pal.setColor(QPalette::Highlight,       QColor("#0066cc"));
    pal.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    pal.setColor(QPalette::PlaceholderText, QColor("#999999"));
    pal.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#999999"));
    pal.setColor(QPalette::Disabled, QPalette::Text,       QColor("#999999"));
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#999999"));
    return pal;
}

// ── High Contrast Stylesheet ──────────────────────────────────────────────

QString highContrastStyleSheet()
{
    return QStringLiteral(R"(
* { color: #ffffff; selection-background-color: #ffff00; selection-color: #000000; }
QMainWindow { background-color: #000000; }
QMenuBar { background-color: #111111; border-bottom: 2px solid #ffff00; color: #ffffff; }
QMenuBar::item:selected { background-color: #ffff00; color: #000000; }
QMenu { background-color: #111111; border: 2px solid #ffff00; }
QMenu::item:selected { background-color: #ffff00; color: #000000; }
QPushButton { background-color: #222222; color: #ffffff; border: 2px solid #ffff00; border-radius: 4px; padding: 8px 16px; font-weight: bold; }
QPushButton:hover { background-color: #ffff00; color: #000000; }
QPushButton:pressed { background-color: #ffcc00; color: #000000; }
QPushButton:disabled { background-color: #333; color: #666; border-color: #666; }
QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox { background: #000000; color: #ffff00; border: 2px solid #ffff00; border-radius: 4px; padding: 6px 10px; font-weight: bold; }
QLineEdit:focus, QSpinBox:focus, QComboBox:focus { border-color: #ffffff; background: #111111; }
QTableView, QTableWidget { background: #000000; alternate-background-color: #0a0a0a; border: 2px solid #ffff00; gridline-color: #333; color: #ffffff; }
QTableView::item:selected, QTableWidget::item:selected { background-color: #ffff00; color: #000000; }
QHeaderView::section { background: #222222; color: #ffff00; border-bottom: 3px solid #ffff00; padding: 6px 8px; font-weight: bold; text-transform: uppercase; }
QScrollBar:vertical { background: #000; width: 16px; border: 1px solid #ffff00; }
QScrollBar::handle:vertical { background: #ffff00; min-height: 30px; }
QScrollBar:horizontal { background: #000; height: 16px; border: 1px solid #ffff00; }
QScrollBar::handle:horizontal { background: #ffff00; min-width: 30px; }
QStatusBar { background: #111111; border-top: 2px solid #ffff00; color: #ffff00; font-weight: bold; }
QGroupBox { border: 2px solid #ffff00; border-radius: 6px; margin-top: 12px; padding-top: 18px; color: #ffff00; font-weight: bold; }
QTabWidget::pane { border: 2px solid #ffff00; background: #000; }
QTabBar::tab { background: #222; color: #aaa; border: 2px solid #555; padding: 8px 16px; font-weight: bold; }
QTabBar::tab:selected { background: #000; color: #ffff00; border-color: #ffff00; }
QCheckBox, QRadioButton { color: #ffffff; spacing: 8px; }
QCheckBox::indicator, QRadioButton::indicator { border: 2px solid #ffff00; background: #000; }
QCheckBox::indicator:checked, QRadioButton::indicator:checked { background: #ffff00; }
QLabel { color: #ffffff; }
QDialog { background: #000000; }
QTextEdit, QPlainTextEdit { background: #000; color: #ffff00; border: 2px solid #ffff00; }
)");
}

QPalette highContrastPalette()
{
    QPalette pal;
    pal.setColor(QPalette::Window,          QColor("#000000"));
    pal.setColor(QPalette::WindowText,      QColor("#ffffff"));
    pal.setColor(QPalette::Base,            QColor("#000000"));
    pal.setColor(QPalette::AlternateBase,   QColor("#0a0a0a"));
    pal.setColor(QPalette::Text,            QColor("#ffffff"));
    pal.setColor(QPalette::Button,          QColor("#222222"));
    pal.setColor(QPalette::ButtonText,      QColor("#ffffff"));
    pal.setColor(QPalette::BrightText,      QColor("#ffff00"));
    pal.setColor(QPalette::Link,            QColor("#ffff00"));
    pal.setColor(QPalette::Highlight,       QColor("#ffff00"));
    pal.setColor(QPalette::HighlightedText, QColor("#000000"));
    pal.setColor(QPalette::PlaceholderText, QColor("#888888"));
    pal.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#666666"));
    pal.setColor(QPalette::Disabled, QPalette::Text,       QColor("#666666"));
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#666666"));
    pal.setColor(QPalette::ToolTipBase,     QColor("#111111"));
    pal.setColor(QPalette::ToolTipText,     QColor("#ffff00"));
    return pal;
}

} // namespace FuturisticTheme
