#include "audio_player.h"
#include "home.h"
#include "partial_text.h"
#include "practice.h"
#include "transcript_library.h"
#include "ui_state.h"
#include "version.h"

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QEvent>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QGuiApplication>
#include <QIcon>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QPlainTextEdit>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QRandomGenerator>
#include <QScrollArea>
#include <QTimer>
#include <QTime>
#include <QTextCursor>
#include <QListWidget>
#include <QLocale>
#include <QActionGroup>
#include <QShortcut>
#include <QSlider>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStyle>
#include <QStyleFactory>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <functional>

namespace {

// The worker sits beside the interface, whatever directory the application was
// installed into. The suffix is named rather than left to the platform: Windows
// would append it when starting the process, but not when the path is used for
// anything else, and one spelling everywhere is easier to reason about.
QString worker_program() {
#if defined(Q_OS_WIN)
    return QCoreApplication::applicationDirPath() + "/eco_escrito_cli.exe";
#else
    return QCoreApplication::applicationDirPath() + "/eco_escrito_cli";
#endif
}

// One light theme, set on the application so dialogs match the window. The
// palette is fixed rather than taken from the desktop: these colours assume a
// light background, and a dark system palette would put pale text on white.
QString application_style() {
    return QStringLiteral(R"(
QMainWindow, QWidget#page { background: #f5f5f7; }
QWidget#header { background: #ffffff; border-bottom: 1px solid #e5e5ea; }
QLabel#brand { font-size: 15px; font-weight: 700; color: #1d1d1f; }
QLabel#offline { color: #15803d; background: #ecfdf3; border-radius: 10px; padding: 3px 10px;
                 font-weight: 600; }
QToolButton[nav="true"] { border: none; border-radius: 8px; padding: 7px 14px; color: #424245;
                          font-weight: 600; background: transparent; }
QToolButton[nav="true"]:hover { background: #ececf0; color: #1d1d1f; }
QToolButton[nav="true"]:checked { background: #e8f1ff; color: #0062cc; }
QToolButton[nav="true"]:disabled { color: #c7c7cc; }
QToolButton[nav="true"]::menu-indicator { image: none; width: 0; }
QLabel#heroTitle { font-size: 26px; font-weight: 700; color: #1d1d1f; }
QLabel#pageTitle { font-size: 18px; font-weight: 700; color: #1d1d1f; }
QLabel#sectionTitle { font-size: 14px; font-weight: 700; color: #1d1d1f; }
QLabel#muted { color: #6e6e73; }
QLabel#sentence { font-size: 22px; font-weight: 600; color: #1d1d1f; }
QLabel#score { font-size: 16px; font-weight: 700; color: #1d1d1f; }
QLabel#resultWords { font-size: 20px; }
QLabel#badge { background: #e8f1ff; color: #0062cc; border-radius: 14px; font-weight: 700; }
QFrame#card { background: #ffffff; border: 1px solid #e5e5ea; border-radius: 14px; }
QFrame#step { background: #ffffff; border: 1px solid #e5e5ea; border-radius: 12px; }
QGroupBox { background: #ffffff; border: 1px solid #e5e5ea; border-radius: 12px; }
QPushButton { background: #ffffff; color: #1d1d1f; border: 1px solid #d2d2d7; border-radius: 8px;
              padding: 6px 14px; }
QPushButton:hover { background: #f2f2f5; }
QPushButton:pressed { background: #e8e8ed; }
QPushButton:checked { background: #e8f1ff; border-color: #99c7ff; color: #0062cc; }
QPushButton:disabled { color: #aeaeb2; background: #f5f5f7; border-color: #e5e5ea; }
QPushButton#mic { background: #007aff; color: #ffffff; border: none; border-radius: 28px;
                  font-size: 16px; font-weight: 700; padding: 0 28px; }
QPushButton#mic:hover { background: #0062cc; }
QPushButton#mic[recording="true"] { background: #dc2626; }
QPushButton#mic[recording="true"]:hover { background: #b91c1c; }
QPushButton#mic:disabled { background: #80bdff; color: #f0f7ff; }
QPushButton#primary { background: #007aff; color: #ffffff; border: none; font-weight: 700; }
QPushButton#primary:hover { background: #0062cc; }
QPushButton#primary:disabled { background: #b3d7ff; color: #f0f7ff; }
QPushButton#link { border: none; background: transparent; color: #007aff; font-weight: 600; }
QPushButton#link:hover { text-decoration: underline; }
QPlainTextEdit, QListWidget { background: #ffffff; border: 1px solid #d2d2d7; border-radius: 10px; }
QPlainTextEdit:focus { border-color: #007aff; }
QListWidget::item { padding: 8px 10px; }
QListWidget::item:selected { background: #e8f1ff; color: #0062cc; }
QStatusBar { background: #ffffff; border-top: 1px solid #e5e5ea; color: #6e6e73; }
QStatusBar QLabel { color: #6e6e73; }
QProgressBar { background: #e5e5ea; border: none; border-radius: 4px; }
QProgressBar::chunk { background: #007aff; border-radius: 4px; }
QScrollArea { background: transparent; border: none; }
QToolButton#brand { border: none; background: transparent; font-size: 15px; font-weight: 700; color: #1d1d1f;
                    padding: 4px 6px; }
QToolButton#brand:hover { color: #000000; }
QLabel#appIcon { background: #ffffff; border: 1px solid #e5e5ea; border-radius: 18px; }
QLabel#eyebrow { font-size: 15px; font-weight: 600; color: #6e6e73; }
QLabel#homeTitle { font-size: 30px; font-weight: 700; color: #1d1d1f; }
QLabel#homeLead { font-size: 15px; color: #6e6e73; }
QLabel#groupTitle { font-size: 13px; font-weight: 600; color: #6e6e73; }
QPushButton#tile { background: #ffffff; border: 1px solid #e5e5ea; border-radius: 14px; text-align: left;
                   padding: 0; }
QPushButton#tile:hover { border-color: #d2d2d7; background: #fcfcfd; }
QPushButton#tile:pressed { background: #f2f2f5; }
QPushButton#tile:focus { border: 2px solid #80bdff; }
QPushButton#tile:disabled { background: #fafafc; }
QPushButton#tile QLabel, QPushButton#recentRow QLabel { background: transparent; }
QLabel#tileTitle { font-size: 15px; font-weight: 600; color: #1d1d1f; }
QLabel#tileBody { color: #6e6e73; }
QLabel#tileMeta { font-size: 12px; color: #8e8e93; }
QLabel#chevron { font-size: 20px; color: #c7c7cc; }
QFrame#group { background: #ffffff; border: 1px solid #e5e5ea; border-radius: 12px; }
QFrame#hairline { background: #e5e5ea; border: none; max-height: 1px; min-height: 1px; }
QPushButton#recentRow { background: transparent; border: none; border-radius: 0; text-align: left; padding: 0; }
QPushButton#recentRow:hover { background: #f5f5f7; }
QPushButton#recentRow:focus { background: #e8f1ff; }
QLabel#recentText { color: #1d1d1f; }
QLabel#recentDate { color: #8e8e93; }
QLabel#footnote { font-size: 12px; color: #8e8e93; }
)");
}

QPalette application_palette() {
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#f5f5f7"));
    palette.setColor(QPalette::WindowText, QColor("#1d1d1f"));
    palette.setColor(QPalette::Base, QColor("#ffffff"));
    palette.setColor(QPalette::AlternateBase, QColor("#fafafc"));
    palette.setColor(QPalette::Text, QColor("#1d1d1f"));
    palette.setColor(QPalette::Button, QColor("#ffffff"));
    palette.setColor(QPalette::ButtonText, QColor("#1d1d1f"));
    palette.setColor(QPalette::Highlight, QColor("#007aff"));
    palette.setColor(QPalette::HighlightedText, QColor("#ffffff"));
    palette.setColor(QPalette::ToolTipBase, QColor("#ffffff"));
    palette.setColor(QPalette::ToolTipText, QColor("#1d1d1f"));
    palette.setColor(QPalette::PlaceholderText, QColor("#a1a1a6"));
    palette.setColor(QPalette::Link, QColor("#007aff"));
    for (const QPalette::ColorRole role : {QPalette::WindowText, QPalette::Text, QPalette::ButtonText}) {
        palette.setColor(QPalette::Disabled, role, QColor("#aeaeb2"));
    }
    return palette;
}

// The glyphs drawn on the landing page's feature tiles.
enum class FeatureGlyph { Practice, Microphone, Document, Waveform };

// A feature's icon in the manner of the macOS system icons: a white glyph on a
// rounded square filled with a soft vertical gradient. Drawn rather than loaded,
// so it is crisp at any scale and adds no files to ship.
QPixmap feature_icon(FeatureGlyph glyph, const QColor& top, const QColor& bottom, int size) {
    const qreal ratio = qApp->devicePixelRatio();
    QPixmap pixmap(static_cast<int>(size * ratio), static_cast<int>(size * ratio));
    pixmap.setDevicePixelRatio(ratio);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    const qreal s = size;
    QLinearGradient fill(0, 0, 0, s);
    fill.setColorAt(0, top);
    fill.setColorAt(1, bottom);
    painter.setPen(Qt::NoPen);
    painter.setBrush(fill);
    painter.drawRoundedRect(QRectF(0, 0, s, s), s * 0.24, s * 0.24);

    QPen pen(Qt::white, s * 0.075, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    switch (glyph) {
        case FeatureGlyph::Practice: {
            // A speech bubble with a tick: say it, and see that it came through.
            QPainterPath bubble;
            bubble.addRoundedRect(QRectF(s * 0.22, s * 0.24, s * 0.56, s * 0.40), s * 0.12, s * 0.12);
            painter.drawPath(bubble);
            painter.drawLine(QPointF(s * 0.36, s * 0.64), QPointF(s * 0.32, s * 0.76));
            painter.drawLine(QPointF(s * 0.32, s * 0.76), QPointF(s * 0.46, s * 0.64));
            painter.drawPolyline(QPolygonF({QPointF(s * 0.39, s * 0.44), QPointF(s * 0.47, s * 0.51),
                                            QPointF(s * 0.61, s * 0.37)}));
            break;
        }
        case FeatureGlyph::Microphone: {
            painter.setBrush(Qt::white);
            painter.drawRoundedRect(QRectF(s * 0.40, s * 0.20, s * 0.20, s * 0.36), s * 0.10, s * 0.10);
            painter.setBrush(Qt::NoBrush);
            painter.drawArc(QRectF(s * 0.31, s * 0.30, s * 0.38, s * 0.36), 200 * 16, 140 * 16);
            painter.drawLine(QPointF(s * 0.50, s * 0.66), QPointF(s * 0.50, s * 0.78));
            painter.drawLine(QPointF(s * 0.40, s * 0.78), QPointF(s * 0.60, s * 0.78));
            break;
        }
        case FeatureGlyph::Document: {
            painter.drawRoundedRect(QRectF(s * 0.28, s * 0.20, s * 0.44, s * 0.60), s * 0.07, s * 0.07);
            for (const qreal y : {0.36, 0.48, 0.60}) {
                const qreal end = y == 0.60 ? 0.52 : 0.60;
                painter.drawLine(QPointF(s * 0.40, s * y), QPointF(s * end, s * y));
            }
            break;
        }
        case FeatureGlyph::Waveform: {
            const qreal heights[] = {0.16, 0.34, 0.50, 0.30, 0.20};
            for (int i = 0; i < 5; ++i) {
                const qreal x = s * (0.26 + 0.12 * i);
                const qreal half = s * heights[i] / 2;
                painter.drawLine(QPointF(x, s * 0.5 - half), QPointF(x, s * 0.5 + half));
            }
            break;
        }
    }
    return pixmap;
}

// Colours for each mark, shared by the result and its legend.
QString mark_colour(WordMark mark) {
    switch (mark) {
        case WordMark::Correct: return "#15803d";
        case WordMark::Unclear: return "#b45309";
        case WordMark::Substituted: return "#b91c1c";
        case WordMark::Missed: return "#9ca3af";
        case WordMark::Extra: return "#7c3aed";
    }
    return "#1f2430";
}

QString quoted(const std::string& word) {
    return QString("“%1”").arg(QString::fromStdString(word).toHtmlEscaped());
}

}  // namespace

class EcoEscritoWindow final : public QMainWindow {
public:
    EcoEscritoWindow() {
        setWindowTitle(QString("Eco-escrito %1").arg(ECO_ESCRITO_VERSION));
        resize(1040, 720);
        // Below this the settings row wraps and the transcript stops being usable.
        setMinimumSize(640, 420);

        auto* central = new QWidget(this);
        auto* layout = new QVBoxLayout(central);
        layout->setContentsMargins(24, 20, 24, 12);
        layout->setSpacing(12);

        build_menus();
        // The navigation sets the floor: narrower, and its labels would be cut.
        setMinimumWidth(std::max(minimumWidth(), menuWidget()->sizeHint().width()));

        auto* recorder_title = new QLabel("Record freely", central);
        recorder_title->setObjectName("pageTitle");
        layout->addWidget(recorder_title);
        auto* recorder_note = new QLabel(
            "Dictate anything. The transcript is saved and appears under Transcripts.", central);
        recorder_note->setObjectName("muted");
        layout->addWidget(recorder_note);

        // ---- Settings: sized to their content, not stretched across the window.
        model_selector_ = new QComboBox(central);
        // base.en first, so it is the default: it transcribes roughly 3.5x
        // faster than small.en, which matters most on long recordings.
        // A name, not a path. The worker searches its approved directories for
        // it, which is the only thing that works when the application is
        // started from a menu entry and inherits no useful working directory.
        model_selector_->addItem("Speed: base.en", "ggml-base.en.bin");
        model_selector_->addItem("Accuracy: small.en", "ggml-small.en.bin");
        model_selector_->setSizeAdjustPolicy(QComboBox::AdjustToContents);

        device_selector_ = new QComboBox(central);
        device_selector_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        device_selector_->setMinimumContentsLength(24);

        duration_selector_ = new QComboBox(central);
        duration_selector_->addItem("15 seconds", 15);
        duration_selector_->addItem("45 seconds", 45);
        duration_selector_->addItem("60 seconds", 60);
        duration_selector_->addItem("5 minutes", 300);
        duration_selector_->addItem("10 minutes", 600);
        duration_selector_->setSizeAdjustPolicy(QComboBox::AdjustToContents);

        keep_audio_ = new QCheckBox("Keep audio files", central);
        keep_audio_->setChecked(true);
        keep_audio_->setToolTip("When off, audio is transcribed and discarded; no WAV file is written.");

        auto* settings = new QHBoxLayout();
        settings->setSpacing(8);
        settings->addWidget(new QLabel("Model:", central));
        settings->addWidget(model_selector_);
        settings->addSpacing(16);
        settings->addWidget(new QLabel("Microphone:", central));
        settings->addWidget(device_selector_);
        settings->addSpacing(16);
        settings->addWidget(new QLabel("Limit:", central));
        settings->addWidget(duration_selector_);
        settings->addSpacing(16);
        settings->addWidget(keep_audio_);
        settings->addStretch();
        layout->addLayout(settings);

        // ---- Recording: button, clock, status and progress read as one thing.
        auto* record_panel = new QGroupBox(central);
        auto* record_layout = new QVBoxLayout(record_panel);
        record_layout->setSpacing(8);

        start_button_ = new QPushButton("Start Recording", record_panel);
        start_button_->setMinimumHeight(36);
        QFont start_font = start_button_->font();
        start_font.setBold(true);
        start_button_->setFont(start_font);
        start_button_->setAccessibleName("Start recording");
        start_button_->setObjectName("primary");
        stop_button_ = new QPushButton("Stop Recording", record_panel);
        stop_button_->setMinimumHeight(36);
        stop_button_->setEnabled(false);

        duration_label_ = new QLabel(record_panel);
        QFont clock_font = duration_label_->font();
        clock_font.setPointSize(clock_font.pointSize() + 4);
        duration_label_->setFont(clock_font);

        auto* record_row = new QHBoxLayout();
        record_row->addWidget(start_button_);
        record_row->addWidget(stop_button_);
        record_row->addSpacing(20);
        record_row->addWidget(duration_label_);
        record_row->addStretch();
        record_layout->addLayout(record_row);

        // A coloured dot reads at a glance; a word has to be found and parsed.
        indicator_ = new QLabel("\u25cf", record_panel);
        indicator_->setToolTip("Idle");
        status_label_ = new QLabel("Ready", record_panel);
        progress_ = new QProgressBar(record_panel);
        progress_->setTextVisible(false);
        progress_->setRange(0, 100);
        progress_->setValue(0);
        progress_->setMaximumHeight(8);
        progress_->hide();

        auto* status_row = new QHBoxLayout();
        status_row->addWidget(indicator_);
        status_row->addWidget(status_label_, 1);
        record_layout->addLayout(status_row);
        record_layout->addWidget(progress_);

        latency_label_ = new QLabel("", record_panel);
        latency_label_->setEnabled(false);
        record_layout->addWidget(latency_label_);
        layout->addWidget(record_panel);

        // ---- Transcript: the output, and the actions that apply to it.
        auto* transcript_header = new QHBoxLayout();
        transcript_title_ = new QLabel("Transcript", central);
        transcript_title_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        transcript_title_->setObjectName("sectionTitle");
        transcript_header->addWidget(transcript_title_);
        transcript_header->addStretch();
        save_button_ = new QPushButton("Save", central);
        copy_button_ = new QPushButton("Copy", central);
        save_button_->setEnabled(false);
        copy_button_->setEnabled(false);
        save_button_->setShortcut(QKeySequence::Save);
        copy_button_->setShortcut(QKeySequence("Ctrl+Shift+C"));
        transcript_header->addWidget(save_button_);
        transcript_header->addWidget(copy_button_);
        layout->addLayout(transcript_header);

        transcript_ = new QPlainTextEdit(central);
        transcript_->setPlaceholderText(
            "Choose a microphone, then press Start Recording.\n\n"
            "Your speech is transcribed on this computer. Nothing is uploaded.");
        transcript_->setReadOnly(true);
        transcript_->setLineWrapMode(QPlainTextEdit::WidgetWidth);
        // Transcripts are read, not skimmed: a little more room per line helps.
        transcript_->document()->setDocumentMargin(10);
        transcript_->setAccessibleName("Transcript");
        transcript_->setAccessibleDescription("The text transcribed from your recording");
        layout->addWidget(transcript_, 1);
        // ---- The library is a separate screen. Recording controls are not
        // merely disabled there, they are absent: nothing on this screen can
        // start a recording, so there is nothing to reason about.
        history_list_ = new QListWidget(this);
        history_list_->setAlternatingRowColors(true);
        history_list_->setContextMenuPolicy(Qt::CustomContextMenu);
        history_list_->setAccessibleName("Saved transcripts");
        history_list_->setMinimumWidth(300);
        history_list_->setUniformItemSizes(false);
        // Elide rather than scroll sideways: a horizontal scrollbar in a list
        // of previews is a worse way to read them than a trimmed line.
        history_list_->setTextElideMode(Qt::ElideRight);
        history_list_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        auto* library_page = new QWidget(this);
        auto* library_layout = new QVBoxLayout(library_page);
        library_layout->setContentsMargins(24, 20, 24, 12);
        library_layout->setSpacing(12);

        auto* library_header = new QHBoxLayout();
        auto* library_title = new QLabel("Saved transcripts", library_page);
        library_title->setObjectName("pageTitle");
        library_header->addWidget(library_title);
        library_header->addStretch();
        delete_all_button_ = new QPushButton("Delete All Recordings...", library_page);
        delete_all_button_->setToolTip("Remove every stored recording and transcript");
        library_header->addWidget(delete_all_button_);
        library_layout->addLayout(library_header);

        auto* library_body = new QHBoxLayout();
        library_body->setSpacing(12);
        library_body->addWidget(history_list_);

        auto* reader = new QWidget(library_page);
        auto* reader_layout = new QVBoxLayout(reader);
        reader_layout->setContentsMargins(0, 0, 0, 0);

        saved_title_ = new QLabel("Select a transcript to read it", reader);
        saved_save_ = new QPushButton("Save a Copy", reader);
        saved_copy_ = new QPushButton("Copy", reader);
        saved_save_->setEnabled(false);
        saved_copy_->setEnabled(false);
        auto* reader_header = new QHBoxLayout();
        reader_header->addWidget(saved_title_);
        reader_header->addStretch();
        reader_header->addWidget(saved_save_);
        reader_header->addWidget(saved_copy_);
        reader_layout->addLayout(reader_header);

        // Its own view, so live output is never disturbed by browsing.
        saved_view_ = new QPlainTextEdit(reader);
        saved_view_->setReadOnly(true);
        saved_view_->setPlaceholderText("Choose a saved transcript from the list.");
        saved_view_->document()->setDocumentMargin(10);
        saved_view_->setAccessibleName("Saved transcript");
        reader_layout->addWidget(saved_view_, 1);

        // Bottom right, under the transcript it would remove, so the target is
        // never ambiguous. Ember rather than plain red: destructive enough to
        // give pause, not an error.
        delete_saved_ = new QPushButton("Delete Transcript", reader);
        delete_saved_->setEnabled(false);
        delete_saved_->setMinimumHeight(32);
        delete_saved_->setCursor(Qt::PointingHandCursor);
        delete_saved_->setToolTip("Delete the transcript shown above. The recording is kept.");
        delete_saved_->setStyleSheet(
            "QPushButton {"
            "  background-color: #c1440e;"
            "  color: #ffffff;"
            "  border: none;"
            "  border-radius: 4px;"
            "  padding: 6px 18px;"
            "}"
            "QPushButton:hover  { background-color: #a63a0c; }"
            "QPushButton:pressed { background-color: #8c3009; }"
            "QPushButton:disabled { background-color: #e0ccc4; color: #9a8d88; }");

        auto* reader_footer = new QHBoxLayout();
        reader_footer->addStretch();
        reader_footer->addWidget(delete_saved_);
        reader_layout->addLayout(reader_footer);

        library_body->addWidget(reader, 1);
        library_layout->addLayout(library_body, 1);

        QWidget* practice_widget = build_practice_page();

        // ---- Speeches: play back what was actually said. This is the raw
        // capture, not the cleaned copy or the extracted speech; those exist
        // for Whisper, and are not what someone wants to hear.
        speech_list_ = new QListWidget(this);
        speech_list_->setAlternatingRowColors(true);
        speech_list_->setMinimumWidth(300);
        speech_list_->setTextElideMode(Qt::ElideRight);
        speech_list_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        speech_list_->setAccessibleName("Recordings");

        auto* speech_page = new QWidget(this);
        auto* speech_layout = new QVBoxLayout(speech_page);
        speech_layout->setContentsMargins(24, 20, 24, 12);
        speech_layout->setSpacing(12);

        auto* speech_header = new QHBoxLayout();
        auto* speech_title = new QLabel("Speeches", speech_page);
        speech_title->setObjectName("pageTitle");
        speech_header->addWidget(speech_title);
        speech_header->addStretch();
        speech_layout->addLayout(speech_header);

        auto* speech_body = new QHBoxLayout();
        speech_body->setSpacing(12);
        speech_body->addWidget(speech_list_);

        auto* player_panel = new QWidget(speech_page);
        auto* player_layout = new QVBoxLayout(player_panel);
        player_layout->setContentsMargins(0, 0, 0, 0);
        player_layout->setSpacing(12);

        // Header matches the transcripts screen, so the two read the same way.
        now_playing_ = new QLabel("Select a recording to play it", player_panel);
        QFont playing_font = now_playing_->font();
        playing_font.setBold(true);
        now_playing_->setFont(playing_font);
        now_playing_->setAlignment(Qt::AlignCenter);
        player_layout->addWidget(now_playing_);

        auto* transport_panel = new QGroupBox(player_panel);
        auto* transport_layout = new QVBoxLayout(transport_panel);
        transport_layout->setSpacing(10);

        play_button_ = new QPushButton("Play", transport_panel);
        play_button_->setMinimumHeight(40);
        play_button_->setMinimumWidth(130);
        QFont play_font = play_button_->font();
        play_font.setBold(true);
        play_button_->setFont(play_font);
        play_button_->setEnabled(false);
        stop_play_button_ = new QPushButton("Stop", transport_panel);
        stop_play_button_->setMinimumHeight(40);
        stop_play_button_->setEnabled(false);

        play_time_ = new QLabel("00:00 / 00:00", transport_panel);
        QFont time_font = play_time_->font();
        time_font.setPointSize(time_font.pointSize() + 3);
        play_time_->setFont(time_font);

        auto* transport = new QHBoxLayout();
        transport->addStretch();
        transport->addWidget(play_button_);
        transport->addWidget(stop_play_button_);
        transport->addSpacing(20);
        transport->addWidget(play_time_);
        transport->addStretch();
        transport_layout->addLayout(transport);

        position_slider_ = new QSlider(Qt::Horizontal, transport_panel);
        position_slider_->setRange(0, 0);
        position_slider_->setEnabled(false);
        transport_layout->addWidget(position_slider_);
        player_layout->addWidget(transport_panel);

        // The space under the controls was empty; this is what belongs in it.
        recording_details_ = new QLabel(player_panel);
        recording_details_->setTextInteractionFlags(Qt::TextSelectableByMouse);
        recording_details_->setWordWrap(true);
        recording_details_->setEnabled(false);
        recording_details_->setAlignment(Qt::AlignCenter);
        player_layout->addWidget(recording_details_);
        player_layout->addStretch();

        delete_speech_ = new QPushButton("Delete Recording", player_panel);
        delete_speech_->setEnabled(false);
        delete_speech_->setMinimumHeight(32);
        delete_speech_->setCursor(Qt::PointingHandCursor);
        delete_speech_->setToolTip("Delete this recording and its processed copies. "
                                  "The transcript is kept.");
        delete_speech_->setStyleSheet(
            "QPushButton {"
            "  background-color: #c1440e;"
            "  color: #ffffff;"
            "  border: none;"
            "  border-radius: 4px;"
            "  padding: 6px 18px;"
            "}"
            "QPushButton:hover  { background-color: #a63a0c; }"
            "QPushButton:pressed { background-color: #8c3009; }"
            "QPushButton:disabled { background-color: #e0ccc4; color: #9a8d88; }");
        auto* speech_footer = new QHBoxLayout();
        speech_footer->addStretch();
        speech_footer->addWidget(delete_speech_);
        player_layout->addLayout(speech_footer);

        speech_body->addWidget(player_panel, 1);
        speech_layout->addLayout(speech_body, 1);

        QWidget* home_page_widget = build_home_page();

        // Added in Page order, which is what setCurrentIndex relies on.
        stack_ = new QStackedWidget(this);
        stack_->addWidget(home_page_widget);
        stack_->addWidget(practice_widget);
        stack_->addWidget(central);
        stack_->addWidget(library_page);
        stack_->addWidget(speech_page);
        setCentralWidget(stack_);

        // ---- Status bar: context that used to be four stacked labels.
        context_label_ = new QLabel(this);
        context_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);
        statusBar()->addWidget(context_label_, 1);
        update_context_bar();

        process_ = new QProcess(this);
        process_->setProcessChannelMode(QProcess::MergedChannels);
        timer_ = new QTimer(this);
        timer_->setInterval(250);
        limit_timer_ = new QTimer(this);
        limit_timer_->setSingleShot(true);
        // Refreshes the elapsed figure while a long transcription runs, so the
        // display keeps moving between chunk reports.
        processing_timer_ = new QTimer(this);
        processing_timer_->setInterval(1000);
        // Playback position lives on the audio thread; this is how the slider
        // and clock learn about it.
        play_timer_ = new QTimer(this);
        play_timer_->setInterval(200);

        populate_devices();
        // Resolved here rather than waiting for the worker, so saved
        // transcripts can be read before anything has been recorded.
        data_directory_ = QString::fromStdString(application_data_directory().string());
        connect(go_record_button_, &QPushButton::clicked, this, [this] { show_recorder(); });
        connect(delete_all_button_, &QPushButton::clicked, this, [this] { delete_recordings(); });
        connect(speech_list_, &QListWidget::currentRowChanged, this,
                [this](int row) { select_recording(row); });
        connect(play_button_, &QPushButton::clicked, this, [this] { toggle_playback(); });
        connect(stop_play_button_, &QPushButton::clicked, this, [this] { stop_playback(); });
        connect(delete_speech_, &QPushButton::clicked, this,
                [this] { delete_selected_recording(); });
        connect(position_slider_, &QSlider::sliderReleased, this, [this] {
            player_.seek_seconds(position_slider_->value());
            refresh_play_time();
        });
        connect(saved_copy_, &QPushButton::clicked, this, [this] { copy_saved(); });
        connect(saved_save_, &QPushButton::clicked, this, [this] { save_saved(); });
        connect(delete_saved_, &QPushButton::clicked, this, [this] { delete_selected_transcript(); });
        connect(history_list_, &QListWidget::currentRowChanged, this,
                [this](int row) { show_history_entry(row); });
        connect(history_list_, &QListWidget::customContextMenuRequested, this,
                [this](const QPoint& at) { show_history_menu(at); });
        connect(duration_selector_, qOverload<int>(&QComboBox::currentIndexChanged), this,
                [this](int) { refresh_clock(0); });
        refresh_clock(0);

        connect(start_button_, &QPushButton::clicked, this, [this] { start_recording(false); });
        connect(stop_button_, &QPushButton::clicked, this, [this] {
            if (state_ == UiState::Recording) {
                stop_recording();
            } else {
                cancel_work();
            }
        });
        connect(copy_button_, &QPushButton::clicked, this, [this] { copy_transcript(); });
        connect(save_button_, &QPushButton::clicked, this, [this] { save_transcript(); });
        connect(process_, &QProcess::readyRead, this, [this] { consume_worker_output(); });
        connect(process_, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
            if (closing_ || error == QProcess::Crashed) {
                return;
            }
            set_failed("Audio worker problem: " + process_->errorString());
        });
        connect(process_, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
                this, [this](int exit_code, QProcess::ExitStatus status) { handle_worker_exit(exit_code, status); });
        connect(timer_, &QTimer::timeout, this, [this] { refresh_elapsed(); });
        connect(processing_timer_, &QTimer::timeout, this, [this] { report_transcription_progress(); });
        connect(play_timer_, &QTimer::timeout, this, [this] { refresh_play_time(); });
        connect(limit_timer_, &QTimer::timeout, this, [this] {
            if (state_ != UiState::Recording) {
                return;
            }
            set_status("Recording limit reached; finishing...");
            stop_recording();
        });

        next_sentence();
        apply_state(UiState::Ready);
        // Start on the landing page, which leads to everything else.
        show_home();
    }

private:
    // The screens, in the order they sit in the stack.
    enum Page { home_page, practice_page, recorder_page, library_page, speeches_page };

    // A practice attempt is the same pipeline with a fixed short limit, literal
    // decoding, and nothing kept: it is a drill, not something to file away.
    void start_recording(bool practice) {
        // Only these states may begin a recording, so a second click, a stray
        // shortcut, or a timer cannot start a concurrent worker.
        if (!ui_state_may_start(state_)) {
            return;
        }
        if (process_->state() != QProcess::NotRunning) {
            return;
        }
        practice_mode_ = practice;

        transcript_->clear();
        final_transcription_.clear();
        transcript_path_.clear();
        model_summary_ = "Validating model";
        input_summary_.clear();
        storage_summary_.clear();
        update_context_bar();
        latency_label_->clear();
        partial_words_.clear();
        partial_trimmed_ = false;
        completed_ = false;
        error_shown_ = false;
        cancelled_ = false;
        chunks_done_ = 0;
        chunks_total_ = 0;
        const QString model = model_selector_->currentData().toString();
        const QString duration = practice ? QString::number(practice_limit_seconds)
                                          : duration_selector_->currentData().toString();
        // No --threads: the worker's measured default is the single source of truth.
        QStringList arguments{model, "--stream", "--duration", duration};
        if (practice) {
            arguments << "--practice" << "--no-retain-audio" << "--no-retain-transcript";
        } else if (!keep_audio_->isChecked()) {
            arguments << "--no-retain-audio";
        }
        const int device = device_selector_->currentData().toInt();
        if (device >= 0) {
            arguments << "--device" << QString::number(device);
        }
        limit_seconds_ = duration.toInt();
        // No waitForStarted: blocking here would freeze the window. The state
        // stays LoadingModel until the worker says it is ready.
        process_->start(worker_program(), arguments);
        apply_state(UiState::LoadingModel);
        set_status("Loading model...");
    }

    // The worker has validated the model, opened the device, and is waiting.
    // Recording, and the clock, start here rather than at the button press.
    void begin_recording() {
        if (state_ != UiState::LoadingModel) {
            return;
        }
        process_->write("\n");
        elapsed_.start();
        timer_->start();
        limit_timer_->start(limit_seconds_ * 1000);
        apply_state(UiState::Recording);
        set_status("Recording. Speak now.");
        refresh_elapsed();
    }

    // Shows how much is done and how long it has taken, so a long wait reads
    // as work rather than as a freeze.
    void report_transcription_progress() {
        if (state_ != UiState::Processing || chunks_total_ <= 0) {
            return;
        }
        progress_->setRange(0, chunks_total_);
        progress_->setValue(chunks_done_);

        const qint64 seconds = processing_elapsed_.isValid() ? processing_elapsed_.elapsed() / 1000 : 0;
        QString status = QString("Transcribing %1 of %2").arg(chunks_done_).arg(chunks_total_);
        if (seconds > 0) {
            status += QString(" (%1s elapsed").arg(seconds);
            // Once a chunk is done the remainder can be estimated honestly.
            if (chunks_done_ > 0 && chunks_done_ < chunks_total_) {
                const qint64 remaining = seconds * (chunks_total_ - chunks_done_) / chunks_done_;
                status += QString(", about %1s left").arg(remaining);
            }
            status += ")";
        }
        set_status(status);
    }

    // Abandons whatever the worker is doing. Available while loading, stopping
    // and transcribing, so no phase is a dead end.
    void cancel_work() {
        if (!ui_state_is_busy(state_)) {
            return;
        }
        timer_->stop();
        limit_timer_->stop();
        processing_timer_->stop();
        set_status("Cancelling...");
        if (process_->state() != QProcess::NotRunning) {
            process_->write("q\n");
            if (!process_->waitForFinished(2000)) {
                process_->kill();
                process_->waitForFinished(1000);
            }
        }
        cancelled_ = true;
        apply_state(UiState::Ready);
        set_status("Cancelled. Start recording to try again.");
    }

    void stop_recording() {
        if (state_ != UiState::Recording) {
            return;
        }
        if (process_->state() == QProcess::Running) {
            process_->write("\n");
        }
        timer_->stop();
        limit_timer_->stop();
        apply_state(UiState::Stopping);
        set_status("Finishing recording...");
    }

    void consume_worker_output() {
        output_buffer_ += QString::fromUtf8(process_->readAll());
        int newline_index = output_buffer_.indexOf('\n');
        while (newline_index >= 0) {
            const QString line = output_buffer_.left(newline_index).trimmed();
            output_buffer_.remove(0, newline_index + 1);
            handle_worker_line(line);
            newline_index = output_buffer_.indexOf('\n');
        }
    }

    void handle_worker_line(const QString& line) {
        if (line.isEmpty()) {
            return;
        }

        if (line.startsWith("ERROR|") || line.startsWith("WARN|")) {
            handle_worker_report(line);
            return;
        }

        if (line.startsWith("SAVED|")) {
            handle_saved_line(line);
            return;
        }

        if (line.startsWith("MODEL|")) {
            handle_model_line(line);
            return;
        }

        if (line.startsWith("INPUT|")) {
            input_summary_ = line.section('|', 1);
            update_context_bar();
            return;
        }

        if (line.startsWith("PARTIAL|")) {
            const QStringList parts = line.split('|');
            if (parts.size() >= 4) {
                append_partial_text(parts.mid(3).join('|'),
                                    parts.at(1).toLongLong(), parts.at(2).toDouble());
            }
            return;
        }

        if (line == "WORDS" || line.startsWith("WORDS|")) {
            handle_words_line(line);
            return;
        }

        if (line.startsWith("FINAL|")) {
            final_transcription_ = line.section('|', 1);
            render_final_transcription();
            if (practice_mode_) {
                render_practice_result();
            }
            // Completion is keyed off the transcription itself, not off a file
            // being written, so it still works when retention is off.
            completed_ = true;
            processing_timer_->stop();
            apply_state(UiState::Completed);
            set_status(QString("Transcription complete (%1s)")
                .arg(processing_elapsed_.isValid() ? processing_elapsed_.elapsed() / 1000 : 0));
            if (process_->state() == QProcess::Running) {
                process_->write("q\n");
            }
            return;
        }

        if (line.startsWith("DATADIR|")) {
            data_directory_ = line.section('|', 1);
            storage_summary_ = data_directory_;
            update_context_bar();
            return;
        }

        if (line.startsWith("PROGRESS|")) {
            const QStringList parts = line.split('|');
            if (parts.size() >= 3) {
                chunks_done_ = parts.at(1).toInt();
                chunks_total_ = parts.at(2).toInt();
                report_transcription_progress();
            }
            return;
        }

        if (line.startsWith("STREAMSTATS|")) {
            handle_stream_stats(line);
            return;
        }

        if (line.startsWith("READY|")) {
            // Emitted at every prompt; only the first one starts a recording.
            begin_recording();
            return;
        }

        if (line.startsWith("PROCESSING|")) {
            if (state_ == UiState::Stopping || state_ == UiState::Recording) {
                timer_->stop();
                limit_timer_->stop();
                chunks_done_ = 0;
                chunks_total_ = 0;
                // Live-streaming figures describe a phase that has ended.
                latency_label_->clear();
                processing_elapsed_.start();
                processing_timer_->start();
                apply_state(UiState::Processing);
                set_status("Preparing audio...");
            }
            return;
        }

    }

    // Secondary actions live here rather than in the toolbar. Deleting
    // recordings is destructive and should be reached deliberately, not brushed
    // past while choosing a microphone.
    // The bar across the top is navigation. Pressing an entry goes to that
    // screen and does nothing else: pressing Recording must not find that a
    // recording has already started. Each screen owns its own actions.
    void build_menus() {
        auto* header = new QWidget(this);
        header->setObjectName("header");
        auto* bar = new QHBoxLayout(header);
        bar->setContentsMargins(16, 8, 16, 8);
        bar->setSpacing(4);

        // The name doubles as the way home, as it does on most sites and many
        // Mac applications.
        auto* brand = new QToolButton(header);
        brand->setObjectName("brand");
        brand->setText("Eco-escrito");
        brand->setIcon(QIcon(":/icons/eco-escrito-512.png"));
        brand->setIconSize(QSize(22, 22));
        brand->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        brand->setCursor(Qt::PointingHandCursor);
        brand->setToolTip("Home (Ctrl+0)");
        brand->setAccessibleName("Eco-escrito home");
        brand->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        bar->addWidget(brand);
        bar->addSpacing(20);

        auto* screens = new QActionGroup(this);
        screens->setExclusive(true);

        const auto add_screen = [this, screens, header, bar](const QString& text, const QKeySequence& key,
                                                             auto handler) {
            auto* action = new QAction(text, this);
            action->setCheckable(true);
            action->setShortcut(key);
            action->setToolTip(QString("%1 (%2)").arg(QString(text).remove('&'),
                                                      key.toString(QKeySequence::NativeText)));
            screens->addAction(action);
            addAction(action);
            connect(action, &QAction::triggered, this, handler);

            auto* button = new QToolButton(header);
            button->setDefaultAction(action);
            // Never squeezed into "Pr…ce": the window stops narrowing instead.
            button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            button->setProperty("nav", true);
            button->setCursor(Qt::PointingHandCursor);
            bar->addWidget(button);
            return action;
        };

        home_action_ = add_screen("H&ome", QKeySequence("Ctrl+0"), [this] { show_home(); });
        connect(brand, &QToolButton::clicked, home_action_, &QAction::trigger);
        practice_action_ = add_screen("&Practice", QKeySequence("Ctrl+1"), [this] { show_practice(); });
        recorder_action_ = add_screen("&Recording", QKeySequence("Ctrl+2"),
                                      [this] { show_recorder(); });
        history_action_ = add_screen("&Transcripts", QKeySequence("Ctrl+H"),
                                     [this] { show_library(); });
        speeches_action_ = add_screen("&Speeches", QKeySequence("Ctrl+P"),
                                      [this] { show_speeches(); });
        bar->addStretch();

        // Said once, where it is always visible, rather than in a footer.
        auto* offline = new QLabel("\u25cf  Offline", header);
        offline->setObjectName("offline");
        offline->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        offline->setToolTip("Everything runs on this computer. Nothing is uploaded.");
        bar->addWidget(offline);
        bar->addSpacing(8);

        auto* help_menu = new QMenu(this);
        help_menu->addAction("&Privacy", this, [this] { show_privacy_notice(); });
        help_menu->addAction("&About", this, [this] { show_about(); });
        help_menu->addSeparator();
        QAction* quit = help_menu->addAction("&Quit", this, [this] { close(); });
        quit->setShortcut(QKeySequence::Quit);
        // A menu's shortcuts only work while it is open unless the window holds
        // the action too.
        addAction(quit);
        auto* help = new QToolButton(header);
        help->setText("&Help");
        help->setMenu(help_menu);
        help->setPopupMode(QToolButton::InstantPopup);
        help->setProperty("nav", true);
        help->setCursor(Qt::PointingHandCursor);
        bar->addWidget(help);

        setMenuWidget(header);

        // Recording keys stay available without occupying the navigation bar.
        record_action_ = new QAction(this);
        record_action_->setShortcut(QKeySequence("Ctrl+R"));
        connect(record_action_, &QAction::triggered, this, [this] {
            if (state_ == UiState::Recording) {
                stop_recording();
            } else if (stack_->currentIndex() == practice_page) {
                start_practice();
            } else {
                show_recorder();
                start_recording(false);
            }
        });
        addAction(record_action_);

        cancel_action_ = new QAction(this);
        cancel_action_->setShortcut(QKeySequence(Qt::Key_Escape));
        connect(cancel_action_, &QAction::triggered, this, [this] { cancel_work(); });
        addAction(cancel_action_);
    }

    // One line replacing four stacked labels. Only what is known is shown, so a
    // fresh window does not read as a list of things that failed.
    void update_context_bar() {
        QStringList parts;
        if (!model_summary_.isEmpty()) {
            parts << model_summary_;
        }
        if (!input_summary_.isEmpty()) {
            parts << input_summary_;
        }
        if (!storage_summary_.isEmpty()) {
            parts << storage_summary_;
        }
        context_label_->setText(parts.isEmpty() ? QString("Ready to record")
                                                : parts.join("   ·   "));
    }

    void copy_saved() {
        QApplication::clipboard()->setText(saved_view_->toPlainText());
        statusBar()->showMessage("Saved transcript copied", 3000);
    }

    void save_saved() {
        const int row = history_list_->currentRow();
        const QString suggested = (row >= 0 && row < static_cast<int>(entries_.size()))
            ? QString::fromStdString(entries_[static_cast<std::size_t>(row)].path.filename().string())
            : QString("transcription.txt");
        const QString path = QFileDialog::getSaveFileName(this, "Save a copy", suggested);
        if (path.isEmpty()) {
            return;
        }
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Save failed", "The transcript could not be saved.");
            return;
        }
        file.write(saved_view_->toPlainText().toUtf8());
    }

    void copy_transcript() {
        QApplication::clipboard()->setText(transcript_->toPlainText());
    }

    // The single place any control's enabled state is decided.
    void apply_state(UiState state) {
        state_ = state;

        const bool has_text = !transcript_->toPlainText().trimmed().isEmpty();
        const ControlStates controls = controls_for(state, devices_ready_, has_text);
        start_button_->setEnabled(controls.start);
        stop_button_->setEnabled(controls.stop || controls.cancel);
        stop_button_->setText(controls.cancel ? "Cancel" : "Stop Recording");
        model_selector_->setEnabled(controls.model);
        duration_selector_->setEnabled(controls.duration);
        device_selector_->setEnabled(controls.device);
        keep_audio_->setEnabled(controls.keep_audio);

        save_button_->setEnabled(controls.save);
        copy_button_->setEnabled(controls.copy);
        record_action_->setEnabled(controls.start || controls.stop);
        record_action_->setText(controls.stop ? "&Stop Recording" : "&Start Recording");
        cancel_action_->setEnabled(controls.cancel);

        history_list_->setEnabled(controls.history);
        // Navigating away mid-recording would hide the controls for the thing
        // still running, so the bar closes while it does.
        history_action_->setEnabled(controls.history);
        home_action_->setEnabled(!ui_state_is_busy(state));
        for (QPushButton* tile : home_tiles_) {
            tile->setEnabled(!ui_state_is_busy(state));
        }
        practice_action_->setEnabled(!ui_state_is_busy(state));
        recorder_action_->setEnabled(!ui_state_is_busy(state));
        speeches_action_->setEnabled(controls.history);
        delete_all_button_->setEnabled(controls.delete_recordings);

        const bool idle = !ui_state_is_busy(state);
        practice_button_->setEnabled(controls.start || controls.stop || controls.cancel);
        practice_button_->setText(state == UiState::Recording ? "\u25a0  Stop and Check"
                                  : controls.cancel           ? "Cancel"
                                                              : "\u25cf  Start Speaking");
        practice_button_->setProperty("recording", state == UiState::Recording);
        practice_button_->style()->unpolish(practice_button_);
        practice_button_->style()->polish(practice_button_);
        sound_selector_->setEnabled(idle && !own_sentence_->isChecked());
        next_sentence_->setEnabled(idle);
        own_sentence_->setEnabled(idle);
        custom_sentence_->setReadOnly(!idle);
        try_again_->setEnabled(controls.start);
        next_after_result_->setEnabled(idle);

        apply_indicator(state);
        explain_disabled_controls(state, controls);

        // Visible only while something is running, so an idle window does not
        // show a bar that looks like stalled work.
        progress_->setVisible(ui_state_is_busy(state));

        if (controls.progress_indeterminate) {
            // The worker gives no completion fraction for these phases.
            progress_->setRange(0, 0);
        } else if (state == UiState::Recording) {
            progress_->setRange(0, limit_seconds_ > 0 ? limit_seconds_ : 100);
        } else if (state == UiState::Processing) {
            // Filled in by PROGRESS| reports; start indeterminate only until
            // the first one arrives.
            progress_->setRange(0, chunks_total_ > 0 ? chunks_total_ : 0);
            progress_->setValue(chunks_done_);
        } else {
            progress_->setRange(0, 100);
            progress_->setValue(0);
        }
    }

    // Elapsed against the limit, so the limit is visible before you commit to it.
    // Colour carries the state; the tooltip spells it out for anyone who cannot
    // rely on colour alone.
    void apply_indicator(UiState state) {
        QString colour = "#9e9e9e";
        QString meaning = "Idle";
        switch (state) {
            case UiState::Recording:
                colour = "#d32f2f";
                meaning = "Recording";
                break;
            case UiState::LoadingModel:
            case UiState::Stopping:
            case UiState::Processing:
                colour = "#f9a825";
                meaning = "Working";
                break;
            case UiState::Completed:
                colour = "#2e7d32";
                meaning = "Finished";
                break;
            case UiState::Error:
                colour = "#c62828";
                meaning = "Stopped with a problem";
                break;
            default:
                break;
        }
        indicator_->setStyleSheet("color: " + colour + ";");
        indicator_->setToolTip(meaning);
        indicator_->setAccessibleName(meaning);
    }

    // A disabled control that does not say why is a dead end. Each one explains
    // itself in the state it is unavailable in.
    void explain_disabled_controls(UiState state, const ControlStates& controls) {
        const QString busy = "Not available while a recording is in progress.";

        start_button_->setToolTip(controls.start
            ? "Begin recording (Ctrl+R)"
            : busy);
        stop_button_->setToolTip(controls.stop
            ? "Stop recording and transcribe (Ctrl+R)"
            : (controls.cancel ? "Abandon this recording (Esc)"
                               : "Nothing is running."));

        const QString settings_tip = ui_state_is_idle(state)
            ? QString()
            : "Cannot be changed until the current recording finishes.";
        model_selector_->setToolTip(settings_tip);
        duration_selector_->setToolTip(settings_tip);
        keep_audio_->setToolTip(ui_state_is_idle(state)
            ? "When off, audio is transcribed and discarded; no WAV file is written."
            : settings_tip);
        device_selector_->setToolTip(!devices_ready_
            ? "Looking for microphones..."
            : settings_tip);

        const QString nothing_yet = "Available once a transcription has finished.";
        save_button_->setToolTip(controls.save ? "Save the transcript to a file (Ctrl+S)" : nothing_yet);
        copy_button_->setToolTip(controls.copy ? "Copy the transcript (Ctrl+Shift+C)" : nothing_yet);
    }

    // ------------------------------------------------------------- history

    void show_practice() {
        if (ui_state_is_busy(state_)) {
            return;
        }
        stop_playback_if_running();
        stack_->setCurrentIndex(practice_page);
        practice_action_->setChecked(true);
        context_label_->setText("Practise a sentence, or record freely");
        apply_state(state_);
    }

    void show_speeches() {
        if (!ui_state_may_start(state_)) {
            return;
        }
        refresh_speeches();
        stack_->setCurrentIndex(speeches_page);
        speeches_action_->setChecked(true);
        context_label_->setText(recordings_.empty()
            ? QString("No recordings")
            : QString("%1 recording%2").arg(recordings_.size())
                  .arg(recordings_.size() == 1 ? "" : "s"));
        record_action_->setEnabled(false);
        cancel_action_->setEnabled(false);
        speech_list_->setFocus();
    }

    void refresh_speeches() {
        const int previous = speech_list_->currentRow();
        const QSignalBlocker blocker(speech_list_);
        speech_list_->clear();
        recordings_ = list_recordings(fs::path(data_directory_.toStdString()));

        if (recordings_.empty()) {
            auto* empty = new QListWidgetItem(
                "No recordings yet\n\nGo to Recording to make one", speech_list_);
            empty->setFlags(Qt::NoItemFlags);
            return;
        }

        for (const RecordingEntry& entry : recordings_) {
            // The name the file carries, and nothing else. Length and transcript
            // status belong to the recording being played, not to the list.
            char stamp[32];
            std::tm shown = entry.stamp.when;
            std::strftime(stamp, sizeof(stamp), "%Y%m%d%H%M%S", &shown);
            auto* item = new QListWidgetItem(
                QString("recording-%1").arg(QString::fromUtf8(stamp)), speech_list_);
            item->setToolTip(QString::fromStdString(entry.path.string()));
        }
        if (previous >= 0 && previous < speech_list_->count()) {
            speech_list_->setCurrentRow(previous);
        }
    }

    void select_recording(int row) {
        player_.unload();
        play_timer_->stop();
        if (row < 0 || row >= static_cast<int>(recordings_.size())) {
            now_playing_->setText(recordings_.empty()
                ? QString("Nothing recorded yet")
                : QString("Select a recording to play it"));
            recording_details_->setText(QString());
            play_button_->setEnabled(false);
            stop_play_button_->setEnabled(false);
            delete_speech_->setEnabled(false);
            position_slider_->setEnabled(false);
            position_slider_->setRange(0, 0);
            play_time_->setText("00:00 / 00:00");
            return;
        }

        const RecordingEntry& entry = recordings_[static_cast<std::size_t>(row)];
        std::string reason;
        if (!player_.load(entry.path, reason)) {
            now_playing_->setText("Could not open this recording");
            recording_details_->setText(QString::fromStdString(reason));
            play_button_->setEnabled(false);
            stop_play_button_->setEnabled(false);
            delete_speech_->setEnabled(true);
            return;
        }

        char when[64];
        std::tm shown = entry.stamp.when;
        std::strftime(when, sizeof(when), "%d %b %Y at %H:%M", &shown);
        char stamp[32];
        std::strftime(stamp, sizeof(stamp), "%Y%m%d%H%M%S", &shown);
        now_playing_->setText(QString("recording-%1").arg(QString::fromUtf8(stamp)));
        recording_details_->setText(QString("%1 long, %2 MB.  %3")
            .arg(QTime(0, 0).addSecs(static_cast<int>(entry.seconds)).toString("mm:ss"),
                 QString::number(entry.size_bytes / (1024.0 * 1024.0), 'f', 1),
                 entry.has_transcript ? "Transcribed." : "Not transcribed."));
        play_button_->setEnabled(true);
        play_button_->setText("Play");
        stop_play_button_->setEnabled(true);
        delete_speech_->setEnabled(true);
        position_slider_->setEnabled(true);
        position_slider_->setRange(0, static_cast<int>(player_.duration_seconds()));
        position_slider_->setValue(0);
        refresh_play_time();
    }

    void toggle_playback() {
        if (!player_.is_loaded()) {
            return;
        }
        if (player_.is_playing()) {
            player_.pause();
            play_button_->setText("Play");
            play_timer_->stop();
        } else if (player_.play()) {
            play_button_->setText("Pause");
            play_timer_->start();
        }
        refresh_play_time();
    }

    void stop_playback() {
        player_.stop();
        play_timer_->stop();
        play_button_->setText("Play");
        position_slider_->setValue(0);
        refresh_play_time();
    }

    void refresh_play_time() {
        const int position = static_cast<int>(player_.position_seconds());
        const int total = static_cast<int>(player_.duration_seconds());
        const char* format = total >= 3600 ? "hh:mm:ss" : "mm:ss";
        play_time_->setText(QString("%1 / %2")
            .arg(QTime(0, 0).addSecs(position).toString(format),
                 QTime(0, 0).addSecs(total).toString(format)));
        if (!position_slider_->isSliderDown()) {
            const QSignalBlocker blocker(position_slider_);
            position_slider_->setValue(position);
        }
        // Playback runs on its own thread; this is how the interface learns it
        // reached the end.
        if (player_.has_finished() && player_.is_playing()) {
            stop_playback();
        }
    }

    void delete_selected_recording() {
        const int row = speech_list_->currentRow();
        if (row < 0 || row >= static_cast<int>(recordings_.size())) {
            return;
        }
        const RecordingEntry entry = recordings_[static_cast<std::size_t>(row)];
        char when[64];
        std::tm shown = entry.stamp.when;
        std::strftime(when, sizeof(when), "%d %b %Y, %H:%M", &shown);

        const auto choice = QMessageBox::question(this, "Delete recording",
            QString("Delete the recording from %1?\n\nIts transcript is kept. "
                    "This cannot be undone.").arg(QString::fromUtf8(when)),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (choice != QMessageBox::Yes) {
            return;
        }

        // Released before deleting: the file is open for playback.
        player_.unload();
        play_timer_->stop();

        std::string reason;
        if (!delete_recording(entry.path, entry.stamp,
                              fs::path(data_directory_.toStdString()), reason)) {
            QMessageBox::warning(this, "Could not delete", QString::fromStdString(reason));
            return;
        }

        refresh_speeches();
        const int remaining = static_cast<int>(recordings_.size());
        if (remaining == 0) {
            select_recording(-1);
        } else {
            const int next = qBound(0, row, remaining - 1);
            const QSignalBlocker blocker(speech_list_);
            speech_list_->setCurrentRow(next);
            select_recording(next);
        }
        statusBar()->showMessage("Recording deleted", 3000);
    }

    void show_library() {
        if (!ui_state_may_start(state_)) {
            return;
        }
        stop_playback_if_running();
        refresh_history();
        stack_->setCurrentIndex(library_page);
        // The status bar belongs to whichever screen is showing.
        context_label_->setText(entries_.empty()
            ? QString("No saved transcripts")
            : QString("%1 saved transcript%2").arg(entries_.size())
                  .arg(entries_.size() == 1 ? "" : "s"));
        history_action_->setChecked(true);
        // Nothing on this screen records, so the recording keys are inert here.
        record_action_->setEnabled(false);
        cancel_action_->setEnabled(false);
        history_list_->setFocus();
    }

    void stop_playback_if_running() {
        if (player_.is_loaded()) {
            stop_playback();
            player_.unload();
        }
    }

    void show_recorder() {
        stop_playback_if_running();
        // Microphones are plugged in and unplugged while the application runs.
        // Re-reading on arrival keeps the indices honest.
        if (ui_state_may_start(state_)) {
            refresh_devices();
        }
        stack_->setCurrentIndex(recorder_page);
        recorder_action_->setChecked(true);
        update_context_bar();
        apply_state(state_);
    }

    void toggle_library() {
        if (stack_->currentIndex() == library_page) {
            show_practice();
        } else {
            show_library();
        }
    }

    // Rebuilt from disk rather than tracked incrementally: the list is short,
    // and a stale list is worse than a cheap re-read.
    void refresh_history() {
        const int previous_row = history_list_->currentRow();
        const QSignalBlocker blocker(history_list_);
        history_list_->clear();
        entries_ = list_transcripts(fs::path(data_directory_.toStdString()));

        if (entries_.empty()) {
            auto* empty = new QListWidgetItem(
                "No saved transcripts yet\n\nGo to Recording to make one", history_list_);
            empty->setFlags(Qt::NoItemFlags);
            return;
        }

        for (const TranscriptEntry& entry : entries_) {
            // The name it was saved under, and nothing else. What it says is one
            // click away; repeating a fragment of it on every row is noise.
            char stamp[32];
            std::tm shown = entry.stamp.when;
            std::strftime(stamp, sizeof(stamp), "%Y%m%d%H%M%S", &shown);
            auto* item = new QListWidgetItem(
                QString("transcription-%1").arg(QString::fromUtf8(stamp)), history_list_);
            item->setToolTip(QString::fromStdString(entry.path.string()));
        }
        if (previous_row >= 0 && previous_row < history_list_->count()) {
            history_list_->setCurrentRow(previous_row);
        }
    }

    void clear_reader() {
        saved_view_->clear();
        saved_title_->setText("Select a transcript to read it");
        saved_save_->setEnabled(false);
        saved_copy_->setEnabled(false);
        delete_saved_->setEnabled(false);
    }

    void show_history_entry(int row) {
        if (row < 0 || row >= static_cast<int>(entries_.size())) {
            clear_reader();
            return;
        }
        const TranscriptEntry& entry = entries_[static_cast<std::size_t>(row)];

        std::string text;
        std::string reason;
        if (!read_transcript(entry.path, fs::path(data_directory_.toStdString()), text, reason)) {
            QMessageBox::warning(this, "Could not open transcript",
                                 QString::fromStdString(reason));
            return;
        }

        // Transcripts written before the text was trimmed at the source still
        // carry Whisper's leading space; the file is left alone, the display is
        // not.
        saved_view_->setPlainText(QString::fromStdString(text).trimmed());
        saved_view_->moveCursor(QTextCursor::Start);

        char stamp[32];
        std::tm shown = entry.stamp.when;
        std::strftime(stamp, sizeof(stamp), "%Y%m%d%H%M%S", &shown);
        const CompanionAudio audio =
            companion_audio(entry.stamp, fs::path(data_directory_.toStdString()));
        saved_title_->setText(QString("transcription-%1%2")
            .arg(QString::fromUtf8(stamp), audio.any() ? "" : "   (audio deleted)"));
        saved_save_->setEnabled(true);
        saved_copy_->setEnabled(true);
        delete_saved_->setEnabled(true);
    }

    // Acts on what is on screen, which is what the button sits beneath.
    void delete_selected_transcript() {
        const int row = history_list_->currentRow();
        if (row < 0 || row >= static_cast<int>(entries_.size())) {
            return;
        }
        const TranscriptEntry& entry = entries_[static_cast<std::size_t>(row)];
        char when[64];
        std::tm shown = entry.stamp.when;
        std::strftime(when, sizeof(when), "%d %b %Y, %H:%M", &shown);
        delete_one_transcript(entry.path, QString::fromUtf8(when));
    }

    // Deletes a single transcript. The recording it came from is deliberately
    // left alone: File > Delete All Recordings is the way to remove audio.
    void delete_one_transcript(const fs::path& path, const QString& label) {
        const auto choice = QMessageBox::question(this, "Delete transcript",
            QString("Delete the transcript from %1?\n\nThe recording it came from is kept. "
                    "This cannot be undone.").arg(label),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (choice != QMessageBox::Yes) {
            return;
        }

        std::string reason;
        if (!remove_transcript(path, reason)) {
            QMessageBox::warning(this, "Could not delete", QString::fromStdString(reason));
            return;
        }
        statusBar()->showMessage("Transcript deleted", 3000);
    }

    // The deletion itself, separated from asking: this is what decides where
    // the reader lands afterwards, and it is worth being able to exercise.
    bool remove_transcript(const fs::path& path, std::string& reason) {
        if (!delete_transcript(path, fs::path(data_directory_.toStdString()), reason)) {
            return false;
        }

        // Deleting should leave you somewhere, not staring at an empty box.
        // The row that takes the deleted one's place is the natural next thing
        // to read; at the end of the list that is the one above instead.
        const int removed_row = history_list_->currentRow();
        refresh_history();

        const int remaining = static_cast<int>(entries_.size());
        if (remaining == 0) {
            clear_reader();
        } else {
            const int next = qBound(0, removed_row, remaining - 1);
            const QSignalBlocker blocker(history_list_);
            history_list_->setCurrentRow(next);
            // Called directly: the row index may be unchanged, so relying on
            // the selection signal would leave the old text on screen.
            show_history_entry(next);
        }
        return true;
    }

    void show_history_menu(const QPoint& at) {
        const int row = history_list_->currentRow();
        if (row < 0 || row >= static_cast<int>(entries_.size())) {
            return;
        }
        const TranscriptEntry entry = entries_[static_cast<std::size_t>(row)];
        const fs::path root(data_directory_.toStdString());
        const CompanionAudio audio = companion_audio(entry.stamp, root);

        QMenu menu(this);
        menu.addAction("&Copy", this, [this] { copy_saved(); });
        menu.addAction("&Save a Copy...", this, [this] { save_saved(); });
        menu.addSeparator();

        QStringList kept;
        if (audio.has_recording) { kept << "original"; }
        if (audio.has_cleaned) { kept << "cleaned"; }
        if (audio.has_speech) { kept << "speech only"; }
        QAction* audio_note = menu.addAction(kept.isEmpty()
            ? QString("Audio: not kept")
            : QString("Audio: %1").arg(kept.join(", ")));
        audio_note->setEnabled(false);

        menu.addSeparator();
        menu.addAction("&Delete This Transcript...", this, [this, entry] {
            char when[64];
            std::tm shown = entry.stamp.when;
            std::strftime(when, sizeof(when), "%d %b %Y, %H:%M", &shown);
            delete_one_transcript(entry.path, QString::fromUtf8(when));
        });
        menu.exec(history_list_->mapToGlobal(at));
    }

    void refresh_clock(qint64 seconds) {
        const int limit = limit_seconds_ > 0 ? limit_seconds_
                                             : duration_selector_->currentData().toInt();
        duration_label_->setText(QString("%1 / %2")
            .arg(QTime(0, 0).addSecs(static_cast<int>(seconds)).toString("mm:ss"),
                 QTime(0, 0).addSecs(limit).toString("mm:ss")));
    }

    void refresh_elapsed() {
        if (state_ != UiState::Recording) {
            return;
        }
        // Monotonic: unaffected by clock changes or midnight rollover.
        const qint64 seconds = elapsed_.elapsed() / 1000;
        refresh_clock(seconds);
        progress_->setValue(static_cast<int>(qMin<qint64>(seconds, limit_seconds_)));
    }

    // Asks the worker to enumerate capture devices before any recording starts.
    void refresh_devices() {
        const QVariant chosen = device_selector_->currentData();
        device_selector_->clear();
        device_selector_->addItem("System default", -1);
        populate_devices(chosen);
    }

    void populate_devices(const QVariant& restore = QVariant()) {
        if (device_selector_->count() == 0) {
            device_selector_->addItem("System default", -1);
        }
        restore_device_ = restore;
        device_selector_->setEnabled(false);

        // Enumeration runs asynchronously; blocking here froze the window for
        // as long as the audio backend took to answer.
        auto* probe = new QProcess(this);
        connect(probe, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
                [this, probe](int, QProcess::ExitStatus) {
            add_enumerated_devices(QString::fromUtf8(probe->readAllStandardOutput()));
            probe->deleteLater();
        });
        connect(probe, &QProcess::errorOccurred, this, [this, probe](QProcess::ProcessError) {
            devices_ready_ = true;
            apply_state(state_);
            probe->deleteLater();
        });
        probe->start(worker_program(), {"--list-devices"});
    }

    // DEVICE|index|default|name|kind. The name may itself contain a separator,
    // so it is taken as everything between the known first and last fields.
    void add_enumerated_devices(const QString& output) {
        const QStringList lines = output.split('\n');
        for (const QString& line : lines) {
            if (!line.startsWith("DEVICE|")) {
                continue;
            }
            const QStringList parts = line.split('|');
            if (parts.size() < 5) {
                continue;
            }
            const int index = parts.at(1).toInt();
            const QString kind = parts.last().trimmed();
            const QString name = parts.mid(3, parts.size() - 4).join('|').trimmed();
            const bool is_default = parts.at(2) == "default";

            QString label = name;
            if (is_default) {
                label += "  (default)";
            }
            if (kind == "monitor") {
                // Saying so is the difference between a puzzling silent
                // recording and an obviously wrong choice.
                label += "  — speaker output, not a microphone";
            }
            device_selector_->addItem(label, index);
            if (kind == "monitor") {
                const int row = device_selector_->count() - 1;
                device_selector_->setItemData(row, QColor(Qt::darkGray), Qt::ForegroundRole);
                device_selector_->setItemData(
                    row, "Records what is played through the speakers, not what you say.",
                    Qt::ToolTipRole);
            }
        }
        devices_ready_ = true;
        // Keep the user's choice across a refresh when the device is still there.
        if (restore_device_.isValid()) {
            const int row = device_selector_->findData(restore_device_);
            if (row >= 0) {
                device_selector_->setCurrentIndex(row);
            }
            restore_device_ = QVariant();
        }
        apply_state(state_);
    }

    // The worker reports the validated model as MODEL|variant|language|bytes|path.
    void handle_model_line(const QString& line) {
        const QStringList parts = line.split('|');
        if (parts.size() < 5) {
            return;
        }

        const QString variant = parts.at(1);
        const QString language = parts.at(2) == "multilingual" ? "multilingual" : "English-only";
        const double megabytes = parts.at(3).toDouble() / (1024.0 * 1024.0);
        model_summary_ = QString("%1 (%2, %3 MB)")
            .arg(variant, language, QString::number(megabytes, 'f', 1));
        update_context_bar();
    }

    // The worker announces each artefact as SAVED|KIND|path.
    void handle_saved_line(const QString& line) {
        const QStringList parts = line.split('|');
        if (parts.size() < 3 || parts.at(1) != "TRANSCRIPT") {
            return;
        }

        transcript_path_ = parts.mid(2).join('|');
        storage_summary_ = transcript_path_;
        update_context_bar();
        if (stack_->currentIndex() == library_page) {
            refresh_history();
        }
        if (stack_->currentIndex() == speeches_page) {
            refresh_speeches();
        }
    }

    // Worker reports arrive as SEVERITY|CATEGORY|message.
    void handle_worker_report(const QString& line) {
        const QStringList parts = line.split('|');
        if (parts.size() < 3) {
            return;
        }

        const QString severity = parts.at(0);
        const QString category = parts.at(1);
        const QString message = parts.mid(2).join('|');

        if (severity == "WARN") {
            set_status(message);
            return;
        }

        error_shown_ = true;
        set_failed(actionable_message(category, message));
        if (process_->state() == QProcess::Running) {
            process_->write("q\n");
        }
    }

    static QString actionable_message(const QString& category, const QString& message) {
        if (category == "MICROPHONE") {
            return message + " Check that a microphone is connected and that this application may use it.";
        }
        if (category == "MODEL") {
            return message + " Download the model into models/ and select it again.";
        }
        if (category == "RECORDING") {
            return message + " Speak closer to the microphone and record again.";
        }
        if (category == "TRANSCRIPTION") {
            return message + " Try the base.en model, or record a shorter clip.";
        }
        if (category == "FILE_SAVING") {
            return message + " Check free disk space and write permissions for the working directory.";
        }
        return message;
    }

    void handle_worker_exit(int exit_code, QProcess::ExitStatus status) {
        if (closing_) {
            return;
        }
        if (cancelled_) {
            // We asked it to stop, and may have killed it. That is not a crash,
            // and the message already on screen is the accurate one.
            return;
        }
        if (error_shown_) {
            // Stay in Error: the reported cause remains on screen, and any text
            // that did arrive stays saveable.
            set_failed(status_label_->text());
            return;
        }
        if (status == QProcess::CrashExit) {
            set_failed("The audio worker stopped unexpectedly. Start recording to try again.");
            return;
        }
        if (exit_code != 0) {
            set_failed("The audio worker exited with code " + QString::number(exit_code) + ".");
            return;
        }
        if (completed_ && practice_mode_) {
            set_idle("Done. Read it again, or try another sentence.");
            return;
        }
        set_idle(completed_ ? "Transcription complete" : "Ready");
    }

    void append_partial_text(const QString& incoming, qint64 latency_ms, double queue_seconds) {
        latency_label_->setText(QString("Partial latency: %1 ms, %2 s behind live")
            .arg(latency_ms).arg(QString::number(queue_seconds, 'f', 1)));

        partial_text::append_with_overlap(partial_words_, incoming,
            maximum_overlap_words, maximum_partial_words, partial_trimmed_);
        render_partial_text();
    }

    void render_partial_text() {
        const QString body = partial_words_.join(' ');
        transcript_->setPlainText(partial_trimmed_ ? "[earlier text trimmed] " + body : body);
        transcript_->moveCursor(QTextCursor::End);
    }

    void handle_stream_stats(const QString& line) {
        int dropped = 0;
        int rate_limited = 0;
        for (const QString& field : line.split('|')) {
            if (field.startsWith("dropped=")) {
                dropped = field.section('=', 1).toInt();
            } else if (field.startsWith("rate_limited=")) {
                rate_limited = field.section('=', 1).toInt();
            }
        }
        if (dropped > 0 || rate_limited > 0) {
            latency_label_->setText(QString("Live text skipped %1 window(s); the final "
                                            "transcription covers everything")
                .arg(dropped + rate_limited));
        }
    }

    void show_about() {
        QMessageBox::about(this, "About Eco-escrito",
            QString("<b>Eco-escrito %1</b><br>"
                    "Build %2, %3<br><br>"
                    "Offline speech to text. Audio never leaves this computer.<br><br>"
                    "Transcription by whisper.cpp, audio capture by miniaudio, "
                    "interface built with Qt 6.")
                .arg(ECO_ESCRITO_VERSION, ECO_ESCRITO_GIT_COMMIT, ECO_ESCRITO_BUILD_TYPE));
    }

    void show_privacy_notice() {
        QMessageBox::information(this, "Privacy",
            "Audio is captured, analysed and transcribed entirely on this computer.\n\n"
            "Whisper runs locally against a model file on disk. No audio, transcript or "
            "metadata is uploaded, and neither this window nor the worker it runs opens a "
            "network connection.\n\n"
            "Saved to:\n" + (data_directory_.isEmpty() ? QString("(shown once a recording starts)")
                                                       : data_directory_) + "\n\n"
            "Files are created readable only by you. Nothing is written to a log file.\n\n"
            "Turn off \"Keep audio files\" to transcribe without keeping any WAV file. "
            "\"Delete recordings\" removes every stored recording and transcript.");
    }

    void delete_recordings() {
        const auto choice = QMessageBox::question(this, "Delete recordings",
            "Delete every stored recording and transcript?\n\nThis cannot be undone.",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (choice != QMessageBox::Yes) {
            return;
        }

        QProcess cleaner;
        cleaner.start(worker_program(), {"--delete-recordings"});
        if (!cleaner.waitForFinished(10000)) {
            cleaner.kill();
            cleaner.waitForFinished(1000);
            QMessageBox::warning(this, "Delete recordings", "The deletion did not finish.");
            return;
        }

        const QStringList lines = QString::fromUtf8(cleaner.readAllStandardOutput()).split('\n');
        for (const QString& line : lines) {
            if (!line.startsWith("DELETED|")) {
                continue;
            }
            const QStringList parts = line.split('|');
            if (parts.size() >= 3) {
                QMessageBox::information(this, "Delete recordings",
                    QString("Deleted %1 file(s), %2 MB.")
                        .arg(parts.at(1))
                        .arg(QString::number(parts.at(2).toDouble() / (1024.0 * 1024.0), 'f', 1)));
                transcript_path_.clear();
                refresh_history();
                return;
            }
        }
        QMessageBox::warning(this, "Delete recordings", "Nothing was reported as deleted.");
    }

    void save_transcript() {
        const QString suggested = transcript_path_.isEmpty() ? QString("transcription.txt") : transcript_path_;
        const QString path = QFileDialog::getSaveFileName(this, "Save a copy of the transcription", suggested);
        if (path.isEmpty()) {
            return;
        }
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Save failed", "The transcription could not be saved.");
            return;
        }
        file.write(transcript_->toPlainText().toUtf8());
    }

    void render_final_transcription() {
        if (final_transcription_.isEmpty()) {
            return;
        }
        transcript_->setPlainText(final_transcription_);
        transcript_->moveCursor(QTextCursor::End);
    }

    void closeEvent(QCloseEvent* event) override {
        if (ui_state_is_busy(state_)) {
            const auto choice = QMessageBox::question(this, "Recording in progress",
                "A recording is still being transcribed. Closing now discards it.\n\nClose anyway?",
                QMessageBox::Close | QMessageBox::Cancel, QMessageBox::Cancel);
            if (choice != QMessageBox::Close) {
                event->ignore();
                return;
            }
        }

        closing_ = true;
        timer_->stop();
        limit_timer_->stop();
        if (process_->state() != QProcess::NotRunning) {
            process_->write("q\n");
            if (!process_->waitForFinished(3000)) {
                process_->kill();
                process_->waitForFinished(1000);
            }
        }
        QMainWindow::closeEvent(event);
    }

    // ---------------------------------------------------------------- home

    // One entry per feature. A feature added later gets a tile by adding a line
    // to the table in build_home_page, in the same grid and voice as the rest.
    struct HomeFeature {
        QString title;
        QString description;
        QString meta;
        FeatureGlyph glyph;
        QColor top;
        QColor bottom;
        QString shortcut;
        QLabel** meta_label;
        std::function<void()> open;
    };

    // The landing page: who it is for, what it can do, and what was done last.
    // Laid out the way a Mac application's welcome window is: a calm hero, a
    // grid of large targets, and a short list of recent work.
    QWidget* build_home_page() {
        auto* content = new QWidget(this);
        content->setMaximumWidth(860);
        content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        auto* layout = new QVBoxLayout(content);
        layout->setContentsMargins(32, 40, 32, 28);
        layout->setSpacing(0);

        // ---- Hero. Centred, as a Mac welcome window is: one column, so the
        // eye lands on the icon, then the promise, then the choices below.
        auto* app_icon = new QLabel(content);
        app_icon->setObjectName("appIcon");
        app_icon->setPixmap(QIcon(":/icons/eco-escrito-512.png").pixmap(46, 46));
        app_icon->setAlignment(Qt::AlignCenter);
        app_icon->setFixedSize(76, 76);
        layout->addWidget(app_icon, 0, Qt::AlignHCenter);
        layout->addSpacing(18);

        home_greeting_ = new QLabel(content);
        home_greeting_->setObjectName("eyebrow");
        home_greeting_->setAlignment(Qt::AlignCenter);
        layout->addWidget(home_greeting_);
        layout->addSpacing(2);
        auto* title = new QLabel("Your voice, in writing.", content);
        title->setObjectName("homeTitle");
        title->setAlignment(Qt::AlignCenter);
        layout->addWidget(title);
        layout->addSpacing(8);
        auto* lead = new QLabel("Practise your pronunciation, dictate freely, and keep every transcript. "
                                "Everything happens on this computer, and nothing is sent anywhere.", content);
        lead->setObjectName("homeLead");
        lead->setWordWrap(true);
        lead->setAlignment(Qt::AlignCenter);
        // A measure of about sixty characters reads comfortably; the full width
        // does not. Centred by stretches rather than an alignment flag, which
        // would stop the layout asking the label how tall its wrapped text is.
        lead->setMaximumWidth(560);
        auto* lead_row = new QHBoxLayout();
        lead_row->addStretch();
        lead_row->addWidget(lead, 1);
        lead_row->addStretch();
        layout->addLayout(lead_row);
        layout->addSpacing(40);

        // ---- Features.
        const std::vector<HomeFeature> features = {
            {"Practice", "Read a sentence aloud and see which words came through, changed, or were missed.",
             QString("%1 sentences, 15 seconds each").arg(practice_sentences().size()),
             FeatureGlyph::Practice, QColor("#7d7aff"), QColor("#5856d6"), "Ctrl+1", nullptr,
             [this] { show_practice(); }},
            {"Recording", "Speak freely and watch the words appear. The transcript is saved for you.",
             "Speed or accuracy, up to 10 minutes",
             FeatureGlyph::Microphone, QColor("#ff7a6e"), QColor("#ff3b30"), "Ctrl+2", nullptr,
             [this] { show_recorder(); }},
            {"Transcripts", "Read, copy, save, or delete everything you have dictated.",
             QString(), FeatureGlyph::Document, QColor("#5ab0ff"), QColor("#007aff"), "Ctrl+H",
             &home_transcripts_meta_, [this] { show_library(); }},
            {"Speeches", "Listen back to your recordings, exactly as you said them.",
             QString(), FeatureGlyph::Waveform, QColor("#ffbb4d"), QColor("#ff9500"), "Ctrl+P",
             &home_speeches_meta_, [this] { show_speeches(); }},
        };

        auto* features_title = new QLabel("Get started", content);
        features_title->setObjectName("groupTitle");
        layout->addWidget(features_title);
        layout->addSpacing(10);

        home_grid_ = new QGridLayout();
        home_grid_->setHorizontalSpacing(12);
        home_grid_->setVerticalSpacing(12);
        for (const HomeFeature& feature : features) {
            build_feature_tile(feature, content);
        }
        arrange_tiles(2);
        layout->addLayout(home_grid_);
        // Two columns while each tile has room for its description, one below.
        content->installEventFilter(this);
        home_content_ = content;
        layout->addSpacing(32);

        // ---- Recent work.
        auto* recent_header = new QHBoxLayout();
        auto* recent_title = new QLabel("Recent transcripts", content);
        recent_title->setObjectName("groupTitle");
        recent_header->addWidget(recent_title);
        recent_header->addStretch();
        see_all_ = new QPushButton("See All", content);
        see_all_->setObjectName("link");
        see_all_->setCursor(Qt::PointingHandCursor);
        recent_header->addWidget(see_all_);
        layout->addLayout(recent_header);
        layout->addSpacing(6);

        recent_group_ = new QFrame(content);
        recent_group_->setObjectName("group");
        recent_layout_ = new QVBoxLayout(recent_group_);
        recent_layout_->setContentsMargins(1, 4, 1, 4);
        recent_layout_->setSpacing(0);
        layout->addWidget(recent_group_);
        layout->addSpacing(28);

        // ---- The promise, said once and quietly.
        auto* footnote = new QLabel(
            "<span style='color:#34c759'>\u25cf</span>&nbsp; Running offline. "
            "Audio and transcripts never leave this computer.", content);
        footnote->setObjectName("footnote");
        footnote->setTextFormat(Qt::RichText);
        footnote->setAlignment(Qt::AlignCenter);
        layout->addWidget(footnote);
        layout->addStretch();

        connect(see_all_, &QPushButton::clicked, this, [this] { show_library(); });

        auto* holder = new QWidget(this);
        holder->setObjectName("page");
        auto* holder_layout = new QHBoxLayout(holder);
        holder_layout->setContentsMargins(0, 0, 0, 0);
        holder_layout->addStretch(1);
        holder_layout->addWidget(content, 10);
        holder_layout->addStretch(1);

        auto* scroll = new QScrollArea(this);
        scroll->setWidget(holder);
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        return scroll;
    }

    // A large target: icon, name, what it does, and one line of live detail.
    // A button underneath, so it takes focus, answers Space and Enter, and is
    // announced as one control rather than four labels.
    QPushButton* build_feature_tile(const HomeFeature& feature, QWidget* parent) {
        auto* tile = new QPushButton(parent);
        tile->setObjectName("tile");
        tile->setCursor(Qt::PointingHandCursor);
        tile->setMinimumHeight(112);
        tile->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        tile->setAccessibleName(feature.title);
        tile->setAccessibleDescription(feature.description);
        tile->setToolTip(QString("Open %1 (%2)").arg(feature.title, feature.shortcut));

        auto* row = new QHBoxLayout(tile);
        row->setContentsMargins(18, 16, 14, 16);
        row->setSpacing(14);

        auto* icon = new QLabel(tile);
        icon->setPixmap(feature_icon(feature.glyph, feature.top, feature.bottom, 44));
        icon->setFixedSize(44, 44);
        row->addWidget(icon, 0, Qt::AlignTop);

        auto* text = new QVBoxLayout();
        text->setSpacing(3);
        auto* name = new QLabel(feature.title, tile);
        name->setObjectName("tileTitle");
        text->addWidget(name);
        auto* body = new QLabel(feature.description, tile);
        body->setObjectName("tileBody");
        body->setWordWrap(true);
        text->addWidget(body);
        auto* meta = new QLabel(feature.meta, tile);
        meta->setObjectName("tileMeta");
        text->addSpacing(2);
        text->addWidget(meta);
        text->addStretch();
        row->addLayout(text, 1);
        if (feature.meta_label != nullptr) {
            *feature.meta_label = meta;
        }

        auto* chevron = new QLabel("\u203a", tile);
        chevron->setObjectName("chevron");
        row->addWidget(chevron, 0, Qt::AlignVCenter);

        // Children never take the mouse, so hover and press belong to the tile.
        for (QLabel* label : {icon, name, body, meta, chevron}) {
            label->setAttribute(Qt::WA_TransparentForMouseEvents);
        }
        connect(tile, &QPushButton::clicked, this, feature.open);
        home_tiles_.push_back(tile);
        return tile;
    }

    void arrange_tiles(int columns) {
        if (columns == home_columns_) {
            return;
        }
        home_columns_ = columns;
        for (QPushButton* tile : home_tiles_) {
            home_grid_->removeWidget(tile);
        }
        for (int column = 0; column < 2; ++column) {
            home_grid_->setColumnStretch(column, column < columns ? 1 : 0);
        }
        for (std::size_t i = 0; i < home_tiles_.size(); ++i) {
            const int index = static_cast<int>(i);
            home_grid_->addWidget(home_tiles_[i], index / columns, index % columns);
        }
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
        if (watched == home_content_ && event->type() == QEvent::Resize) {
            constexpr int two_column_width = 700;
            arrange_tiles(home_content_->width() >= two_column_width ? 2 : 1);
        }
        return QMainWindow::eventFilter(watched, event);
    }

    void show_home() {
        if (ui_state_is_busy(state_)) {
            return;
        }
        stop_playback_if_running();
        refresh_home();
        stack_->setCurrentIndex(home_page);
        home_action_->setChecked(true);
        context_label_->setText("Ready");
        apply_state(state_);
    }

    // Read from disk each time Home is shown: the counts and the recent list
    // change on other screens, and a stale landing page is worse than a re-read.
    void refresh_home() {
        home_greeting_->setText(greeting_for_hour(QTime::currentTime().hour()));

        const fs::path root(data_directory_.toStdString());
        const std::vector<TranscriptEntry> transcripts = list_transcripts(root);
        const std::size_t recordings = list_recordings(root).size();
        home_transcripts_meta_->setText(
            QString::fromStdString(count_phrase(transcripts.size(), "transcript", "transcripts")) + " saved");
        home_speeches_meta_->setText(
            QString::fromStdString(count_phrase(recordings, "recording", "recordings")) + " kept");

        while (QLayoutItem* item = recent_layout_->takeAt(0)) {
            if (QWidget* widget = item->widget()) {
                widget->hide();
                widget->deleteLater();
            }
            delete item;
        }
        see_all_->setVisible(!transcripts.empty());

        if (transcripts.empty()) {
            // An empty state says what will appear and how to make it appear.
            auto* empty = new QWidget(recent_group_);
            auto* empty_layout = new QVBoxLayout(empty);
            empty_layout->setContentsMargins(16, 22, 16, 18);
            empty_layout->setSpacing(4);
            auto* note = new QLabel("No transcripts yet", empty);
            note->setObjectName("tileTitle");
            note->setAlignment(Qt::AlignCenter);
            empty_layout->addWidget(note);
            auto* hint = new QLabel("What you dictate on the Recording screen is saved and listed here.", empty);
            hint->setObjectName("muted");
            hint->setAlignment(Qt::AlignCenter);
            hint->setWordWrap(true);
            empty_layout->addWidget(hint);
            auto* start = new QPushButton("Start a Recording", empty);
            start->setObjectName("link");
            start->setCursor(Qt::PointingHandCursor);
            connect(start, &QPushButton::clicked, this, [this] { show_recorder(); });
            empty_layout->addWidget(start, 0, Qt::AlignHCenter);
            recent_layout_->addWidget(empty);
            return;
        }

        constexpr std::size_t shown = 4;
        for (std::size_t i = 0; i < std::min(shown, transcripts.size()); ++i) {
            if (i > 0) {
                auto* line = new QFrame(recent_group_);
                line->setObjectName("hairline");
                recent_layout_->addWidget(line);
            }
            recent_layout_->addWidget(build_recent_row(transcripts[i]));
        }
    }

    QPushButton* build_recent_row(const TranscriptEntry& entry) {
        auto* row = new QPushButton(recent_group_);
        row->setObjectName("recentRow");
        row->setCursor(Qt::PointingHandCursor);
        row->setMinimumHeight(46);
        const QString preview = entry.preview.empty() ? QString("Empty transcript")
                                                      : QString::fromStdString(entry.preview);
        const QString when = format_when(entry.stamp.when);
        row->setAccessibleName(QString("%1, %2").arg(when, preview));
        row->setToolTip(preview);

        auto* layout = new QHBoxLayout(row);
        layout->setContentsMargins(16, 10, 12, 10);
        layout->setSpacing(12);
        auto* text = new QLabel(row);
        text->setObjectName("recentText");
        // Elided here because a QLabel will not: unbroken text would otherwise
        // widen the whole page.
        text->setText(text->fontMetrics().elidedText(preview, Qt::ElideRight, 560));
        text->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        layout->addWidget(text, 1);
        auto* date = new QLabel(when, row);
        date->setObjectName("recentDate");
        layout->addWidget(date);
        auto* chevron = new QLabel("\u203a", row);
        chevron->setObjectName("chevron");
        layout->addWidget(chevron);
        for (QLabel* label : {text, date, chevron}) {
            label->setAttribute(Qt::WA_TransparentForMouseEvents);
        }

        const fs::path path = entry.path;
        connect(row, &QPushButton::clicked, this, [this, path] { open_transcript(path); });
        return row;
    }

    static QString format_when(const std::tm& when) {
        const QDate date(when.tm_year + 1900, when.tm_mon + 1, when.tm_mday);
        const QDate today = QDate::currentDate();
        const QString day = QString::fromUtf8(relative_day(date.daysTo(today)));
        if (!day.isEmpty()) {
            return QString("%1, %2").arg(day, QTime(when.tm_hour, when.tm_min).toString("HH:mm"));
        }
        return QLocale().toString(date, date.year() == today.year() ? "d MMM" : "d MMM yyyy");
    }

    // Opens the library with this transcript already selected and showing.
    void open_transcript(const fs::path& path) {
        show_library();
        if (stack_->currentIndex() != library_page) {
            return;
        }
        for (std::size_t i = 0; i < entries_.size(); ++i) {
            if (entries_[i].path == path) {
                history_list_->setCurrentRow(static_cast<int>(i));
                return;
            }
        }
    }

    // ------------------------------------------------------------ practice

    // A sentence to read, one button, the result word by word, and how it
    // works. Scrollable, so a short window loses nothing.
    QWidget* build_practice_page() {
        auto* content = new QWidget(this);
        content->setMaximumWidth(820);
        content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        auto* layout = new QVBoxLayout(content);
        layout->setContentsMargins(24, 28, 24, 24);
        layout->setSpacing(16);

        auto* title = new QLabel("Practise your pronunciation", content);
        title->setObjectName("heroTitle");
        title->setAlignment(Qt::AlignCenter);
        layout->addWidget(title);
        auto* subtitle = new QLabel("Read a sentence aloud and see which words came through, "
                                    "which changed, and which were missed.", content);
        subtitle->setObjectName("muted");
        subtitle->setWordWrap(true);
        subtitle->setAlignment(Qt::AlignCenter);
        layout->addWidget(subtitle);

        // ---- The sentence to read.
        auto* card = new QFrame(content);
        card->setObjectName("card");
        auto* card_layout = new QVBoxLayout(card);
        card_layout->setContentsMargins(24, 18, 24, 18);
        card_layout->setSpacing(12);

        auto* card_header = new QHBoxLayout();
        auto* sound_label = new QLabel("Sound:", card);
        sound_label->setObjectName("muted");
        sound_selector_ = new QComboBox(card);
        sound_selector_->addItem("All sounds", QString());
        for (const std::string& sound : practice_sounds()) {
            sound_selector_->addItem(QString::fromStdString(sound), QString::fromStdString(sound));
        }
        sound_selector_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        sound_selector_->setAccessibleName("Sound to practise");
        own_sentence_ = new QPushButton("Type My Own", card);
        own_sentence_->setCheckable(true);
        card_header->addWidget(sound_label);
        card_header->addWidget(sound_selector_);
        card_header->addStretch();
        card_header->addWidget(own_sentence_);
        card_layout->addLayout(card_header);

        sentence_label_ = new QLabel(card);
        sentence_label_->setObjectName("sentence");
        sentence_label_->setWordWrap(true);
        sentence_label_->setAlignment(Qt::AlignCenter);
        sentence_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);
        sentence_label_->setMinimumHeight(72);
        sentence_label_->setAccessibleName("Sentence to read");
        card_layout->addWidget(sentence_label_);

        custom_sentence_ = new QPlainTextEdit(card);
        custom_sentence_->setPlaceholderText("Type or paste the sentence you want to practise.");
        custom_sentence_->setMaximumHeight(96);
        custom_sentence_->setAccessibleName("Your own sentence");
        custom_sentence_->hide();
        card_layout->addWidget(custom_sentence_);

        next_sentence_ = new QPushButton("Another Sentence", card);
        next_sentence_->setCursor(Qt::PointingHandCursor);
        auto* next_row = new QHBoxLayout();
        next_row->addStretch();
        next_row->addWidget(next_sentence_);
        next_row->addStretch();
        card_layout->addLayout(next_row);
        layout->addWidget(card);

        // ---- The one action.
        practice_button_ = new QPushButton(content);
        practice_button_->setObjectName("mic");
        practice_button_->setCursor(Qt::PointingHandCursor);
        practice_button_->setMinimumSize(280, 56);
        practice_button_->setAccessibleName("Start or stop speaking");
        auto* mic_row = new QHBoxLayout();
        mic_row->addStretch();
        mic_row->addWidget(practice_button_);
        mic_row->addStretch();
        layout->addLayout(mic_row);

        practice_status_ = new QLabel(practice_instruction, content);
        practice_status_->setObjectName("muted");
        practice_status_->setAlignment(Qt::AlignCenter);
        practice_status_->setWordWrap(true);
        layout->addWidget(practice_status_);

        // ---- The result, hidden until there is one.
        result_card_ = new QFrame(content);
        result_card_->setObjectName("card");
        auto* result_layout = new QVBoxLayout(result_card_);
        result_layout->setContentsMargins(24, 18, 24, 18);
        result_layout->setSpacing(10);

        score_label_ = new QLabel(result_card_);
        score_label_->setObjectName("score");
        result_layout->addWidget(score_label_);

        result_words_ = new QLabel(result_card_);
        result_words_->setObjectName("resultWords");
        result_words_->setWordWrap(true);
        result_words_->setTextFormat(Qt::RichText);
        result_words_->setAccessibleName("Your sentence, word by word");
        result_layout->addWidget(result_words_);

        result_hints_ = new QLabel(result_card_);
        result_hints_->setWordWrap(true);
        result_hints_->setTextFormat(Qt::RichText);
        result_layout->addWidget(result_hints_);

        heard_label_ = new QLabel(result_card_);
        heard_label_->setObjectName("muted");
        heard_label_->setWordWrap(true);
        heard_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);
        result_layout->addWidget(heard_label_);

        // Said plainly, because the tool cannot do better than its recogniser.
        auto* caveat = new QLabel(
            "Whisper listens for whole words and tends to write the word you meant. A word marked "
            "check is one it was unsure of: say it again, slowly.", result_card_);
        caveat->setObjectName("muted");
        caveat->setWordWrap(true);
        result_layout->addWidget(caveat);

        QStringList legend;
        const std::pair<WordMark, const char*> legend_items[] = {
            {WordMark::Correct, "right"}, {WordMark::Substituted, "changed"},
            {WordMark::Missed, "missed"}, {WordMark::Unclear, "check"}, {WordMark::Extra, "extra"}};
        for (const auto& [mark, label] : legend_items) {
            legend << QString("<span style='color:%1'>■</span> %2").arg(mark_colour(mark), label);
        }
        auto* legend_label = new QLabel(legend.join("&nbsp;&nbsp;&nbsp;"), result_card_);
        legend_label->setObjectName("muted");
        legend_label->setTextFormat(Qt::RichText);
        try_again_ = new QPushButton("Try Again", result_card_);
        next_after_result_ = new QPushButton("Next Sentence", result_card_);
        auto* result_footer = new QHBoxLayout();
        result_footer->addWidget(legend_label);
        result_footer->addStretch();
        result_footer->addWidget(try_again_);
        result_footer->addWidget(next_after_result_);
        result_layout->addLayout(result_footer);
        result_card_->hide();
        layout->addWidget(result_card_);

        // ---- How it works, on the screen rather than behind Help.
        auto* how_title = new QLabel("How it works", content);
        how_title->setObjectName("sectionTitle");
        layout->addSpacing(8);
        layout->addWidget(how_title);
        auto* steps = new QHBoxLayout();
        steps->setSpacing(12);
        const std::pair<const char*, const char*> step_text[] = {
            {"Read the sentence aloud",
             "Pick one by the sound you want to work on, or type your own. You have 15 seconds."},
            {"It listens on this computer",
             "Whisper transcribes offline. Practice attempts are not saved."},
            {"See what it heard",
             "Each word is marked right, changed, missed, or worth checking again."}};
        int number = 1;
        for (const auto& [heading, body] : step_text) {
            auto* step = new QFrame(content);
            step->setObjectName("step");
            auto* step_layout = new QVBoxLayout(step);
            step_layout->setContentsMargins(16, 14, 16, 14);
            step_layout->setSpacing(6);
            auto* badge = new QLabel(QString::number(number++), step);
            badge->setObjectName("badge");
            badge->setFixedSize(28, 28);
            badge->setAlignment(Qt::AlignCenter);
            step_layout->addWidget(badge);
            auto* step_heading = new QLabel(heading, step);
            QFont heading_font = step_heading->font();
            heading_font.setBold(true);
            step_heading->setFont(heading_font);
            step_heading->setWordWrap(true);
            step_layout->addWidget(step_heading);
            auto* step_body = new QLabel(body, step);
            step_body->setObjectName("muted");
            step_body->setWordWrap(true);
            step_layout->addWidget(step_body);
            step_layout->addStretch();
            steps->addWidget(step, 1);
        }
        layout->addLayout(steps);

        // Free recording is one click away, not a separate application.
        go_record_button_ = new QPushButton("Record freely instead →", content);
        go_record_button_->setObjectName("link");
        go_record_button_->setCursor(Qt::PointingHandCursor);
        auto* go_row = new QHBoxLayout();
        go_row->addStretch();
        go_row->addWidget(go_record_button_);
        go_row->addStretch();
        layout->addLayout(go_row);
        layout->addStretch();

        connect(practice_button_, &QPushButton::clicked, this, [this] {
            if (state_ == UiState::Recording) {
                stop_recording();
            } else if (ui_state_is_busy(state_)) {
                cancel_work();
            } else {
                start_practice();
            }
        });
        connect(next_sentence_, &QPushButton::clicked, this, [this] { next_sentence(); });
        connect(next_after_result_, &QPushButton::clicked, this, [this] { next_sentence(); });
        connect(try_again_, &QPushButton::clicked, this, [this] { start_practice(); });
        connect(sound_selector_, qOverload<int>(&QComboBox::currentIndexChanged), this,
                [this](int) { next_sentence(); });
        connect(own_sentence_, &QPushButton::toggled, this, [this](bool own) { use_own_sentence(own); });

        auto* holder = new QWidget(this);
        holder->setObjectName("page");
        auto* holder_layout = new QHBoxLayout(holder);
        holder_layout->setContentsMargins(0, 0, 0, 0);
        holder_layout->addStretch(1);
        holder_layout->addWidget(content, 10);
        holder_layout->addStretch(1);

        auto* scroll = new QScrollArea(this);
        scroll->setWidget(holder);
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        return scroll;
    }

    // A different sentence from the chosen sound, never the one just read.
    void next_sentence() {
        const QString sound = sound_selector_->currentData().toString();
        std::vector<const PracticeSentence*> choices;
        for (const PracticeSentence& sentence : practice_sentences()) {
            const bool matches = sound.isEmpty() || sound == QString::fromUtf8(sentence.sound);
            if (matches && QString::fromUtf8(sentence.text) != current_sentence_) {
                choices.push_back(&sentence);
            }
        }
        if (choices.empty()) {
            return;
        }
        const int pick = QRandomGenerator::global()->bounded(static_cast<int>(choices.size()));
        current_sentence_ = QString::fromUtf8(choices[static_cast<std::size_t>(pick)]->text);
        sentence_label_->setText(current_sentence_);
        result_card_->hide();
        if (!ui_state_is_busy(state_)) {
            practice_status_->setText(practice_instruction);
        }
    }

    void use_own_sentence(bool own) {
        sentence_label_->setVisible(!own);
        next_sentence_->setVisible(!own);
        custom_sentence_->setVisible(own);
        sound_selector_->setEnabled(!own);
        own_sentence_->setText(own ? "Use Built-in Sentences" : "Type My Own");
        result_card_->hide();
        if (own) {
            custom_sentence_->setFocus();
        }
    }

    QString practice_sentence() const {
        return own_sentence_->isChecked() ? custom_sentence_->toPlainText().simplified() : current_sentence_;
    }

    void start_practice() {
        if (!ui_state_may_start(state_)) {
            return;
        }
        const QString target = practice_sentence();
        if (practice_words(target.toStdString()).empty()) {
            practice_status_->setText("Type a sentence to read first.");
            custom_sentence_->setFocus();
            return;
        }
        practice_target_ = target;
        heard_words_.clear();
        result_card_->hide();
        start_recording(true);
    }

    // WORDS|word:probability|... The word is everything before the last colon.
    void handle_words_line(const QString& line) {
        heard_words_.clear();
        const QStringList fields = line.split('|');
        for (int i = 1; i < fields.size(); ++i) {
            const int colon = fields.at(i).lastIndexOf(':');
            if (colon <= 0) {
                continue;
            }
            heard_words_.push_back({fields.at(i).left(colon).toStdString(), fields.at(i).mid(colon + 1).toFloat()});
        }
    }

    void render_practice_result() {
        const std::vector<WordResult> results = compare_words(practice_target_.toStdString(), heard_words_);
        const std::size_t correct = correct_word_count(results);
        const std::size_t total = target_word_count(results);

        QStringList words;
        QStringList hints;
        for (const WordResult& result : results) {
            const QString colour = mark_colour(result.mark);
            const QString expected = QString::fromStdString(result.shown).toHtmlEscaped();
            const QString heard = QString::fromStdString(result.heard).toHtmlEscaped();
            switch (result.mark) {
                case WordMark::Correct:
                    words << QString("<span style='color:%1'>%2</span>").arg(colour, expected);
                    break;
                case WordMark::Unclear:
                    words << QString("<span style='color:%1; text-decoration:underline'>%2</span>")
                                 .arg(colour, expected);
                    hints << QString("Check %1").arg(quoted(result.shown));
                    break;
                case WordMark::Substituted:
                    words << QString("<span style='color:%1; text-decoration:line-through'>%2</span> "
                                     "<span style='color:%1; font-weight:700'>%3</span>")
                                 .arg(colour, expected, heard);
                    hints << QString("Heard %1 for %2").arg(quoted(result.heard), quoted(result.shown));
                    break;
                case WordMark::Missed:
                    words << QString("<span style='color:%1; text-decoration:line-through'>%2</span>")
                                 .arg(colour, expected);
                    hints << QString("%1 was not heard").arg(quoted(result.shown));
                    break;
                case WordMark::Extra:
                    words << QString("<span style='color:%1; font-style:italic'>+%2</span>").arg(colour, heard);
                    hints << QString("Extra word %1").arg(quoted(result.heard));
                    break;
            }
        }

        score_label_->setText(correct == total
            ? QString("All %1 words came through clearly.").arg(total)
            : QString("%1 of %2 words came through clearly").arg(correct).arg(total));
        result_words_->setText(words.join(' '));
        constexpr int maximum_hints = 5;
        if (hints.size() > maximum_hints) {
            const int more = static_cast<int>(hints.size()) - maximum_hints;
            hints = hints.mid(0, maximum_hints);
            hints << QString("%1 more").arg(more);
        }
        result_hints_->setText(hints.join("&nbsp;&nbsp;·&nbsp;&nbsp;"));
        result_hints_->setVisible(!hints.isEmpty());
        heard_label_->setText(final_transcription_.isEmpty()
            ? QString("Whisper heard nothing it could write down.")
            : QString("Whisper heard: “%1”").arg(final_transcription_));
        result_card_->show();
    }

    void set_status(const QString& status) {
        status_label_->setText(status);
        if (practice_mode_) {
            practice_status_->setText(status);
        }
    }

    void set_idle(const QString& status) {
        timer_->stop();
        limit_timer_->stop();
        apply_state(completed_ ? UiState::Completed : UiState::Ready);
        set_status(status);
    }

    void set_failed(const QString& status) {
        timer_->stop();
        limit_timer_->stop();
        apply_state(UiState::Error);
        set_status(status);
    }

    QComboBox* model_selector_;
    QComboBox* duration_selector_;
    QComboBox* device_selector_;
    QPushButton* start_button_;
    QPushButton* stop_button_;
    QPushButton* save_button_;
    QPushButton* copy_button_;
    QLabel* status_label_;
    QLabel* duration_label_;
    QLabel* latency_label_;
    QLabel* indicator_;
    QLabel* transcript_title_;
    QListWidget* history_list_;
    QStackedWidget* stack_;
    QPushButton* go_record_button_;
    QAction* home_action_;
    std::vector<QPushButton*> home_tiles_;
    QLabel* home_greeting_;
    QLabel* home_transcripts_meta_;
    QLabel* home_speeches_meta_;
    QGridLayout* home_grid_;
    QWidget* home_content_ = nullptr;
    int home_columns_ = 0;
    QVBoxLayout* recent_layout_;
    QFrame* recent_group_;
    QPushButton* see_all_;
    QPushButton* delete_all_button_;
    QPlainTextEdit* saved_view_;
    QLabel* saved_title_;
    QPushButton* saved_save_;
    QPushButton* saved_copy_;
    QPushButton* delete_saved_;
    QListWidget* speech_list_;
    QLabel* now_playing_;
    QLabel* play_time_;
    QPushButton* play_button_;
    QPushButton* stop_play_button_;
    QPushButton* delete_speech_;
    QLabel* recording_details_;
    QSlider* position_slider_;
    std::vector<RecordingEntry> recordings_;
    AudioPlayer player_;
    QAction* history_action_;
    std::vector<TranscriptEntry> entries_;
    QLabel* context_label_;
    QString model_summary_;
    QString input_summary_;
    QString storage_summary_;
    QAction* practice_action_;
    QAction* speeches_action_;
    QAction* recorder_action_;
    QAction* record_action_;
    QAction* cancel_action_;
    QProgressBar* progress_;
    QCheckBox* keep_audio_;
    QString data_directory_;
    QPlainTextEdit* transcript_;
    QProcess* process_;
    QTimer* timer_;
    QTimer* limit_timer_;
    QTimer* processing_timer_;
    QTimer* play_timer_;
    QElapsedTimer elapsed_;
    UiState state_ = UiState::Ready;
    int limit_seconds_ = 0;
    bool devices_ready_ = false;
    QVariant restore_device_;
    QString output_buffer_;
    QString final_transcription_;
    QStringList partial_words_;
    bool partial_trimmed_ = false;
    static constexpr int maximum_partial_words = 3000;
    static constexpr int maximum_overlap_words = 40;
    QString transcript_path_;
    bool completed_ = false;
    bool closing_ = false;
    bool error_shown_ = false;
    bool cancelled_ = false;
    int chunks_done_ = 0;
    int chunks_total_ = 0;
    QElapsedTimer processing_elapsed_;

    QComboBox* sound_selector_;
    QLabel* sentence_label_;
    QPlainTextEdit* custom_sentence_;
    QPushButton* next_sentence_;
    QPushButton* own_sentence_;
    QPushButton* practice_button_;
    QLabel* practice_status_;
    QFrame* result_card_;
    QLabel* score_label_;
    QLabel* result_words_;
    QLabel* result_hints_;
    QLabel* heard_label_;
    QPushButton* try_again_;
    QPushButton* next_after_result_;
    QString current_sentence_;
    QString practice_target_;
    std::vector<HeardWord> heard_words_;
    bool practice_mode_ = false;
    // Long enough for any built-in sentence read slowly, short enough that a
    // forgotten Stop does not leave the learner waiting.
    static constexpr int practice_limit_seconds = 15;
    static constexpr const char* practice_instruction =
        "Press Start Speaking, read the sentence aloud, then press Stop and Check.";
};

int main(int argc, char* argv[]) {
    QApplication application(argc, argv);
    // Fusion draws the same on every platform, which is what lets one style
    // sheet look right on both Linux and Windows.
    application.setStyle(QStyleFactory::create("Fusion"));
    application.setPalette(application_palette());
    application.setStyleSheet(application_style());
    // Set on the application rather than the window, so message boxes and file
    // dialogs carry it too. On Linux a desktop environment matches a running
    // window to its desktop entry by this name, which is how the launcher and
    // the taskbar end up showing the same icon.
    application.setDesktopFileName("eco-escrito");
    application.setWindowIcon(QIcon(":/icons/eco-escrito-512.png"));
    EcoEscritoWindow window;
    window.show();
    return application.exec();
}
