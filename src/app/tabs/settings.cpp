#include <aasdk_proto/ButtonCodeEnum.pb.h>
#include <aasdk_proto/VideoFPSEnum.pb.h>
#include <aasdk_proto/VideoResolutionEnum.pb.h>
#include <BluezQt/Device>
#include <BluezQt/PendingCall>
#include <f1x/openauto/autoapp/Configuration/AudioOutputBackendType.hpp>
#include <f1x/openauto/autoapp/Configuration/BluetootAdapterType.hpp>
#include <f1x/openauto/autoapp/Configuration/HandednessOfTrafficType.hpp>

#include <app/config.hpp>
#include <app/tabs/settings.hpp>
#include <app/theme.hpp>
#include <app/widgets/switch.hpp>
#include <app/window.hpp>

namespace aasdk = f1x::aasdk;
namespace autoapp = f1x::openauto::autoapp;

SettingsTab::SettingsTab(QWidget *parent) : QTabWidget(parent)
{
    this->tabBar()->setFont(Theme::font_18);

    this->addTab(new GeneralSettingsSubTab(this), "General");
    this->addTab(new BluetoothSettingsSubTab(this), "Bluetooth");
    this->addTab(new OpenAutoSettingsSubTab(this), "OpenAuto");
}

GeneralSettingsSubTab::GeneralSettingsSubTab(QWidget *parent) : QWidget(parent)
{
    this->theme = Theme::get_instance();
    this->config = Config::get_instance();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);

    layout->addWidget(this->settings_widget());
    layout->addWidget(this->controls_widget());
}

QWidget *GeneralSettingsSubTab::settings_widget()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    layout->addWidget(this->dark_mode_row_widget(), 1);
    layout->addWidget(this->color_row_widget(), 1);
    layout->addWidget(Theme::br(widget), 1);
    layout->addWidget(this->si_units_row_widget(), 1);
    layout->addWidget(Theme::br(widget), 1);
    layout->addWidget(this->brightness_row_widget(), 1);

    return widget;
}

QWidget *GeneralSettingsSubTab::dark_mode_row_widget()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);

    QLabel *label = new QLabel("Dark Mode", widget);
    label->setFont(Theme::font_16);
    layout->addWidget(label, 1);

    Switch *toggle = new Switch(widget);
    toggle->setChecked(this->config->get_dark_mode());
    connect(toggle, &Switch::stateChanged, [theme = this->theme, config = this->config](bool state) {
        theme->set_mode(state);
        config->set_dark_mode(state);
    });
    layout->addWidget(toggle, 1, Qt::AlignHCenter);

    return widget;
}

QWidget *GeneralSettingsSubTab::brightness_row_widget()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);

    QLabel *label = new QLabel("Brightness", widget);
    label->setFont(Theme::font_16);
    layout->addWidget(label, 1);

    layout->addWidget(this->brightness_widget(), 1);

    return widget;
}

QWidget *GeneralSettingsSubTab::brightness_widget()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);

    QSlider *slider = new QSlider(Qt::Orientation::Horizontal, widget);
    slider->setRange(76, 255);
    slider->setSliderPosition(this->config->get_brightness());
    connect(slider, &QSlider::valueChanged,
            [config = this->config](int position) { config->set_brightness(position); });

    QPushButton *dim_button = new QPushButton(widget);
    dim_button->setFlat(true);
    dim_button->setIconSize(Theme::icon_32);
    this->theme->add_button_icon("brightness_low", dim_button);
    connect(dim_button, &QPushButton::clicked,
            [slider]() { slider->setSliderPosition(std::max(76, slider->sliderPosition() - 18)); });

    QPushButton *brighten_button = new QPushButton(widget);
    brighten_button->setFlat(true);
    brighten_button->setIconSize(Theme::icon_32);
    this->theme->add_button_icon("brightness_high", brighten_button);
    connect(brighten_button, &QPushButton::clicked,
            [slider]() { slider->setSliderPosition(std::min(255, slider->sliderPosition() + 18)); });

    layout->addStretch(1);
    layout->addWidget(dim_button);
    layout->addWidget(slider, 4);
    layout->addWidget(brighten_button);
    layout->addStretch(1);

    return widget;
}

