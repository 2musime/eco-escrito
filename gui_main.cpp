#include "audio_player.h"
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
#include <QDesktopServices>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QFile>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
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
#include <QUrl>
#include <QVBoxLayout>

#include <algorithm>

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
QMainWindow, QWidget#page { background: #ffffff; }
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
QLabel#muted { color: #6e6e73; }
QLabel#sentence { font-size: 30px; font-weight: 600; color: #1d1d1f; }
QLabel#clock { font-size: 44px; font-weight: 300; color: #1d1d1f; }
QLabel#readerTitle { font-size: 17px; font-weight: 600; color: #1d1d1f; }
QLabel#score { font-size: 15px; font-weight: 600; color: #1d1d1f; }
QLabel#resultWords { font-size: 22px; }
QFrame#card { background: #f5f5f7; border: none; border-radius: 14px; }
QPushButton { background: #ffffff; color: #1d1d1f; border: 1px solid #d2d2d7; border-radius: 8px;
              padding: 6px 14px; }
QPushButton:hover { background: #f2f2f5; }
QPushButton:pressed { background: #e8e8ed; }
QPushButton:checked { background: #e8f1ff; border-color: #99c7ff; color: #0062cc; }
QPushButton:disabled { color: #aeaeb2; background: #f5f5f7; border-color: #e5e5ea; }
QPushButton#mic { background: #007aff; color: #ffffff; border: none; border-radius: 26px;
                  font-size: 15px; font-weight: 600; padding: 0 28px; }
QPushButton#mic:hover { background: #0062cc; }
QPushButton#mic[recording="true"] { background: #ff3b30; }
QPushButton#mic[recording="true"]:hover { background: #d70015; }
QPushButton#mic:disabled { background: #80bdff; color: #f0f7ff; }
QPushButton#quiet, QPushButton#destructive { border: none; background: transparent; color: #007aff;
                                             padding: 4px 8px; }
QPushButton#quiet:hover, QPushButton#destructive:hover { background: #f2f2f5; }
QPushButton#quiet:checked { background: #e8f1ff; color: #0062cc; }
QPushButton#quiet:disabled, QPushButton#destructive:disabled { background: transparent; color: #c7c7cc; }
QPushButton#destructive { color: #ff3b30; }
QPlainTextEdit, QListWidget { background: #ffffff; border: 1px solid #d2d2d7; border-radius: 10px; }
QPlainTextEdit#sheet { background: #f5f5f7; border: none; border-radius: 12px; }
QListWidget#sidebar { background: transparent; border: none; border-right: 1px solid #e5e5ea;
                      border-radius: 0; }
QPlainTextEdit#reader { background: #ffffff; border: none; font-size: 15px; }
QPlainTextEdit#sheet { font-size: 15px; }
QListWidget#sidebar { padding: 0 12px 0 0; outline: none; }
QListWidget::item { padding: 8px 10px; }
QListWidget#sidebar::item { padding: 9px 12px; border-radius: 8px; color: #1d1d1f; }
QListWidget#sidebar::item:hover { background: #ececf0; }
QListWidget::item:selected, QListWidget#sidebar::item:selected { background: #007aff; color: #ffffff; }
QListWidget#sidebar::item:disabled { color: #8e8e93; }
QComboBox { background: #ffffff; border: 1px solid #d2d2d7; border-radius: 8px; padding: 4px 10px;
            color: #1d1d1f; }
QComboBox:hover { border-color: #aeaeb2; }
QComboBox::drop-down { border: none; width: 22px; }
QComboBox::down-arrow { image: url(:/icons/chevron-down.png); width: 10px; height: 6px; }
QComboBox:disabled { color: #aeaeb2; border-color: #e5e5ea; }
QStatusBar { background: #ffffff; border-top: 1px solid #e5e5ea; color: #6e6e73; }
QStatusBar QLabel { color: #6e6e73; padding: 0 8px; }
QProgressBar { background: #e5e5ea; border: none; border-radius: 2px; }
QProgressBar::chunk { background: #007aff; border-radius: 2px; }
QScrollArea { background: transparent; border: none; }
QToolButton#brand { border: none; background: transparent; font-size: 15px; font-weight: 700; color: #1d1d1f;
                    padding: 4px 6px; }
QToolButton#brand:hover { color: #000000; }
QLabel#welcomeTitle { font-size: 32px; font-weight: 600; color: #1d1d1f; }
QPushButton#welcomePrimary { background: #007aff; color: #ffffff; border: none; border-radius: 10px;
                             font-weight: 600; padding: 0 22px; }
QPushButton#welcomePrimary:hover { background: #0062cc; }
QPushButton#welcomeSecondary { border-radius: 10px; padding: 0 22px; }
QStatusBar::item { border: none; }
QPushButton#footerLink { border: none; background: transparent; color: #6e6e73; text-decoration: underline;
                         padding: 0 6px; }
QPushButton#footerLink:hover { color: #1d1d1f; }
QPushButton#starLink { border: 1px solid #e5e5ea; border-radius: 10px; background: transparent; color: #424245;
                       padding: 1px 10px; margin: 0 6px; }
QPushButton#starLink:hover { border-color: #ffcc00; background: #fffbea; color: #1d1d1f; }
QLabel#footnote { font-size: 12px; color: #8e8e93; padding: 0 8px; }
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
        // The version is in the footer; the title bar carries the name alone.
        setWindowTitle("Eco-escrito");
        resize(1040, 720);
        // Below this the settings row wraps and the transcript stops being usable.
        setMinimumSize(640, 420);

        build_menus();
        // The navigation sets the floor: narrower, and its labels would be cut.
        setMinimumWidth(std::max(minimumWidth(), menuWidget()->sizeHint().width()));

        // ---- Recording. The clock and one button are the screen; settings sit
        // quietly above them and the transcript fills what is left.
        auto* recorder = new QWidget(this);
        auto* layout = new QVBoxLayout(recorder);
        layout->setContentsMargins(24, 28, 24, 20);
        layout->setSpacing(0);

        // Self-describing values, so the row needs no labels.
        model_selector_ = new QComboBox(recorder);
        // base.en first, so it is the default: it transcribes roughly 3.5x
        // faster than small.en, which matters most on long recordings.
        // A name, not a path. The worker searches its approved directories for
        // it, which is the only thing that works when the application is
        // started from a menu entry and inherits no useful working directory.
        model_selector_->addItem("Speed: base.en", "ggml-base.en.bin");
        model_selector_->addItem("Accuracy: small.en", "ggml-small.en.bin");
        model_selector_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        model_selector_->setAccessibleName("Model");

        device_selector_ = new QComboBox(recorder);
        device_selector_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        device_selector_->setMinimumContentsLength(18);
        device_selector_->setAccessibleName("Microphone");

        duration_selector_ = new QComboBox(recorder);
        duration_selector_->addItem("15 seconds", 15);
        duration_selector_->addItem("45 seconds", 45);
        duration_selector_->addItem("60 seconds", 60);
        duration_selector_->addItem("5 minutes", 300);
        duration_selector_->addItem("10 minutes", 600);
        duration_selector_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        duration_selector_->setAccessibleName("Recording limit");

        keep_audio_ = new QCheckBox("Keep audio", recorder);
        keep_audio_->setChecked(true);

        auto* settings = new QHBoxLayout();
        settings->setSpacing(8);
        settings->addStretch();
        settings->addWidget(model_selector_);
        settings->addWidget(device_selector_);
        settings->addWidget(duration_selector_);
        settings->addSpacing(8);
        settings->addWidget(keep_audio_);
        settings->addStretch();
        layout->addLayout(settings);
        layout->addSpacing(36);

        duration_label_ = new QLabel(recorder);
        duration_label_->setObjectName("clock");
        duration_label_->setAlignment(Qt::AlignCenter);
        layout->addWidget(duration_label_);
        layout->addSpacing(20);

        // One button: start, then stop, then cancel while the worker is busy.
        start_button_ = new QPushButton(recorder);
        start_button_->setObjectName("mic");
        start_button_->setCursor(Qt::PointingHandCursor);
        start_button_->setMinimumSize(240, 52);
        start_button_->setAccessibleName("Start or stop recording");
        layout->addWidget(start_button_, 0, Qt::AlignHCenter);
        layout->addSpacing(14);

        // A coloured dot reads at a glance; a word has to be found and parsed.
        indicator_ = new QLabel("●", recorder);
        status_label_ = new QLabel("Ready", recorder);
        status_label_->setObjectName("muted");
        auto* status_row = new QHBoxLayout();
        status_row->setSpacing(6);
        status_row->addStretch();
        status_row->addWidget(indicator_);
        status_row->addWidget(status_label_);
        status_row->addStretch();
        layout->addLayout(status_row);

        progress_ = new QProgressBar(recorder);
        progress_->setTextVisible(false);
        progress_->setRange(0, 100);
        progress_->setValue(0);
        progress_->setFixedSize(240, 4);
        progress_->hide();
        layout->addSpacing(8);
        layout->addWidget(progress_, 0, Qt::AlignHCenter);

        latency_label_ = new QLabel("", recorder);
        latency_label_->setObjectName("footnote");
        latency_label_->setAlignment(Qt::AlignCenter);
        layout->addWidget(latency_label_);
        layout->addSpacing(20);

        // ---- The transcript, and the two things to do with it.
        auto* transcript_header = new QHBoxLayout();
        transcript_header->addStretch();
        copy_button_ = new QPushButton("Copy", recorder);
        save_button_ = new QPushButton("Save", recorder);
        for (QPushButton* button : {copy_button_, save_button_}) {
            button->setObjectName("quiet");
            button->setCursor(Qt::PointingHandCursor);
            button->setEnabled(false);
            transcript_header->addWidget(button);
        }
        save_button_->setShortcut(QKeySequence::Save);
        copy_button_->setShortcut(QKeySequence("Ctrl+Shift+C"));
        layout->addLayout(transcript_header);
        layout->addSpacing(4);

        transcript_ = new QPlainTextEdit(recorder);
        transcript_->setObjectName("sheet");
        transcript_->setPlaceholderText("Your words will appear here.");
        transcript_->setReadOnly(true);
        transcript_->setLineWrapMode(QPlainTextEdit::WidgetWidth);
        // Transcripts are read, not skimmed: a little more room per line helps.
        transcript_->document()->setDocumentMargin(16);
        transcript_->setAccessibleName("Transcript");
        transcript_->setAccessibleDescription("The text transcribed from your recording");
        layout->addWidget(transcript_, 1);
        QWidget* recorder_page = centred(recorder, 860);

        // ---- Transcripts. A sidebar of sessions and a reader, nothing else.
        // Recording controls are not merely disabled here, they are absent:
        // nothing on this screen can start a recording.
        history_list_ = new QListWidget(this);
        history_list_->setObjectName("sidebar");
        history_list_->setContextMenuPolicy(Qt::CustomContextMenu);
        history_list_->setAccessibleName("Saved transcripts");
        history_list_->setFixedWidth(280);
        history_list_->setTextElideMode(Qt::ElideRight);
        history_list_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        auto* library = new QWidget(this);
        auto* library_layout = new QHBoxLayout(library);
        library_layout->setContentsMargins(24, 24, 24, 20);
        library_layout->setSpacing(24);

        auto* sidebar = new QVBoxLayout();
        sidebar->setSpacing(8);
        sidebar->addWidget(history_list_, 1);
        delete_all_button_ = new QPushButton("Delete All Recordings...", library);
        delete_all_button_->setObjectName("destructive");
        delete_all_button_->setCursor(Qt::PointingHandCursor);
        delete_all_button_->setToolTip("Remove every stored recording and transcript");
        sidebar->addWidget(delete_all_button_, 0, Qt::AlignLeft);
        library_layout->addLayout(sidebar);

        auto* reader = new QWidget(library);
        auto* reader_layout = new QVBoxLayout(reader);
        reader_layout->setContentsMargins(0, 0, 0, 0);
        reader_layout->setSpacing(8);

        saved_title_ = new QLabel(reader);
        saved_title_->setObjectName("readerTitle");
        saved_copy_ = new QPushButton("Copy", reader);
        saved_save_ = new QPushButton("Save a Copy", reader);
        // Under the transcript it would remove would be safer still, but the
        // three actions read as one row, and the confirmation names the target.
        delete_saved_ = new QPushButton("Delete", reader);
        delete_saved_->setToolTip("Delete this transcript. The recording is kept.");
        auto* reader_header = new QHBoxLayout();
        reader_header->addWidget(saved_title_);
        reader_header->addStretch();
        for (QPushButton* button : {saved_copy_, saved_save_, delete_saved_}) {
            button->setObjectName(button == delete_saved_ ? "destructive" : "quiet");
            button->setCursor(Qt::PointingHandCursor);
            button->setEnabled(false);
            reader_header->addWidget(button);
        }
        reader_layout->addLayout(reader_header);

        // Its own view, so live output is never disturbed by browsing.
        saved_view_ = new QPlainTextEdit(reader);
        saved_view_->setObjectName("reader");
        saved_view_->setReadOnly(true);
        saved_view_->document()->setDocumentMargin(0);
        saved_view_->setAccessibleName("Saved transcript");
        reader_layout->addWidget(saved_view_, 1);
        library_layout->addWidget(reader, 1);
        QWidget* library_page = centred(library, 1120);
        clear_reader();

        QWidget* practice_widget = build_practice_page();

        // ---- Speeches: play back what was actually said. This is the raw
        // capture, not the cleaned copy or the extracted speech; those exist
        // for Whisper, and are not what someone wants to hear.
        speech_list_ = new QListWidget(this);
        speech_list_->setObjectName("sidebar");
        speech_list_->setFixedWidth(280);
        speech_list_->setTextElideMode(Qt::ElideRight);
        speech_list_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        speech_list_->setAccessibleName("Recordings");

        auto* speeches = new QWidget(this);
        auto* speech_layout = new QHBoxLayout(speeches);
        speech_layout->setContentsMargins(24, 24, 24, 20);
        speech_layout->setSpacing(24);
        speech_layout->addWidget(speech_list_);

        auto* player_panel = new QWidget(speeches);
        auto* player_layout = new QVBoxLayout(player_panel);
        player_layout->setContentsMargins(0, 0, 0, 0);
        player_layout->setSpacing(0);
        player_layout->addStretch(1);

        now_playing_ = new QLabel(player_panel);
        now_playing_->setObjectName("readerTitle");
        now_playing_->setAlignment(Qt::AlignCenter);
        player_layout->addWidget(now_playing_);
        player_layout->addSpacing(6);
        recording_details_ = new QLabel(player_panel);
        recording_details_->setObjectName("muted");
        recording_details_->setTextInteractionFlags(Qt::TextSelectableByMouse);
        recording_details_->setWordWrap(true);
        recording_details_->setAlignment(Qt::AlignCenter);
        player_layout->addWidget(recording_details_);
        player_layout->addSpacing(28);

        play_time_ = new QLabel("00:00 / 00:00", player_panel);
        play_time_->setObjectName("clock");
        play_time_->setAlignment(Qt::AlignCenter);
        player_layout->addWidget(play_time_);
        player_layout->addSpacing(16);

        position_slider_ = new QSlider(Qt::Horizontal, player_panel);
        position_slider_->setRange(0, 0);
        position_slider_->setEnabled(false);
        position_slider_->setMaximumWidth(420);
        player_layout->addWidget(position_slider_, 0, Qt::AlignHCenter);
        position_slider_->setMinimumWidth(320);
        player_layout->addSpacing(20);

        play_button_ = new QPushButton("Play", player_panel);
        play_button_->setObjectName("mic");
        play_button_->setCursor(Qt::PointingHandCursor);
        play_button_->setMinimumSize(160, 52);
        play_button_->setEnabled(false);
        stop_play_button_ = new QPushButton("Stop", player_panel);
        stop_play_button_->setObjectName("quiet");
        stop_play_button_->setCursor(Qt::PointingHandCursor);
        stop_play_button_->setEnabled(false);
        auto* transport = new QHBoxLayout();
        transport->setSpacing(12);
        transport->addStretch();
        transport->addWidget(play_button_);
        transport->addWidget(stop_play_button_);
        transport->addStretch();
        player_layout->addLayout(transport);
        player_layout->addStretch(1);

        delete_speech_ = new QPushButton("Delete Recording", player_panel);
        delete_speech_->setObjectName("destructive");
        delete_speech_->setEnabled(false);
        delete_speech_->setCursor(Qt::PointingHandCursor);
        delete_speech_->setToolTip("Delete this recording and its processed copies. "
                                  "The transcript is kept.");
        player_layout->addWidget(delete_speech_, 0, Qt::AlignRight);
        speech_layout->addWidget(player_panel, 1);
        QWidget* speech_page = centred(speeches, 1120);

        QWidget* welcome_widget = build_welcome_page();

        // Added in Page order, which is what setCurrentIndex relies on.
        stack_ = new QStackedWidget(this);
        stack_->addWidget(welcome_widget);
        stack_->addWidget(practice_widget);
        stack_->addWidget(recorder_page);
        stack_->addWidget(library_page);
        stack_->addWidget(speech_page);
        setCentralWidget(stack_);

        // ---- Status bar: context that used to be four stacked labels.
        context_label_ = new QLabel(this);
        context_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);
        statusBar()->addWidget(context_label_, 1);
        // Who made it, said once and quietly, with the one thing a visitor might
        // want to do about it. Both open in the user's browser, and only when
        // clicked; the application never fetches anything from either.
        auto* author_link = new QPushButton(QString("Made by %1").arg(ECO_ESCRITO_AUTHOR), this);
        author_link->setObjectName("footerLink");
        author_link->setCursor(Qt::PointingHandCursor);
        author_link->setToolTip(QString("@%1 on GitHub").arg(ECO_ESCRITO_AUTHOR_HANDLE));
        connect(author_link, &QPushButton::clicked, this, [] { open_link(ECO_ESCRITO_AUTHOR_URL); });
        statusBar()->addPermanentWidget(author_link);
        auto* star_link = new QPushButton("\u2606 Star on GitHub", this);
        star_link->setObjectName("starLink");
        star_link->setCursor(Qt::PointingHandCursor);
        star_link->setToolTip("Like Eco-escrito? A star helps others find it.");
        connect(star_link, &QPushButton::clicked, this, [] { open_link(ECO_ESCRITO_HOMEPAGE_URL); });
        statusBar()->addPermanentWidget(star_link);

        auto* privacy_link = new QPushButton("Privacy", this);
        privacy_link->setObjectName("footerLink");
        privacy_link->setCursor(Qt::PointingHandCursor);
        connect(privacy_link, &QPushButton::clicked, this, [this] { show_privacy_notice(); });
        statusBar()->addPermanentWidget(privacy_link);
        auto* version = new QLabel(QString("Eco-escrito %1").arg(ECO_ESCRITO_VERSION), this);
        version->setObjectName("footnote");
        statusBar()->addPermanentWidget(version);
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

        connect(start_button_, &QPushButton::clicked, this, [this] {
            if (state_ == UiState::Recording) {
                stop_recording();
            } else if (ui_state_is_busy(state_)) {
                cancel_work();
            } else {
                start_recording(false);
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
        // Always open on the welcome page.
        show_welcome();
    }

private:
    // The screens, in the order they sit in the stack.
    enum Page { welcome_page, practice_page, recorder_page, library_page, speeches_page };

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
        brand->setToolTip("Welcome page (Ctrl+0)");
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

        // The welcome page has no tab: the name is the way back to it. Still in
        // the group, so no tab is highlighted while it shows.
        welcome_action_ = new QAction(this);
        welcome_action_->setCheckable(true);
        welcome_action_->setShortcut(QKeySequence("Ctrl+0"));
        screens->addAction(welcome_action_);
        addAction(welcome_action_);
        connect(welcome_action_, &QAction::triggered, this, [this] { show_welcome(); });
        connect(brand, &QToolButton::clicked, welcome_action_, &QAction::trigger);
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
        context_label_->setText(parts.isEmpty() ? QString("Ready")
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
        start_button_->setEnabled(controls.start || controls.stop || controls.cancel);
        start_button_->setText(controls.stop     ? "\u25a0  Stop Recording"
                               : controls.cancel ? "Cancel"
                                                 : "\u25cf  Start Recording");
        start_button_->setProperty("recording", controls.stop);
        start_button_->style()->unpolish(start_button_);
        start_button_->style()->polish(start_button_);
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
        welcome_action_->setEnabled(!ui_state_is_busy(state));
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

        start_button_->setToolTip(controls.start  ? "Begin recording (Ctrl+R)"
                                  : controls.stop ? "Stop recording and transcribe (Ctrl+R)"
                                  : controls.cancel ? "Abandon this recording (Esc)"
                                                    : busy);
        keep_audio_->setAccessibleDescription("When off, audio is transcribed and discarded.");

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
        context_label_->setText("Ready");
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
        if (speech_list_->currentRow() < 0 && !recordings_.empty()) {
            speech_list_->setCurrentRow(0);
        }
        speech_list_->setFocus();
    }

    void refresh_speeches() {
        const int previous = speech_list_->currentRow();
        const QSignalBlocker blocker(speech_list_);
        speech_list_->clear();
        recordings_ = list_recordings(fs::path(data_directory_.toStdString()));

        if (recordings_.empty()) {
            auto* empty = new QListWidgetItem("No recordings yet", speech_list_);
            empty->setFlags(Qt::NoItemFlags);
            return;
        }

        for (const RecordingEntry& entry : recordings_) {
            // When it was said, and nothing else. Length and transcript status
            // belong to the recording being played, not to the list.
            auto* item = new QListWidgetItem(friendly_stamp(entry.stamp.when), speech_list_);
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
            now_playing_->setText(recordings_.empty() ? QString("Nothing recorded yet") : QString());
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

        now_playing_->setText(friendly_stamp(entry.stamp.when));
        recording_details_->setText(QString("%1  \u00b7  %2 MB  \u00b7  %3")
            .arg(QTime(0, 0).addSecs(static_cast<int>(entry.seconds)).toString("m:ss"),
                 QString::number(entry.size_bytes / (1024.0 * 1024.0), 'f', 1),
                 entry.has_transcript ? "Transcribed" : "Not transcribed"));
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
        // Selected, not merely current, so the row being read is highlighted.
        if (history_list_->currentRow() < 0 && !entries_.empty()) {
            history_list_->setCurrentRow(0);
        }
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

    // How a session is named on screen: when it happened, as a person would say
    // it. The file name stays the record on disk and is in each row's tooltip.
    static QString friendly_stamp(const std::tm& when) {
        const QDate date(when.tm_year + 1900, when.tm_mon + 1, when.tm_mday);
        const QDate today = QDate::currentDate();
        const QString time = QTime(when.tm_hour, when.tm_min).toString("HH:mm");
        const qint64 days = date.daysTo(today);
        if (days == 0) {
            return "Today, " + time;
        }
        if (days == 1) {
            return "Yesterday, " + time;
        }
        const QString format = date.year() == today.year() ? "ddd d MMM" : "d MMM yyyy";
        return QLocale::c().toString(date, format) + ", " + time;
    }

    // Rebuilt from disk rather than tracked incrementally: the list is short,
    // and a stale list is worse than a cheap re-read.
    void refresh_history() {
        const int previous_row = history_list_->currentRow();
        const QSignalBlocker blocker(history_list_);
        history_list_->clear();
        entries_ = list_transcripts(fs::path(data_directory_.toStdString()));

        if (entries_.empty()) {
            auto* empty = new QListWidgetItem("No transcripts yet", history_list_);
            empty->setFlags(Qt::NoItemFlags);
            return;
        }

        for (const TranscriptEntry& entry : entries_) {
            // When it was said, and nothing else. What it says is one click
            // away; repeating a fragment of it on every row is noise.
            auto* item = new QListWidgetItem(friendly_stamp(entry.stamp.when), history_list_);
            item->setToolTip(QString::fromStdString(entry.path.string()));
        }
        if (previous_row >= 0 && previous_row < history_list_->count()) {
            history_list_->setCurrentRow(previous_row);
        }
    }

    void clear_reader() {
        saved_view_->clear();
        saved_title_->setText(QString());
        saved_title_->setToolTip(QString());
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

        const CompanionAudio audio =
            companion_audio(entry.stamp, fs::path(data_directory_.toStdString()));
        saved_title_->setText(friendly_stamp(entry.stamp.when));
        saved_title_->setToolTip(audio.any() ? QString() : QString("Its audio has been deleted."));
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

    // Handed to the desktop's browser. Nothing here makes a connection.
    static void open_link(const char* url) {
        QDesktopServices::openUrl(QUrl(QString::fromUtf8(url)));
    }

    void show_about() {
        QMessageBox::about(this, "About Eco-escrito",
            QString("<b>Eco-escrito %1</b><br>"
                    "Build %2, %3<br><br>"
                    "Offline speech to text. Audio never leaves this computer.<br><br>"
                    "Made by %4, who loves building useful things with technology.<br>"
                    "<a href='%5'>GitHub</a> &nbsp;\u00b7&nbsp; <a href='%6'>Instagram</a> "
                    "&nbsp;\u00b7&nbsp; <a href='%7'>Star the project</a><br><br>"
                    "Transcription by whisper.cpp, audio capture by miniaudio, "
                    "interface built with Qt 6.")
                .arg(ECO_ESCRITO_VERSION, ECO_ESCRITO_GIT_COMMIT, ECO_ESCRITO_BUILD_TYPE, ECO_ESCRITO_AUTHOR,
                     ECO_ESCRITO_AUTHOR_URL, ECO_ESCRITO_INSTAGRAM_URL, ECO_ESCRITO_HOMEPAGE_URL));
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

    // ------------------------------------------------------------- welcome

    // The icon, one line, and the two ways in. Everything else is in the bar,
    // so nothing here repeats it.
    QWidget* build_welcome_page() {
        auto* page = new QWidget(this);
        page->setObjectName("page");
        auto* layout = new QVBoxLayout(page);
        layout->setContentsMargins(24, 24, 24, 24);
        layout->setSpacing(0);
        layout->addStretch(1);

        auto* icon = new QLabel(page);
        icon->setPixmap(QIcon(":/icons/eco-escrito-512.png").pixmap(120, 120));
        icon->setAlignment(Qt::AlignCenter);
        layout->addWidget(icon);
        layout->addSpacing(28);

        auto* title = new QLabel("Your voice, in writing. On your own computer.", page);
        title->setObjectName("welcomeTitle");
        title->setAlignment(Qt::AlignCenter);
        title->setWordWrap(true);
        layout->addWidget(title);
        layout->addSpacing(28);

        auto* record = new QPushButton("New Recording", page);
        record->setObjectName("welcomePrimary");
        record->setMinimumHeight(40);
        record->setCursor(Qt::PointingHandCursor);
        record->setDefault(true);
        auto* practise = new QPushButton("Practise Pronunciation", page);
        practise->setObjectName("welcomeSecondary");
        practise->setMinimumHeight(40);
        practise->setCursor(Qt::PointingHandCursor);
        auto* actions = new QHBoxLayout();
        actions->setSpacing(12);
        actions->addStretch();
        actions->addWidget(record);
        actions->addWidget(practise);
        actions->addStretch();
        layout->addLayout(actions);
        layout->addStretch(1);

        connect(record, &QPushButton::clicked, this, [this] { show_recorder(); });
        connect(practise, &QPushButton::clicked, this, [this] { show_practice(); });
        return page;
    }

    void show_welcome() {
        if (ui_state_is_busy(state_)) {
            return;
        }
        stop_playback_if_running();
        stack_->setCurrentIndex(welcome_page);
        welcome_action_->setChecked(true);
        context_label_->setText("Ready");
        apply_state(state_);
    }

    // ------------------------------------------------------------ practice

    // The sentence is the screen: a quiet choice above it, one button below,
    // and the result word by word once there is one.
    QWidget* build_practice_page() {
        auto* content = new QWidget(this);
        auto* layout = new QVBoxLayout(content);
        layout->setContentsMargins(24, 28, 24, 24);
        layout->setSpacing(0);

        sound_selector_ = new QComboBox(content);
        sound_selector_->addItem("All sounds", QString());
        for (const std::string& sound : practice_sounds()) {
            sound_selector_->addItem(QString::fromStdString(sound), QString::fromStdString(sound));
        }
        sound_selector_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        sound_selector_->setAccessibleName("Sound to practise");
        own_sentence_ = new QPushButton("Type My Own", content);
        own_sentence_->setObjectName("quiet");
        own_sentence_->setCursor(Qt::PointingHandCursor);
        own_sentence_->setCheckable(true);
        auto* choose = new QHBoxLayout();
        choose->setSpacing(8);
        choose->addStretch();
        choose->addWidget(sound_selector_);
        choose->addWidget(own_sentence_);
        choose->addStretch();
        layout->addLayout(choose);
        layout->addStretch(1);

        sentence_label_ = new QLabel(content);
        sentence_label_->setObjectName("sentence");
        sentence_label_->setWordWrap(true);
        sentence_label_->setAlignment(Qt::AlignCenter);
        sentence_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);
        sentence_label_->setMinimumHeight(80);
        sentence_label_->setAccessibleName("Sentence to read");
        layout->addWidget(sentence_label_);

        custom_sentence_ = new QPlainTextEdit(content);
        custom_sentence_->setObjectName("sheet");
        custom_sentence_->setPlaceholderText("Type the sentence you want to practise.");
        custom_sentence_->setFixedHeight(96);
        custom_sentence_->document()->setDocumentMargin(12);
        custom_sentence_->setAccessibleName("Your own sentence");
        custom_sentence_->hide();
        layout->addWidget(custom_sentence_);
        layout->addSpacing(8);

        next_sentence_ = new QPushButton("Another Sentence", content);
        next_sentence_->setObjectName("quiet");
        next_sentence_->setCursor(Qt::PointingHandCursor);
        layout->addWidget(next_sentence_, 0, Qt::AlignHCenter);
        layout->addSpacing(28);

        practice_button_ = new QPushButton(content);
        practice_button_->setObjectName("mic");
        practice_button_->setCursor(Qt::PointingHandCursor);
        practice_button_->setMinimumSize(240, 52);
        practice_button_->setAccessibleName("Start or stop speaking");
        layout->addWidget(practice_button_, 0, Qt::AlignHCenter);
        layout->addSpacing(14);

        practice_status_ = new QLabel(practice_instruction, content);
        practice_status_->setObjectName("muted");
        practice_status_->setAlignment(Qt::AlignCenter);
        practice_status_->setWordWrap(true);
        layout->addWidget(practice_status_);
        layout->addSpacing(28);

        // ---- The result, hidden until there is one.
        result_card_ = new QFrame(content);
        result_card_->setObjectName("card");
        auto* result_layout = new QVBoxLayout(result_card_);
        result_layout->setContentsMargins(24, 20, 24, 18);
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

        QStringList legend;
        const std::pair<WordMark, const char*> legend_items[] = {
            {WordMark::Correct, "right"}, {WordMark::Substituted, "changed"},
            {WordMark::Missed, "missed"}, {WordMark::Unclear, "check"}, {WordMark::Extra, "extra"}};
        for (const auto& [mark, label] : legend_items) {
            legend << QString("<span style='color:%1'>●</span> %2").arg(mark_colour(mark), label);
        }
        auto* legend_label = new QLabel(legend.join("&nbsp;&nbsp;&nbsp;"), result_card_);
        legend_label->setObjectName("footnote");
        legend_label->setTextFormat(Qt::RichText);
        // Said plainly, because the tool cannot do better than its recogniser;
        // kept to a tooltip so the result itself stays uncluttered.
        legend_label->setToolTip(
            "Whisper listens for whole words and tends to write the word you meant.\n"
            "A word marked check is one it was unsure of: say it again, slowly.");
        try_again_ = new QPushButton("Try Again", result_card_);
        next_after_result_ = new QPushButton("Next Sentence", result_card_);
        auto* result_footer = new QHBoxLayout();
        result_footer->addWidget(legend_label);
        result_footer->addStretch();
        for (QPushButton* button : {try_again_, next_after_result_}) {
            button->setObjectName("quiet");
            button->setCursor(Qt::PointingHandCursor);
            result_footer->addWidget(button);
        }
        result_layout->addLayout(result_footer);
        result_card_->hide();
        layout->addWidget(result_card_);
        layout->addStretch(2);

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
        return centred(content, 760, true);
    }

    // Every screen is a column on white, centred and capped, as the welcome
    // page is, so a full-screen window does not stretch a line across it.
    QWidget* centred(QWidget* content, int maximum_width, bool scroll = false) {
        content->setMaximumWidth(maximum_width);
        content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        auto* holder = new QWidget(this);
        holder->setObjectName("page");
        auto* holder_layout = new QHBoxLayout(holder);
        holder_layout->setContentsMargins(0, 0, 0, 0);
        holder_layout->addStretch(1);
        holder_layout->addWidget(content, 100);
        holder_layout->addStretch(1);
        if (!scroll) {
            return holder;
        }
        auto* area = new QScrollArea(this);
        area->setWidget(holder);
        area->setWidgetResizable(true);
        area->setFrameShape(QFrame::NoFrame);
        return area;
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
    QPushButton* save_button_;
    QPushButton* copy_button_;
    QLabel* status_label_;
    QLabel* duration_label_;
    QLabel* latency_label_;
    QLabel* indicator_;
    QListWidget* history_list_;
    QStackedWidget* stack_;
    QAction* welcome_action_;
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
        "Read it aloud, then press Stop and Check.";
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