QWidget *GeneralSettingsSubTab::si_units_row_widget()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);

    QLabel *label = new QLabel("SI Units", widget);
    label->setFont(Theme::font_16);
    layout->addWidget(label, 1);

    Switch *toggle = new Switch(widget);
    toggle->setChecked(this->config->get_si_units());
    connect(toggle, &Switch::stateChanged, [config = this->config](bool state) { config->set_si_units(state); });
    layout->addWidget(toggle, 1, Qt::AlignHCenter);

    return widget;
}

QWidget *GeneralSettingsSubTab::color_row_widget()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);

    QLabel *label = new QLabel("Color", widget);
    label->setFont(Theme::font_16);
    layout->addWidget(label, 1);

    QComboBox *box = new QComboBox(widget);
    box->setMinimumContentsLength(16);
    box->setItemDelegate(new QStyledItemDelegate());
    box->setFont(Theme::font_14);
    QMap<QString, QColor> colors = this->theme->get_colors();
    QMap<QString, QColor>::iterator it;
    QPixmap pixmap(Theme::icon_16);
    for (it = colors.begin(); it != colors.end(); it++) {
        pixmap.fill(it.value());
        box->addItem(QIcon(pixmap), it.key());
    }
    box->setCurrentText(this->config->get_color());
    connect(box, QOverload<const QString &>::of(&QComboBox::activated),
            [theme = this->theme, config = this->config](const QString &color) {
                theme->set_color(color);
                config->set_color(color);
            });
    connect(this->theme, &Theme::color_updated, [box](QMap<QString, QColor> &colors) {
        QMap<QString, QColor>::iterator it;
        QPixmap pixmap(Theme::icon_16);
        for (it = colors.begin(); it != colors.end(); it++) {
            pixmap.fill(it.value());
            box->setItemIcon(box->findText(it.key()), QIcon(pixmap));
        }
    });
    layout->addWidget(box, 1, Qt::AlignHCenter);

    return widget;
}

QWidget *GeneralSettingsSubTab::controls_widget()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    QPushButton *save_button = new QPushButton("save", widget);
    save_button->setFont(Theme::font_14);
    save_button->setFlat(true);
    save_button->setIconSize(Theme::icon_36);
    this->theme->add_button_icon("save", save_button);
    connect(save_button, &QPushButton::clicked, [config = this->config]() { config->save(); });
    layout->addWidget(save_button, 0, Qt::AlignRight);

    return widget;
}

BluetoothSettingsSubTab::BluetoothSettingsSubTab(QWidget *parent) : QWidget(parent)
{
    this->bluetooth = Bluetooth::get_instance();
    this->theme = Theme::get_instance();
    this->config = Config::get_instance();

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);

    layout->addWidget(this->controls_widget());
    layout->addWidget(this->devices_widget());
}

QWidget *BluetoothSettingsSubTab::controls_widget()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    QLabel *label = new QLabel("Media Player", widget);
    label->setFont(Theme::font_16);
    layout->addStretch();
    layout->addWidget(label);

    QLabel *connected_device = new QLabel(this->bluetooth->get_media_player().first, widget);
    connected_device->setStyleSheet("padding-left: 16px;");
    connected_device->setFont(Theme::font_14);
    connect(this->bluetooth, &Bluetooth::media_player_changed,
            [connected_device](QString name, BluezQt::MediaPlayerPtr) { connected_device->setText(name); });
    layout->addWidget(connected_device);
    layout->addStretch();

    layout->addWidget(this->scanner_widget());

    return widget;
}

QWidget *BluetoothSettingsSubTab::scanner_widget()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);

    QPushButton *button = new QPushButton("scan", widget);
    button->setFont(Theme::font_14);
    button->setFlat(true);
    button->setEnabled(this->bluetooth->has_adapter());
    button->setIconSize(Theme::icon_36);
    this->theme->add_button_icon("bluetooth_searching", button);
    connect(button, &QPushButton::clicked, [bluetooth = this->bluetooth]() { bluetooth->scan(); });
    layout->addWidget(button);

    ProgressIndicator *loader = new ProgressIndicator(widget);
    connect(this->bluetooth, &Bluetooth::scan_status, [button, loader](bool status) {
        button->setEnabled(!status);
        if (status)
            loader->startAnimation();
        else
            loader->stopAnimation();
    });
    layout->addWidget(loader);

    return widget;
}

QWidget *BluetoothSettingsSubTab::devices_widget()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    for (BluezQt::DevicePtr device : this->bluetooth->get_devices()) {
        if (device->address() == this->config->get_bluetooth_device()) device->connectToDevice()->waitForFinished();
        QPushButton *button = new QPushButton(device->name(), widget);
        button->setFont(Theme::font_16);
        button->setCheckable(true);
        if (device->isConnected()) button->setChecked(true);
        connect(button, &QPushButton::clicked, [config = this->config, button, device](bool checked = false) {
            button->setChecked(!checked);
            if (checked) {
                device->connectToDevice()->waitForFinished();
                config->set_bluetooth_device(device->address());
            }
            else {
                device->disconnectFromDevice()->waitForFinished();
                config->set_bluetooth_device(QString());
            }
        });

        this->devices[device] = button;
        layout->addWidget(button);
    }
    connect(this->bluetooth, &Bluetooth::device_added, [this, layout, widget](BluezQt::DevicePtr device) {
        QPushButton *button = new QPushButton(device->name(), widget);
        button->setFont(Theme::font_16);
        button->setCheckable(true);
        if (device->isConnected()) button->setChecked(true);
        connect(button, &QPushButton::clicked, [button, device](bool checked = false) {
            button->setChecked(!checked);
            if (checked)
                device->connectToDevice()->waitForFinished();
            else
                device->disconnectFromDevice()->waitForFinished();
        });
        this->devices[device] = button;
        layout->addWidget(button);
    });
    connect(this->bluetooth, &Bluetooth::device_changed, [this](BluezQt::DevicePtr device) {
        this->devices[device]->setText(device->name());
        this->devices[device]->setChecked(device->isConnected());
    });
    connect(this->bluetooth, &Bluetooth::device_removed, [this, layout](BluezQt::DevicePtr device) {
        layout->removeWidget(devices[device]);
        delete this->devices[device];
        this->devices.remove(device);
    });

    return widget;
}

OpenAutoSettingsSubTab::OpenAutoSettingsSubTab(QWidget *parent) : QWidget(parent)
{
    this->bluetooth = Bluetooth::get_instance();
    this->theme = Theme::get_instance();
    this->config = Config::get_instance();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);

    layout->addWidget(this->settings_widget());
}

QWidget *OpenAutoSettingsSubTab::settings_widget()
{
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    layout->addWidget(this->rhd_row_widget(), 1);
    layout->addWidget(Theme::br(widget), 1);
    layout->addWidget(this->frame_rate_row_widget(), 1);
    layout->addWidget(this->resolution_row_widget(), 1);
    layout->addWidget(this->dpi_row_widget(), 1);
    layout->addWidget(Theme::br(widget), 1);
    layout->addWidget(this->rt_audio_row_widget(), 1);
    layout->addWidget(this->audio_channels_row_widget(), 1);
    layout->addWidget(Theme::br(widget), 1);
    layout->addWidget(this->bluetooth_row_widget(), 1);

    return widget;
}

QWidget *OpenAutoSettingsSubTab::rhd_row_widget()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);

    QLabel *label = new QLabel("Right-Hand-Drive", widget);
    label->setFont(Theme::font_16);
    layout->addWidget(label, 1);

    Switch *toggle = new Switch(widget);
    toggle->setChecked(this->config->openauto_config->getHandednessOfTrafficType() ==
                       autoapp::configuration::HandednessOfTrafficType::RIGHT_HAND_DRIVE);
    connect(toggle, &Switch::stateChanged, [config = this->config](bool state) {
        config->openauto_config->setHandednessOfTrafficType(
            state ? autoapp::configuration::HandednessOfTrafficType::RIGHT_HAND_DRIVE
                  : autoapp::configuration::HandednessOfTrafficType::LEFT_HAND_DRIVE);
    });
    layout->addWidget(toggle, 1, Qt::AlignHCenter);

    return widget;
}

QWidget *OpenAutoSettingsSubTab::frame_rate_row_widget()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);

    QLabel *label = new QLabel("Frame Rate", widget);
    label->setFont(Theme::font_16);
    layout->addWidget(label, 1);

    QGroupBox *group = new QGroupBox(widget);
    QHBoxLayout *group_layout = new QHBoxLayout(group);

    QRadioButton *fps_30_button = new QRadioButton("30fps", group);
    fps_30_button->setFixedHeight(fps_30_button->height());
    fps_30_button->setFont(Theme::font_14);
    fps_30_button->setChecked(this->config->openauto_config->getVideoFPS() == aasdk::proto::enums::VideoFPS::_30);
    connect(fps_30_button, &QRadioButton::clicked,
            [config = this->config]() { config->openauto_config->setVideoFPS(aasdk::proto::enums::VideoFPS::_30); });
    group_layout->addWidget(fps_30_button);

    QRadioButton *fps_60_button = new QRadioButton("60fps", group);
    fps_60_button->setFixedHeight(fps_60_button->height());
    fps_60_button->setFont(Theme::font_14);
    fps_60_button->setChecked(this->config->openauto_config->getVideoFPS() == aasdk::proto::enums::VideoFPS::_60);
    connect(fps_60_button, &QRadioButton::clicked,
            [config = this->config]() { config->openauto_config->setVideoFPS(aasdk::proto::enums::VideoFPS::_60); });
    group_layout->addWidget(fps_60_button);

    layout->addWidget(group, 1, Qt::AlignHCenter);

    return widget;
}

QWidget *OpenAutoSettingsSubTab::resolution_row_widget()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);

    QLabel *label = new QLabel("Resolution", widget);
    label->setFont(Theme::font_16);
    layout->addWidget(label, 1);

    QGroupBox *group = new QGroupBox(widget);
    QHBoxLayout *group_layout = new QHBoxLayout(group);

    QRadioButton *res_480_button = new QRadioButton("480p", group);
    res_480_button->setFixedHeight(res_480_button->height());
    res_480_button->setFont(Theme::font_14);
    res_480_button->setChecked(this->config->openauto_config->getVideoResolution() ==
                               aasdk::proto::enums::VideoResolution::_480p);
    connect(res_480_button, &QRadioButton::clicked, [config = this->config]() {
        config->openauto_config->setVideoResolution(aasdk::proto::enums::VideoResolution::_480p);
    });
    group_layout->addWidget(res_480_button);

    QRadioButton *res_720_button = new QRadioButton("720p", group);
    res_720_button->setFixedHeight(res_720_button->height());
    res_720_button->setFont(Theme::font_14);
    res_720_button->setChecked(this->config->openauto_config->getVideoResolution() ==
                               aasdk::proto::enums::VideoResolution::_720p);
    connect(res_720_button, &QRadioButton::clicked, [config = this->config]() {
        config->openauto_config->setVideoResolution(aasdk::proto::enums::VideoResolution::_720p);
    });
    group_layout->addWidget(res_720_button);

    QRadioButton *res_1080_button = new QRadioButton("1080p", group);
    res_1080_button->setFixedHeight(res_1080_button->height());
    res_1080_button->setFont(Theme::font_14);
    res_1080_button->setChecked(this->config->openauto_config->getVideoResolution() ==
                                aasdk::proto::enums::VideoResolution::_1080p);
    connect(res_1080_button, &QRadioButton::clicked, [config = this->config]() {
        config->openauto_config->setVideoResolution(aasdk::proto::enums::VideoResolution::_1080p);
    });
    group_layout->addWidget(res_1080_button);

    layout->addWidget(group, 1, Qt::AlignHCenter);

    return widget;
}

QWidget *OpenAutoSettingsSubTab::dpi_row_widget()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);

    QLabel *label = new QLabel("DPI", widget);
    label->setFont(Theme::font_16);
    layout->addWidget(label, 1);

    layout->addWidget(this->dpi_widget(), 1);

    return widget;
}

QWidget *OpenAutoSettingsSubTab::dpi_widget()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);

    QSlider *slider = new QSlider(Qt::Orientation::Horizontal, widget);
    slider->setFixedHeight(slider->height());
    slider->setRange(0, 512);
    slider->setSliderPosition(this->config->openauto_config->getScreenDPI());
    QLabel *value = new QLabel(QString::number(slider->sliderPosition()), widget);
    value->setFixedHeight(value->height());
    value->setFont(Theme::font_14);
    connect(slider, &QSlider::valueChanged, [config = this->config, value](int position) {
        value->setText(QString::number(position));
        config->openauto_config->setScreenDPI(position);
    });

    layout->addStretch(2);
    layout->addWidget(slider, 4);
    layout->addWidget(value, 1);
    layout->addStretch(1);

    return widget;
}

QWidget *OpenAutoSettingsSubTab::rt_audio_row_widget()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);

    QLabel *label = new QLabel("RtAudio", widget);
    label->setFont(Theme::font_16);
    layout->addWidget(label, 1);

    Switch *toggle = new Switch(widget);
    toggle->setChecked(this->config->openauto_config->getAudioOutputBackendType() ==
                       autoapp::configuration::AudioOutputBackendType::RTAUDIO);
    connect(toggle, &Switch::stateChanged, [config = this->config](bool state) {
        config->openauto_config->setAudioOutputBackendType(
            state ? autoapp::configuration::AudioOutputBackendType::RTAUDIO
                  : autoapp::configuration::AudioOutputBackendType::QT);
    });
    layout->addWidget(toggle, 1, Qt::AlignHCenter);

    return widget;
}

QWidget *OpenAutoSettingsSubTab::audio_channels_row_widget()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);

    QLabel *label = new QLabel("Audio Channels", widget);
    label->setFont(Theme::font_16);
    layout->addWidget(label, 1);

    QGroupBox *group = new QGroupBox(widget);
    QHBoxLayout *group_layout = new QHBoxLayout(group);

    QCheckBox *music_button = new QCheckBox("Music", group);
    music_button->setFixedHeight(music_button->height());
    music_button->setFont(Theme::font_14);
    music_button->setChecked(this->config->openauto_config->musicAudioChannelEnabled());
    connect(music_button, &QCheckBox::toggled,
            [config = this->config](bool checked) { config->openauto_config->setMusicAudioChannelEnabled(checked); });
    group_layout->addWidget(music_button);
    group_layout->addStretch(2);

    QCheckBox *speech_button = new QCheckBox("Speech", group);
    speech_button->setFixedHeight(speech_button->height());
    speech_button->setFont(Theme::font_14);
    speech_button->setChecked(this->config->openauto_config->speechAudioChannelEnabled());
    connect(speech_button, &QCheckBox::toggled,
            [config = this->config](bool checked) { config->openauto_config->setSpeechAudioChannelEnabled(checked); });
    group_layout->addWidget(speech_button);

    layout->addWidget(group, 1, Qt::AlignHCenter);

    return widget;
}

QWidget *OpenAutoSettingsSubTab::bluetooth_row_widget()
{
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(widget);

    QLabel *label = new QLabel("Bluetooth", widget);
    label->setFont(Theme::font_16);
    layout->addWidget(label, 1);

    Switch *toggle = new Switch(widget);
    toggle->setChecked(this->config->openauto_config->getBluetoothAdapterType() ==
                       autoapp::configuration::BluetoothAdapterType::LOCAL);
    connect(toggle, &Switch::stateChanged, [config = this->config](bool state) {
        config->openauto_config->setBluetoothAdapterType(state ? autoapp::configuration::BluetoothAdapterType::LOCAL
                                                                : autoapp::configuration::BluetoothAdapterType::NONE);
    });
    layout->addWidget(toggle, 1, Qt::AlignHCenter);

    return widget;
}
